#pragma once

#include <mcast/mcast_extension.hpp>

namespace mpipe
{
	class ExtOrderBookTop : public McastExtension
	{
	public:

		using McastExtension::McastExtension;

		bool Online() override
		{
			return host->security->book.Valid();
		}

		void Reset(std::int64_t ts) override
		{
			host->security->book.Reset();
		}

		void Update(const McastBookUpdate& upd) override
		{
			switch (upd.type)
			{
				case McastEntryType::Bid: break;
				case McastEntryType::Ask: break;
				default: return;
			}
			
			switch (upd.type)
			{
				case McastEntryType::Bid: host->security->book.SetBid(upd.px, upd.qty); break;
				case McastEntryType::Ask: host->security->book.SetAsk(upd.px, upd.qty); break;
				default: break;
			}
		}

		void UpdateEnd(std::int64_t ts) override
		{
			host->security->book.Commit(ts);
		}
	};
}
