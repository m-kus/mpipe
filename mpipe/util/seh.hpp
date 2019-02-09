#pragma once

#include <util/exception.hpp>

#ifdef _WIN32

#include <WinBase.h>
#include <eh.h>

inline const char* seh_message(unsigned int code)
{
	switch (code)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		return "The thread attempts to read from or write to a virtual address "
			"for which it does not have access.";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		return "The thread attempts to access an array element that is out of "
			"bounds, and the underlying hardware supports bounds checking.";
	case EXCEPTION_BREAKPOINT:
		return "A breakpoint is encountered.";
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		return "he thread attempts to read or write data that is misaligned on "
			"hardware that does not provide alignment. For example, 16-bit values "
			"must be aligned on 2-byte boundaries, 32-bit values on 4-byte "
			"boundaries, and so on.";
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		return "One of the operands in a floating point operation is denormal. A "
			"denormal value is one that is too small to represent as a standard "
			"floating point value.";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		return "The thread attempts to divide a floating point value by a floating "
			"point divisor of 0 (zero).";
	case EXCEPTION_FLT_INEXACT_RESULT:
		return "The result of a floating point operation cannot be represented "
			"exactly as a decimal fraction.";
	case EXCEPTION_FLT_INVALID_OPERATION:
		return "A floating point exception that is not included in this list.";
	case EXCEPTION_FLT_OVERFLOW:
		return "The exponent of a floating point operation is greater than the "
			"magnitude allowed by the corresponding type.";
	case EXCEPTION_FLT_STACK_CHECK:
		return "The stack has overflowed or underflowed, because of a floating "
			"point operation.";
	case EXCEPTION_FLT_UNDERFLOW:
		return "The exponent of a floating point operation is less than the "
			"magnitude allowed by the corresponding type.";
	case EXCEPTION_GUARD_PAGE:
		return "The thread accessed memory allocated with the PAGE_GUARD modifier.";
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		return "The thread tries to execute an invalid instruction.";
	case EXCEPTION_IN_PAGE_ERROR:
		return "The thread tries to access a page that is not present, and the "
			"system is unable to load the page. For example, this exception might "
			"occur if a network connection is lost while running a program over a "
			"network.";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		return "The thread attempts to divide an integer value by an integer divisor "
			"of 0 (zero).";
	case EXCEPTION_INT_OVERFLOW:
		return "The result of an integer operation creates a value that is too large "
			"to be held by the destination register. In some cases, this will result "
			"in a carry out of the most significant bit of the result. Some operations "
			"do not set the carry flag.";
	case EXCEPTION_INVALID_DISPOSITION:
		return "An exception handler returns an invalid disposition to the exception "
			"dispatcher. Programmers using a high-level language such as C should never "
			"encounter this exception.";
	case EXCEPTION_INVALID_HANDLE:
		return "The thread used a handle to a kernel object that was invalid (probably "
			"because it had been closed.)";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		return "The thread attempts to continue execution after a non-continuable "
			"exception occurs.";
	case EXCEPTION_PRIV_INSTRUCTION:
		return "The thread attempts to execute an instruction with an operation that "
			"is not allowed in the current computer mode.";
	case EXCEPTION_SINGLE_STEP:
		return "A trace trap or other single instruction mechanism signals that one "
			"instruction is executed.";
	case EXCEPTION_STACK_OVERFLOW:
		return "The thread uses up its stack.";
	case STATUS_UNWIND_CONSOLIDATE:
		return "A frame consolidation has been executed.";
	default: return "unknown error";
	}
}

inline void SehRethrow(unsigned int e_code, PEXCEPTION_POINTERS e_info)
{
	throw mpipe_error_message(seh_message(e_code));
}

inline void SehEnable()
{
	_set_se_translator(SehRethrow);
}

#else

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void SehRethrow(int signal, siginfo_t *si, void *arg)
{
	throw mpipe_error_code(signal);
}

inline void SehEnable()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = SehRethrow;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL);
}

#endif

inline void SehTranslationEnable() 
{
	static bool enabled = false;
	if (!enabled)
	{
		SehEnable();
		enabled = true;
		log_format("ok");
	}
}
