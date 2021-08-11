#include "titanic.hpp"

SAIGON_NAMESPACE_BEGIN
titanic::titanic(zmqpp::context_t& ctx)
	: ctx_{ctx}
{
}

void titanic::start()
{
	zmqpp::actor request([this](zmqpp::socket_t* pipe) -> bool {
		return this->request(pipe);
		});
	
}

bool titanic::request(zmqpp::socket_t* pipe)
{
	pipe->send(zmqpp::signal::ok);
	// TODO

	return true;
}

void run(std::stop_token tok)
{

}

SAIGON_NAMESPACE_END