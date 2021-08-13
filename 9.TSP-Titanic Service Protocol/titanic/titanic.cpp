#include "titanic.hpp"
#include "request_worker.hpp"
#include "reply_worker.hpp"
#include "mdcliapi.hpp"
#include "utils/zmqutil.hpp"
#include <filesystem>
#include <fstream>
#include "logger/logger_define.hpp"

SAIGON_NAMESPACE_BEGIN
titanic::titanic(zmqpp::context_t& ctx,
	std::string_view brokerep,
	std::string_view adminep
)
	: ctx_{ctx},
	brokerep_{brokerep},
	adminep_{adminep}
{
}

void titanic::start()
{
	zmqpp::actor request([this](zmqpp::socket_t* pipe) -> bool {
		return this->request(pipe);
		});

	reply_worker reply(ctx_, brokerep_, adminep_);
	reply.start();
	run(request.pipe());

	reply.wait();
}

void titanic::wait()
{

}

bool titanic::request(zmqpp::socket_t* pipe)
{
	pipe->send(zmqpp::signal::ok);
	request_worker reqwrk(pipe, ctx_, brokerep_, adminep_);
	reqwrk.start();
	reqwrk.wait();
	return true;
}

void titanic::run(zmqpp::socket_t* pipe)
{
	using namespace std::string_literals;
	namespace fs = std::filesystem;
	std::string queuefile = TITANIC_DIR + "/queue"s;
	zmqpp::poller_t poller{};
	poller.add(*pipe);
	while (1) {
		poller.poll(1000);

		if (poller.events(*pipe) == zmqpp::poller_t::poll_in) {
			// Ensure message direcotry exists
			std::filesystem::create_directories(TITANIC_DIR);

			// append UUID to queue, prefixed with '-' for pending
			zmqpp::message_t msg;
			pipe->receive(msg);
			
			auto uuid = msg.get<std::string>(0);
			std::ofstream ofs(queuefile, std::ios::out | std::ios::app);
			ofs << std::format("-{}\n", uuid);
		}

		if (fs::exists(queuefile)) {
			std::fstream fs(queuefile, std::ios::in | std::ios::out);
			fs.seekg(0, std::ios::end);
			auto size = fs.tellg();
			fs.seekg(0);
			size_t pos = 0;
			std::string uuid(UUID_LENGTH, '\0');
			while (pos < size) {
				fs.read(uuid.data(), UUID_LENGTH);
				if (uuid[0] == '-') {
					SPDLOG_INFO("Processing request {}", uuid);
					if (service_success(uuid.substr(1))) {
						fs.seekp(pos);
						fs.write("+", 1);
					}
				}
				pos += UUID_LENGTH + 2;
				fs.seekg(pos);
			}
		}
	}
}

int titanic::service_success(std::string_view uuid)
{
	// Load rquest message, service will be fist frame
	auto req = zmqutil::load(std::format("{}/{}.req", TITANIC_DIR, uuid));
	auto srvname = req.get<std::string>(0);
	req.pop_front();

	// Create MDP client session with short timeout
	mdcliapi client(ctx_, brokerep_);

	// Use MMI protocal to check if service is available
	client.send(MMI_SERVICE, srvname);
	auto mmireply = client.recv();
	if (0 == mmireply.parts()) return 0;

	if (mmireply.get<std::string>(0) == MMI_FOUND) {
		client.send(srvname, req);
		auto reply = client.recv();
		if (reply.parts()) {
			zmqutil::save(std::format("{}/{}.rep", TITANIC_DIR, uuid), reply);
			return 1;
		}
	}
	return 0;
}

SAIGON_NAMESPACE_END