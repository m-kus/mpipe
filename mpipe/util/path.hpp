#pragma once

#include <util/string.hpp>

#include <cassert>
#include <algorithm>
#include <iostream>
#include <string>

#include <sys/stat.h> // stat
#include <errno.h>    // errno, ENOENT, EEXIST
#if defined(_WIN32)
#include <direct.h>   // _mkdir
#endif

inline bool DirectoryExists(const std::string& path)
{
#if defined(_WIN32)
	struct _stat info;
	if (_stat(path.c_str(), &info) != 0)
	{
		return false;
	}
	return (info.st_mode & _S_IFDIR) != 0;
#else 
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
	{
		return false;
	}
	return (info.st_mode & S_IFDIR) != 0;
#endif
}

inline bool DirectoriesCreate(const std::string& path)
{
#if defined(_WIN32)
	int ret = _mkdir(path.c_str());
#else
	mode_t mode = 0755;
	int ret = mkdir(path.c_str(), mode);
#endif
	if (ret == 0)
		return true;

	switch (errno)
	{
	case ENOENT:
		// parent didn't exist, try to create it
	{
		std::size_t pos = path.find_last_of('/');
		if (pos == std::string::npos)
#if defined(_WIN32)
			pos = path.find_last_of('\\');
		if (pos == std::string::npos)
#endif
			return false;
		if (!DirectoriesCreate(path.substr(0, pos)))
			return false;
	}
	// now, try to create again
#if defined(_WIN32)
	return 0 == _mkdir(path.c_str());
#else 
	return 0 == mkdir(path.c_str(), mode);
#endif

	case EEXIST:
		// done!
		return DirectoryExists(path);

	default:
		return false;
	}
}

inline std::string PathGetBasename(const std::string& path)
{
	std::string basename = path;
	if (path[0] == '"')
		basename = basename.substr(1, basename.size() - 2);

	size_t last_delim = basename.find_last_of("\\/");
	if (last_delim != std::string::npos)
		basename = basename.substr(last_delim + 1);

	size_t last_dot = basename.find_last_of(".");
	if (last_dot != std::string::npos)
		basename = basename.substr(0, last_dot);

	return basename;
}

inline std::string PathGetDirectory(const std::string& path)
{
	size_t last_delim = path.find_last_of("\\/");
	if (last_delim != std::string::npos)
		return path.substr(0, last_delim);

	return "./";
}
