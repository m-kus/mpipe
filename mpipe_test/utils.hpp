#pragma once

#include <core/mpipe_security.hpp>
#include <core/mpipe_signal.hpp>
#include <mcast/mcast_book.hpp>
#include <util/time.hpp>

using namespace mpipe;

inline Security MakeSecurity(
	std::int64_t leg_factor = 1,
	int64_t position = 0,
	const char* full_code = "sec",
	const char* class_code = "test")
{
	Security security{};
	security.class_code = class_code;
	security.full_code = full_code;
	security.leg_factor = leg_factor;
	security.pos.position = position;

	if (position > 0)
	{
		security.pos.buy_chunks.emplace_back(time_now(), npabs(position));
	}
	else
	if (position < 0)
	{
		security.pos.sell_chunks.emplace_back(time_now(), npabs(position));
	}

	return security;
}

inline Signal MakeOpenLongNow()
{
	Signal signal;
	signal.action = Action::Open;
	signal.side = Side::Buy;
	signal.ts = time_now();

	return signal;
}

inline McastBookUpdate MakeUpdate(
	std::string full_code,
	McastUpdateAction action,
	McastEntryType type,
	std::int64_t px,
	std::int64_t qty,
	std::int64_t rpt_seq)
{
	McastBookUpdate update{};
	update.full_code = full_code;
	update.action = action;
	update.type = type;
	update.ts = time_now();
	update.px = px;
	update.qty = qty;
	update.rpt_seq = rpt_seq;

	return update;
}

struct UpdatesCounterBook : public McastBook
{
	std::int64_t counter;

	UpdatesCounterBook(Security* security, std::int64_t tolerance = 0)
		: McastBook(security, tolerance)
		, counter(0)
	{
	}

	void UpdateImpl(const McastBookUpdate& update) override
	{
		counter++;
	}

	void ForceOnline(std::int64_t rpt_seq)
	{
		status_.store(McastBookStatus::Online);
		rpt_seq_ = rpt_seq;
	}
};
