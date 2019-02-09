#pragma once

#include <mcast/mcast_book.hpp>
#include <util/logger.hpp>

namespace mpipe
{
	struct McastExtension
	{
		McastBook* host;
		std::string suffix;
		
		McastExtension(const std::string& _suffix = "")
			: suffix(_suffix)
		{
		}

		virtual void Reset(std::int64_t ts) {};
		virtual bool Online() { return true; };
		virtual void UpdateBegin(std::int64_t ts) {};
		virtual void Update(const McastBookUpdate& update) {};
		virtual void UpdateEnd(std::int64_t ts) {};
	};
}
