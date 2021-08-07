#include "mdworker_factory.hpp"
#include "utils/config_parser.hpp"
#include "echoworker.hpp"
#include "constant.hpp"
#include <boost/algorithm/string.hpp>

SAIGON_NAMESPACE_BEGIN
std::vector<std::unique_ptr<iworker>> mdworker_factory::create(zmqpp::context_t& ctx, const config_parser& config)
{
	using namespace std::string_literals;
	std::vector<std::unique_ptr<iworker>> wrks;
	auto adminep	= config.get_value(ADMIN_ENDPOINT);
	auto brokerep	= config.get_value(BACKEND_ENDPOINT);

	for (auto const& s : config.get_service()) {
		if (boost::iequals(s.name, echoworker::NAME)) {
			for (int i = 0; i < s.quantity; ++i) {
				wrks.push_back(std::make_unique<echoworker>(ctx, brokerep, adminep));
			}
		}
	}
	return wrks;
}
SAIGON_NAMESPACE_END