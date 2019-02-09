#pragma once

#include <core/mpipe_order.hpp>
#include <core/mpipe_signal.hpp>
#include <core/mpipe_security.hpp>
#include <util/concurrent.hpp>

#include <unordered_map>

namespace mpipe
{
	class State : public spin_lock
	{
	private:

		std::unordered_map<std::string, Security*> full_code_index_;

	public:

		Signal signal;
		std::vector<Order> orders;
		std::vector<Security*> securities;

	public:

		State(const std::unordered_map<std::string, Security*>& _securities)
		{
			orders.reserve(_securities.size());
			securities.reserve(_securities.size());

			for (auto& item : _securities)
			{
				full_code_index_[item.first] = item.second;
				securities.push_back(item.second);
			}
		}
		
		Security* GetSecurity(const std::string& full_code)
		{
			return full_code_index_[full_code];
		}
	
		bool BookPending(const std::vector<Security*>& list)
		{
			bool pending = false;

			for (auto& sec : list)
			{
				if (sec->book.ts == 0)
					return false;

				if (sec->book.ts == signal.ts)
					pending = true;
			}

			return pending;
		}

		bool LastPending(const std::vector<Security*>& list)
		{
			bool pending = false;

			for (auto& sec : list)
			{
				if (sec->last_ts == 0)
					return false;

				if (sec->last_ts == signal.ts)
					pending = true;
			}

			return pending;
		}

		std::string __repr__()
		{
			std::stringstream ss;
			ss << "signal: {" << std::endl << signal.__repr__("  ") << "}" << std::endl;

			ss << "orders: ";
			for (auto& order : orders)
				ss << "{" << std::endl << order.__repr__("  ") << "}" << std::endl;

			if (orders.empty())
				ss << std::endl;

			ss << "securities: ";
			for (auto& sec : securities)
				ss << "{" << std::endl << sec->__repr__("  ") << "}" << std::endl;

			if (securities.empty())
				ss << std::endl;

			return ss.str();
		}
	};

	class ScopedState
	{
	private:

		State *state_;

	public:

		ScopedState(State* state)
			: state_(state)
		{
			state_->lock();
		}

		~ScopedState()
		{
			state_->unlock();
		}

		State* operator->()
		{
			return state_;
		}

		operator State*()
		{
			return state_;
		}
	};
}
