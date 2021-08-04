#ifndef LOGGER_DEFINE_H_
#define LOGGER_DEFINE_H_

#ifdef IS_DEBUG
#define SPDLOG_ACTIVE_LEVEL				SPDLOG_LEVEL_DEBUG
#define SPDLOG_DEBUG_ON					1
#endif // IS_DEBUG

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT	1
#include "spdlog/spdlog.h"

#define LOGENTER	SPDLOG_INFO("Enter {")
#define LOGEXIT		SPDLOG_INFO("Exit }")

#endif // !LOGGER_DEFINE_H_