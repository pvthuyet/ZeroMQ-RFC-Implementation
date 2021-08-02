#include "worker_queue.hpp"
#include <zmqpp/zmqpp.hpp>
#include <fmt/core.h>

int main(int argc, char* argv[])
{
	using namespace saigon;
	//if (argc < 2) {
	//	fmt::print("Usage: ppqueue [config.json]\n");
	//	return -1;
	//}

	zmqpp::context_t ctx;
	zmqpp::socket_t frontend(ctx, zmqpp::socket_type::router);
	zmqpp::socket_t backend(ctx, zmqpp::socket_type::router);
	frontend.bind("tcp://*:5555");	// for clients
	backend.bind("tcp://*:5556");	// for workers

	// queue of available workers
	worker_queue queue;
	
	zmqpp::poller_t poller{};
	poller.add(frontend);
	poller.add(backend);
	

	while (1) {
		poller.poll(HEARTBEAT_INTERVAL);

	}

	// send out heartbeats at regular intervals

	return EXIT_SUCCESS;
}