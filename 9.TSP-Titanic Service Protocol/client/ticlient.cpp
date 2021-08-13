#include "ticlient.hpp"
#include "constant.hpp"
#include "logger/logger_define.hpp"

SAIGON_NAMESPACE_BEGIN
ticlient::ticlient(zmqpp::context_t& ctx,
	std::string_view broker,
	std::string_view id):
	client_(ctx, broker, id)
{}

std::string ticlient::send(std::string_view srvname, zmqpp::message_t& req)
{
	// Send 'echo' request to titanic
	req.push_front(srvname.data());
	auto reply = service_call(TITANIC_REQUEST, req);
	if (0 == reply.parts()) {
		throw std::runtime_error(std::format("Request to {} service is timeout", TITANIC_REQUEST));
	}
	auto status = reply.get<std::string>(0);
	if (status == MMI_FOUND) {
		return reply.get<std::string>(1);
	}
	throw std::runtime_error(std::format("Service fatal error {}", status));
}

zmqpp::message_t ticlient::recv(std::string_view uuid)
{
	SPDLOG_INFO("Asking result for {}", uuid);
	// Wait until we get a reply
	while (1) {
		zmqpp::message_t msg(uuid.data());
		auto res = service_call(TITANIC_REPLY, msg);
		if (!res.parts()) {
			SPDLOG_INFO("No reply yet, trying again...");
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		}
		else {
			// close request
			//zmqpp::message_t clmsg(uuid.data());
			//auto clres = service_call(TITANIC_CLOSE, clmsg);
			return res;
		}
	}
	return {};
}

zmqpp::message_t ticlient::service_call(std::string_view srvname, zmqpp::message_t& req)
{
	client_.send(srvname, req);
	auto reply = client_.recv();
	if (reply.parts()) {
		auto status = reply.get<std::string>(0);
		if (status == MMI_FOUND) {
			return reply;
		}
		//throw std::runtime_error(std::format("Server fatal error ({})", status));
	}
	return {};
}

SAIGON_NAMESPACE_END