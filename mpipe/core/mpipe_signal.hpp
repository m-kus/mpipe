#pragma once

#include <core/mpipe_enum.hpp>
#include <util/time.hpp>

#include <string>
#include <sstream>

namespace mpipe
{
	struct Signal
	{
		std::int64_t ts;
		Side side;
		Action action;

		Signal()
		{
			Reset();
		}

		void Reset()
		{
			ts = 0;
			side = Side::All;
			action = Action::Bypass;
		}

		bool operator !=(const Signal& right)
		{
			return side != right.side
				|| action != right.action;
		}

		Signal& operator =(const Signal& right)
		{
			ts = right.ts;
			side = right.side;
			action = right.action;
			return *this;
		}

		bool Tradable() const
		{
			switch (action)
			{
				case Action::Open: return side != Side::All;
				case Action::Close: return true;
				case Action::ForceClose: return true;
				default: return false;
			}
		}

		std::string __repr__(const std::string& prefix = "")
		{
			std::stringstream ss;
			ss << prefix << "ts: " << ts << " (" << ts_format(ts) << ")" << std::endl;
			ss << prefix << "side: " << side_str(side) << std::endl;
			ss << prefix << "action " << action_str(action) << std::endl;

			return ss.str();
		}
	};
}
