#include <vld.h>
#include "flcliapi.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include "utils/config_parser.hpp"
#include "utils/zmqutil.hpp"
#include "flp_constant.hpp"
#include <boost/algorithm/string.hpp>

int main(int argc, char* argv[])
{
	using namespace std::string_literals;
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 2) {
			SPDLOG_INFO("Usage: flclient config.json");
			return EXIT_FAILURE;
		}

		// Load config file
		sg::config_parser conf;
		conf.load(argv[1]);
		zmqpp::context_t ctx;

		sg::flcliapi cli(ctx);
		cli.connect(conf.get_value(server_endpoint));
		cli.connect(conf.get_value(server_endpoint2));
		cli.connect(conf.get_value(server_endpoint3));
		for (int i = 1; i < 100000; ++i) {
			zmqpp::message_t msg;
			msg << std::format("hello {}", i);
			auto reply = cli.request(msg);
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		}
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}
	return EXIT_SUCCESS;
}