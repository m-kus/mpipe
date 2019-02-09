#pragma once

#include <util/string.hpp>
#include <util/path.hpp>

#define STACK_TRACE_MAX_DEPTH	16
#define STACK_TRACE_MAX_SYMBOL	1024

#ifdef _WIN32

#pragma warning (disable : 4091)

#include <Windows.h>
#include <Dbghelp.h>

#pragma comment (lib, "Dbghelp.lib")

#define SYMBOL_MAX_PATH			1024

class StackTrace
{
private:

	HANDLE process_handle_;

public:

	StackTrace()
		: process_handle_(GetCurrentProcess())
	{
		char module_path[SYMBOL_MAX_PATH]{};
		GetModuleFileNameA(nullptr, module_path, SYMBOL_MAX_PATH);

		std::vector<std::string> pdb_dir = {
			PathGetDirectory(module_path),
			"C:\\Windows\\System32",
			"C:\\Windows"
		};

		std::string symbol_path = string_join(pdb_dir, ";");
		SymInitialize(process_handle_, symbol_path.c_str(), TRUE);
	}

	~StackTrace()
	{
		SymCleanup(process_handle_);
	}

	std::vector<std::string> Capture()
	{
		void *trace[STACK_TRACE_MAX_DEPTH]{};
		WORD trace_size = RtlCaptureStackBackTrace(0, STACK_TRACE_MAX_DEPTH, trace, NULL);

		HANDLE this_process = GetCurrentProcess();

		SYMBOL_INFO *symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + STACK_TRACE_MAX_SYMBOL);
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = STACK_TRACE_MAX_SYMBOL;

		std::vector<std::string> stack_trace;
		for (auto i = 0; i < trace_size; i++)
		{
			if (SymFromAddr(this_process, (DWORD64)trace[i], 0, symbol))
				stack_trace.push_back(symbol->Name);
		}

		free(symbol);
		return stack_trace;
	}
};

#else

#include <execinfo.h>

class StackTrace
{
public:

	std::vector<std::string> Capture()
	{
		void *entries[10];
		std::size_t entries_count = backtrace(entries, STACK_TRACE_MAX_DEPTH);
		char **entries_symbols = backtrace_symbols(entries, entries_count);

		std::vector<std::string> stack_trace;
		for (std::size_t i = 0; i < entries_count; i++)
			stack_trace.push_back(std::string(entries_symbols[i]));

		return stack_trace;
	}
};

#endif
