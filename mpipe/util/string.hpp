#pragma once

#include <string>
#include <vector>
#include <locale>
#include <memory>
//#include <codecvt>
#include <sstream>
#include <iostream>
#include <iterator>
#include <cstdio>

template<typename ... Args>
inline std::string string_format(const char* format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format, args ...) + 1; // Extra space for '\0'

	std::unique_ptr<char[]> buffer(new char[size]);
	std::snprintf(buffer.get(), size, format, args ...);

	return std::string(buffer.get(), buffer.get() + size - 1); // We don't want the '\0' inside
}

inline std::vector<std::string> string_split(const std::string& source, char delimiter)
{
	std::vector<std::string> tokens;
	std::istringstream iss(source);
	std::string token;

	while (std::getline(iss, token, delimiter))
		tokens.push_back(token);

	return tokens;
}

inline std::string string_join(const std::vector<std::string>& slices, const char* delimiter)
{
	std::stringstream ss;
	std::copy(slices.begin(), slices.end(), std::ostream_iterator<std::string>(ss, delimiter));

	return ss.str();
}

inline std::string string_conv(const wchar_t* source)
{
	std::wstring wstr(source);
	return std::string(wstr.begin(), wstr.end());
}

//inline std::string string_conv_utf8(const std::wstring& source)
//{
//	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
//	return converter.to_bytes(source);
//}
