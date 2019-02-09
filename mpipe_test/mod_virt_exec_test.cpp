#include <modules/mod_virt_exec.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

struct PosCheckModule : public Module
{
	void Mutate(State* state) override
	{
		state->signal.action = Action::Terminate;
	}
};

TEST(MpipeModuleTest, ModVirtExecTest)
{
	Security sec = MakeSecurity(1, 0);
	State state({ 
		{ "sec", &sec } 
	});
	PosCheckModule pos_check;
	ModVirtExec virt_exec(".trades");
	virt_exec.Start(&state, { &pos_check });

	state.signal = MakeOpenLongNow();
	state.orders.emplace_back(1, Side::Buy, &sec);

	{
		ScopedState lock(&state);
		virt_exec.Mutate(&state);
	}

	virt_exec.Join();

	ASSERT_EQ(sec.pos.position, 1);
}
