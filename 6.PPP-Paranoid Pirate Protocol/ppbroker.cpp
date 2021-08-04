#include <iostream>
#include <vld.h>
#include "worker_queue.hpp"
#include "paranoid_pirate_broker.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"

int main(int argc, char* argv[])
{
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 3) {
			SPDLOG_INFO("Usage: ppproxy config.json");
			return EXIT_FAILURE;
		}

		// Load config file
		// ..
		zmqpp::context_t ctx;
		paranoid_pirate_broker proxy(ctx);
		proxy.start(argv[1], argv[2]);

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