#pragma once

#include <core/mpipe.hpp>
#include <core/mpipe_security.hpp>
#include <core/mpipe_enum.hpp>
#include <util/numeric.hpp>

#include <functional>

#ifndef _WIN32
#define __fallthrough
#endif

namespace mpipe
{
	enum class McastBookStatus
	{
		Online,
		RecoverFree,
		RecoverWait,
		RecoverCollect,
		RecoverApply
	};

	enum class McastUpdateAction
	{
		New,
		Change,
		Delete,
		Trade,
		Heartbeat
	};

	enum class McastEntryType
	{
		Bid,
		Ask,
		Buy,
		Sell,
		Empty
	};

	inline const char* status_str(McastBookStatus status)
	{
		switch (status)
		{
			case McastBookStatus::Online: return "online";
			case McastBookStatus::RecoverFree: return "recover_free";
			case McastBookStatus::RecoverWait: return "recover_wait";
			case McastBookStatus::RecoverCollect: return "recover_collect";
			case McastBookStatus::RecoverApply: return "recover_apply";
			default: return "";
		}
	}

	inline const char* update_action_str(McastUpdateAction action)
	{
		switch (action)
		{
			case McastUpdateAction::New: return "new";
			case McastUpdateAction::Change: return "change";
			case McastUpdateAction::Delete: return "delete";
			case McastUpdateAction::Trade: return "trade";
			case McastUpdateAction::Heartbeat: return "heartbeat";
			default: return "";
		}
	}

	inline const char* entry_type_str(McastEntryType type)
	{
		switch (type)
		{
			case McastEntryType::Bid: return "bid";
			case McastEntryType::Ask: return "ask";
			case McastEntryType::Buy: return "buy";
			case McastEntryType::Sell: return "sell";
			case McastEntryType::Empty: return "empty";
			default: return "";
		}
	}

	struct McastBookUpdate
	{
		std::string full_code;
		McastUpdateAction action;
		McastEntryType type;
		std::int64_t ts;
		std::int64_t px;
		std::int64_t qty;
		std::int64_t rpt_seq;
		std::int64_t entry_id;
		
		std::string __repr__()
		{
			std::stringstream ss;
			ss << "full_code: " << full_code << std::endl;
			ss << "action: " << update_action_str(action) << std::endl;
			ss << "type: " << entry_type_str(type) << std::endl;
			ss << "ts: " << ts_format(ts) << std::endl;
			ss << "px: " << px << std::endl;
			ss << "qty: " << qty << std::endl;
			ss << "rpt_seq: " << rpt_seq << std::endl;
			ss << "entry_id: " << entry_id << std::endl;

			return ss.str();
		}

		bool operator <(const McastBookUpdate& update)
		{
			return rpt_seq < update.rpt_seq;
		}
	};
}
