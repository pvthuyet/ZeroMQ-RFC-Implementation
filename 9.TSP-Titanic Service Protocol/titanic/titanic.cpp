#include "titanic.hpp"
#include "request_worker.hpp"

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

SAIGON_NAMESPACE_END