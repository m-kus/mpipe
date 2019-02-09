#pragma once

#include <map>
#include <cassert>
#include <sstream>
#include <cinttypes>

namespace mpipe
{
	typedef std::map<std::int64_t, std::int64_t> book_map_t;

	struct Book
	{
		book_map_t book;
		book_map_t::iterator bid;
		book_map_t::iterator ask;
		std::int64_t ts;

		Book()
			: bid(book.end())
			, ask(book.end())
			, ts(0)
		{
		}

		bool Valid()
		{
			return bid != book.end()
				&& ask != book.end()
				&& bid->second > 0
				&& ask->second > 0;
		}

		void Reset()
		{
			ts = 0;
			bid = book.end();
			ask = book.end();
			book.clear();
		}

		void Commit(std::int64_t _ts)
		{
			ts = Valid() ? _ts : 0;
		}

		book_map_t::iterator Set(std::int64_t px, std::int64_t qty)
		{
			auto it = book.find(px);

			if (it != book.end())
			{
				it->second = qty;
			}
			else
			{
				it = book.emplace(px, qty).first;
			}

			return it;
		}

		void SetBid(std::int64_t px, std::int64_t qty)
		{
			bid = Set(px, qty);
		}

		void SetAsk(std::int64_t px, std::int64_t qty)
		{
			ask = Set(px, qty);
		}

		void TrySetBid(std::int64_t px, std::int64_t qty)
		{
			if (bid == book.end() 
				|| px > bid->first)
			{
				SetBid(px, qty);
			}
			else
			{
				Set(px, qty);
			}
		}

		void TrySetAsk(std::int64_t px, std::int64_t qty)
		{
			if (ask == book.end()
				|| px < ask->first)
			{
				SetAsk(px, qty);
			}
			else
			{
				Set(px, qty);
			}
		}

		void Delete(std::int64_t px)
		{
			auto it = Set(px, 0);

			if (bid == it)
			{
				for (; bid != book.begin() && bid->second == 0; --bid) { ; }
				if (bid->second == 0
					&& bid == book.begin())
				{
					bid = book.end();
				}
			}
			else
			if (ask == it)
			{
				for (; ask != book.end() && ask->second == 0; ++ask) { ; }
			}

			it = book.erase(it);
		}

		std::string __repr__(const std::string& prefix = "")
		{
			std::stringstream ss;			
			if (Valid())
			{
				ss << prefix << "bid: " << bid->second << " @ " << bid->first << std::endl;
				ss << prefix << "ask: " << ask->second << " @ " << ask->first << std::endl;
			}

			return ss.str();
		}
	};
}
