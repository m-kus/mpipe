#pragma once

#include <core/mpipe.hpp>
#include <util/numeric.hpp>

#include <algorithm>
#include <list>

namespace mpipe
{
	class ModBalancer : public Module
	{
	private:

		struct Imbalance
		{
			Security *sec;
			std::int64_t pos;
			std::int64_t common_pos;

			Imbalance(Security* _sec)
				: sec(_sec)
				, pos(_sec->pos.position)
				, common_pos(_sec->pos.position / _sec->leg_factor)
			{
			}

			bool operator <(const Imbalance& right)
			{
				return npabs(common_pos) < npabs(right.common_pos);
			}
		};

	public:

		void Mutate(State* state) override
		{
			std::vector<Imbalance> imbalance;
			imbalance.reserve(state->securities.size());

			for (auto& sec : state->securities)
			{
				if (sec->leg_factor != 0)
				{
					if (sec->pos.buy_limit != 0 || sec->pos.sell_limit != 0)
						return;

					imbalance.emplace_back(sec);
				}
			}

			auto minmax = std::minmax_element(imbalance.begin(), imbalance.end());
			if (minmax.first->common_pos == minmax.second->common_pos)
				return;

			auto target = state->signal.action == Action::Open ? minmax.second : minmax.first;
			state->orders.clear();

			for (auto& i : imbalance)
			{
				std::int64_t delta = target->common_pos * i.sec->leg_factor - i.sec->pos.position;
				if (delta != 0)
					state->orders.emplace_back(npabs(delta), (Side)npsign(delta), i.sec);
			}

			if (!state->orders.empty()
				&& state->signal.action != Action::Open)
			{
				state->signal.action = Action::Close;
			}
		}
	};
}
