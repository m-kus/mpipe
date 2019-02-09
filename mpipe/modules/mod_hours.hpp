#pragma once

#include <core/mpipe.hpp>
#include <util/logger.hpp>
#include <util/lazy.hpp>

#include <vector>
#include <queue>
#include <ctime>
#include <locale>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <ctime>
#include <limits>

#undef max

namespace mpipe 
{
	struct ModHoursRule : public SchedulerEvent
	{
		Side side;
		Action rule;

		ModHoursRule(const std::string& _time, Side _side, Action _rule)
			: SchedulerEvent(SchedulerEventType::Daily, _time)
			, side(_side)
			, rule(_rule)
		{
		}
	};

	class ModHours : public Module, public LazyScheduler
	{
	private:

		std::vector<ModHoursRule> rules_;
		Action long_rule_;
		Action short_rule_;

	private:

		void ApplyRule(ModHoursRule* e)
		{
			switch (e->side)
			{
				case Side::Buy:
				{
					long_rule_ = e->rule;
					break;
				}
				case Side::Sell:
				{
					short_rule_ = e->rule;
					break;
				}
				case Side::All:
				{
					long_rule_ = e->rule;
					short_rule_ = e->rule;
					break;
				}
				default: break;
			}			
		}

	public:

		ModHours(std::vector<ModHoursRule> rules)
			: rules_(rules)
			, long_rule_(Action::Bypass)
			, short_rule_(Action::Bypass)
		{
			for (auto& rule : rules_)
				AddEvent(&rule);
		}

		void Mutate(State* state) override
		{
			if (state->signal.ts >= next_ts_)
			{
				ModHoursRule *rule = nullptr;
				while (GetEvent(state->signal.ts, &rule))
					ApplyRule(rule);

				if (rule != nullptr)
					log_format("long: %s, short: %s", action_str(long_rule_), action_str(short_rule_));
			}

			if (state->signal.side == Side::Buy
				|| state->signal.side == Side::All)
			{
				if (state->signal.action > long_rule_)
					state->signal.action = long_rule_;
			}

			if (state->signal.side == Side::Sell
				|| state->signal.side == Side::All)
			{
				if (state->signal.action > short_rule_)
					state->signal.action = short_rule_;
			}

			if (state->signal.action == Action::ForceClose)
				state->signal.side = Side::All;
		}

		void ForceClose(std::int64_t min_period)
		{
			next_ts_ += min_period;

			ModHoursRule force_close("", Side::All, Action::ForceClose);
			ApplyRule(&force_close);
		}
	};
}
