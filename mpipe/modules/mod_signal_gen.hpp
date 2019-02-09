#pragma once

#include <core/mpipe.hpp>
#include <util/numeric.hpp>

namespace mpipe
{
	class ModSignalGen : public Module
	{
	private:		
		
		Action action_;
		Side side_;

	public:

		ModSignalGen(Action action, Side side)
			: action_(action)
			, side_(side)
		{
		}

		void Mutate(State* state) override
		{
			if (BookGood(state->securities))
			{
				state->signal.action = action_;
				state->signal.side = side_;
			}
		}
	};
}
