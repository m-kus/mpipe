#include <modules/mod_hours.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>


TEST(MpipeModuleTest, ModHoursTestBypass)
{
	State state({});
	ModHours hours({});

	state.signal = MakeOpenLongNow();

	hours.Mutate(&state);
	ASSERT_EQ(state.signal.action, Action::Bypass);
}
