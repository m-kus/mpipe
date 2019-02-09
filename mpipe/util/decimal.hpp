#pragma once

#include <util/numeric.hpp>
#include <util/string.hpp>
#include <cstring>

inline std::int64_t decimal_factor(std::int64_t exponent)
{
	static const std::int64_t factor[] =
	{
		1,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000,
		1000000000,
		10000000000,
	};
	return factor[exponent];
}

inline std::double_t string_to_double(const char* p, int size = -1)
{
	std::double_t result = 0.0;
	bool negative = false;
	const char *end = size != -1 ? p + size : p + strlen(p);

	if (*p == '-')
	{
		++p;
		negative = true;
	}

	for (; *p >= '0' && *p <= '9' && p < end; ++p)
		result = (result * 10.0) + (*p - '0');

	if (*p == '.' && p < end)
	{
		++p;

		std::double_t tail = 0.0;
		int exponent = 0;

		for (; *p >= '0' && *p <= '9' && p < end; ++p, ++exponent)
			tail = (tail * 10.0) + (*p - '0');

		result += tail / decimal_factor(exponent);
	}

	if (negative)
		result = -result;

	return result;
}

inline std::string decimal_format(std::int64_t value, std::int64_t decimals)
{
	std::int64_t pwr = decimal_factor(decimals);
	switch (pwr)
	{
		case 1: return std::to_string(value);
		default: return string_format("%lld.%0*lld", value / pwr, decimals, npabs(value) % pwr);
	}
}

inline std::double_t decimal_double(std::int64_t value, std::int64_t decimals)
{
	return (std::double_t)value / (std::double_t)decimal_factor(decimals);
}

inline std::int64_t decimal_int(std::double_t value, std::int64_t decimals)
{
	return value * decimal_factor(decimals);
}
