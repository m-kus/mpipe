#include <core/mpipe.hpp>
#include <gtest/gtest.h>

#include <future>

using namespace mpipe;

struct CounterModule : public Module
{
	int counter;
	bool stopped;

	CounterModule()
		: counter(0)
		, stopped(false)
	{
	}

	void Mutate(State* state) override
	{
		counter++;
		state->signal.action = Action::Terminate;
	}

	void StopImpl() override
	{
		stopped = true;
	}
};

TEST(MpipeTest, ModuleMutatedStopped)
{
	State state({});
	Module active;
	CounterModule counter;
	active.Start(&state, { &counter });
	active.ExecutePipe(time_now());

	ASSERT_EQ(counter.counter, 1);
	ASSERT_TRUE(counter.stopped);
}

TEST(MpipeTest, SignalTerminateNotExecuted)
{
	State state({});
	Module active;
	CounterModule counter;

	state.signal.action = Action::Terminate;
	active.Start(&state, { &counter });

	ASSERT_EQ(counter.counter, 0);
}

TEST(MpipeTest, SignalSignalReset)
{
	State state({});
	Module active;
	CounterModule counter;

	state.signal.action = Action::Open;
	state.signal.side = Side::Buy;
	state.orders.emplace_back(0, Side::Buy, nullptr);

	active.Start(&state, {});
	active.ExecutePipe(time_now());

	ASSERT_EQ(state.signal.action, Action::Bypass);
	ASSERT_EQ(state.signal.side, Side::All);
	ASSERT_TRUE(state.orders.empty());
}

TEST(MpipeTest, ModuleJoin)
{
	Module active;

	auto ret = std::async(std::launch::async, [&] {
		sleep_mcs(10000);
		active.Stop();
		return true;
	});	

	active.Join();
	ASSERT_TRUE(ret.get());
}
