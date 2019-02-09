#pragma once

#include <util/time.hpp>
#include <util/path.hpp>
#include <util/string.hpp>

#include <spdlog/spdlog.h>

#include <memory>

class Logger
{
private:

	std::shared_ptr<spdlog::logger> logger_;
	const std::int64_t queue_size_ = 1024;
	const std::int64_t flush_interval_ms_ = 1000;

public:

	Logger(const std::string& name, bool console, bool async)
	{
		spdlog::set_pattern("%H:%M:%S:%f  %t@%P  %v");

		if (async)
		{
			spdlog::set_async_mode(
				queue_size_,
				spdlog::async_overflow_policy::block_retry,
				nullptr,
				std::chrono::milliseconds(flush_interval_ms_)
			);
		}

		if (console)
		{
			logger_ = spdlog::stdout_color_mt(name);			
		}
		else
		{
			DirectoriesCreate(".logs");
			logger_ = spdlog::daily_logger_mt(name, ".logs/daily");
		}
	}

	~Logger()
	{
		logger_->flush();
		spdlog::drop_all();
	}

	void Write(const std::string& function, const std::string& message)
	{
		logger_->info("{}  {}", function, message);
	}

	static Logger* Instance(bool console = false, bool async = true)
	{
		static std::unique_ptr<Logger> logger(new Logger("default", console, async));
		return logger.get();
	}
	
	static void WriteDefault(const std::string& function, const std::string& message)
	{		
		Instance()->Write(function, message);
	}
};

template<typename ... Args>
inline void Log(const char* function, const char* format, Args ... args)
{
	Logger::WriteDefault(function, string_format(format, args ...));
}

inline void LogInit(bool console, bool async)
{
	Logger::Instance(console, async);
}

#ifdef _WIN32
#define log_format(format, ...)		Log(__FUNCTION__, format, __VA_ARGS__)
#else
#define log_format(format, ...)		Log(__FUNCTION__, format, ##__VA_ARGS__)
#endif
