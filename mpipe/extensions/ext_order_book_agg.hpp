#pragma once

#include <mcast/mcast_extension.hpp>

namespace mpipe
{
	class ExtOrderBookAgg : public McastExtension
	{
	protected:

		void Change(const McastBookUpdate& upd)
		{
			switch (upd.type)
			{
				case McastEntryType::Bid:
				{
					host->security->book.TrySetBid(upd.px, upd.qty);
					break;
				}
				case McastEntryType::Ask:
				{
					host->security->book.TrySetAsk(upd.px, upd.qty);
					break;
				}
				default: break;
			}
		}

		void Delete(const McastBookUpdate& upd)
		{
			switch (upd.type)
			{
				case McastEntryType::Bid: __fallthrough;
				case McastEntryType::Ask: 
				{
					host->security->book.Delete(upd.px);
					break;
				}
				default: break;
			}
		}

	public:
		
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
			switch (upd.action)
			{
				case McastUpdateAction::New: __fallthrough;
				case McastUpdateAction::Change: Change(upd); break;
				case McastUpdateAction::Delete: Delete(upd); break;
				default: break;
			}
		}

		void UpdateEnd(std::int64_t ts) override
		{
			host->security->book.Commit(ts);
		}
	};
}
