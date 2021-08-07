#include "config_parser.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

SAIGON_NAMESPACE_BEGIN

void config_parser::load(std::string_view path)
{
	using json = nlohmann::json;
	std::ifstream ifs(path.data());
	auto js = std::make_unique<json>(json::parse(ifs));
	for (auto& [k, v] : js->items()) {
		if (k == "services") {
			for (auto& el : v.items()) {
				services_.push_back(el.value().get<service_info>());
			}
		}
		else {
			baseinfo_.insert(std::make_pair(k, v));
		}
	}
}

const std::vector<service_info>& config_parser::get_service() const
{
	return services_;
}

std::string config_parser::get_value(std::string const& key) const
{
	if (baseinfo_.count(key)) {
		return baseinfo_.at(key);
	}
	return {};
}

SAIGON_NAMESPACE_END