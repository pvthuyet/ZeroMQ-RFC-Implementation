#pragma once

#include "define.hpp"
#include "bsp_constant.hpp"
#include <zmqpp/zmqpp.hpp>
#include <string>

SAIGON_NAMESPACE_BEGIN

class binary_star_server1
{
private:
	zmqpp::context_t& ctx_;
	std::string frontend_host_;
	std::string statepub_host_;
	std::string statesub_ep_;
	bstar_t fsm_; // finite-state machine

public:
	binary_star_server1(zmqpp::context_t& ctx,
		std::string_view fehost,
		std::string_view statepub_host,
		std::string_view statesub_ep
		);
	void start(bool isprimary);

private:
	bool update_state(bstar_t& fsm);
};

SAIGON_NAMESPACE_END