#pragma once

#include <util/exception.hpp>
#include <cpp-httplib/httplib.h>

inline std::string http_get(const std::string& host, const std::string& query)
{		
	httplib::Client cli(host.c_str(), 80);
	
	httplib::Request req;
	req.set_header("Host", host.c_str());
	req.method = "GET";
	req.path = query;	

	httplib::Response res;
	if (!cli.send(req, res))
		throw mpipe_error_message_code(GetLastError());

	if (res.status != 200)
		throw mpipe_error_message_code(res.status);
		
	return res.body;
}
