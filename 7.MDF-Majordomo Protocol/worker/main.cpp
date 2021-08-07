#include <vld.h>
#include "mdwrkapi.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include <boost/stacktrace.hpp>

int main()
{
	using namespace sg;
	logger::get_instance();
	try {
		zmqpp::context_t ctx;
		sg::mdwrk session(ctx, "tcp://localhost:5555", "echo", "001");
		while (1) {
			auto req = session.recv();
			if (req) {
				session.send(std::move(req));
			}
		}
	}
	catch (const std::exception& ex) {
		//SPDLOG_ERROR(ex.what());
		//SPDLOG_ERROR(boost::stacktrace::stacktrace());
		std::cerr << boost::stacktrace::stacktrace() << std::endl;
	}


	return EXIT_SUCCESS;
}