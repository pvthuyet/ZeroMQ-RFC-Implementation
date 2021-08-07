#pragma once

#include "define.hpp"
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

SAIGON_NAMESPACE_BEGIN
struct service_info
{
	std::string name;
	int quantity;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(service_info, name, quantity)
};

class config_parser
{
private:
	std::unordered_map<std::string, std::string> baseinfo_;
	std::vector<service_info> services_;

public:
	void load(std::string_view path);
	const std::vector<service_info>& get_service() const;
	std::string get_value(std::string const& key) const;
};

SAIGON_NAMESPACE_END