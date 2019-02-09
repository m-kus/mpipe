#pragma once

#ifndef _WIN32
#define __fallthrough
#endif

namespace mpipe
{
	enum Action
	{
		Terminate = 0,
		ForceClose = 1,
		Bypass = 2,
		Close = 3,
		Open = 4
	};

	enum Side
	{
		All = 0,
		Buy = 1,
		Sell = -1
	};

	enum class OrderStatus
	{
		Pending,
		Placed,
		Canceled,
		Filled
	};

	inline const char* action_str(Action action)
	{
		switch (action)
		{
			case Action::Bypass: return "bypass";
			case Action::ForceClose: return "force close";
			case Action::Close: return "close";
			case Action::Open: return "open";
			default: return "";
		}
	}

	inline const char* side_str(Side side)
	{
		switch (side)
		{
			case Side::All: return "all";
			case Side::Buy: return "buy";
			case Side::Sell: return "sell";
			default: return "";
		}
	}
}
