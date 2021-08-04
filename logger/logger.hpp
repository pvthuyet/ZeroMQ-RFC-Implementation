#ifndef SG_LOGGER_H_
#define SG_LOGGER_H_

#include "define.hpp"

SAIGON_NAMESPACE_BEGIN
class logger
{
public:
	static logger& get_instance();
	logger(logger const&) = delete;
	logger& operator=(logger const&) = delete;
	logger(logger&&) = delete;
	logger& operator=(logger&&) = delete;

private:
	logger();
	void initialze();
};
SAIGON_NAMESPACE_END

#endif // !SG_LOGGER_H_