#pragma once

#include <util/stacktrace.hpp>
#include <util/logger.hpp>

#define MAX_ERROR_MESSAGE	512

#ifdef _WIN32

inline void GetErrorMessage(std::int32_t code, char* message)
{
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, 0, message, MAX_ERROR_MESSAGE, NULL);
}

#else

#include <errno.h>

inline std::int32_t GetLastError()
{
	return errno;
}

inline void GetErrorMessage(std::int32_t code, char* message)
{
	std::strncpy(message, strerror(code), MAX_ERROR_MESSAGE);
}

#endif

struct mpipe_exception : public std::exception
{
	int exit_code;
	std::string exit_msg;

	mpipe_exception(const char* file, const char* function, int line, const char* message, int code)
		: exit_code(code)
	{
		char error_message[MAX_ERROR_MESSAGE] = {};
		if (!message)
		{
			GetErrorMessage(code, error_message);
			message = error_message;
		}

		Log(function, "%s [%d]\r\n", message, code);

#ifndef _DEBUG
		static StackTrace stack_trace;
		auto trace = stack_trace.Capture();
		for (std::size_t i = 0; i < trace.size(); i++)
			Log(trace[i].c_str(), "\r\n");
#endif
	}

	virtual const char* what() const noexcept override
	{
		return exit_msg.c_str();
	}
};

#define mpipe_error_code(code)				mpipe_exception(__FILE__, __FUNCTION__, __LINE__, NULL, code)
#define mpipe_error_message(message)		mpipe_exception(__FILE__, __FUNCTION__, __LINE__, message, 0)
