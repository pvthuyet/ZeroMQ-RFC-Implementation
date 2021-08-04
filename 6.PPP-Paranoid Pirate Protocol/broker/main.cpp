#include <iostream>
#include <vld.h>
#include "worker_queue.hpp"
#include "ppbroker.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"

int main(int argc, char* argv[])
{
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 2) {
			SPDLOG_INFO("Usage: ppproxy config.json");
			return EXIT_FAILURE;
		}

		// Load config file
		// ..
		zmqpp::context_t ctx;
		ppbroker broker(ctx);
		broker.start("tcp://*:5555", "tcp://*:5556");

		// stop
		//std::thread t([&ctx]() {
		//	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		//	ctx.terminate();
		//	});
		//t.join();
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}