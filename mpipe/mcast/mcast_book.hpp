#pragma once

#include <list>
#include <atomic>

#include <mcast/mcast_types.hpp>
#include <core/mpipe_security.hpp>
#include <util/time.hpp>
#include <util/logger.hpp>
#include <util/concurrent.hpp>

namespace mpipe
{
	class McastBook
	{
	protected:

		Module *snapshot_feed_;

		std::atomic<McastBookStatus> status_;
		std::list<McastBookUpdate> updates_;
		spin_lock lock_;

		std::size_t tolerance_;
		std::int64_t rpt_seq_;

	public:

		Security *security;
		
	public:

		McastBook(Security* _security, std::size_t tolerance = 0)
			: snapshot_feed_(nullptr)
			, status_(McastBookStatus::RecoverFree)
			, tolerance_(tolerance)
			, rpt_seq_(0)
			, security(_security)
		{
		}

		virtual ~McastBook() {}
		virtual void ResetImpl(std::int64_t ts) {}
		virtual bool OnlineImpl() { return true; }
		virtual void UpdateImpl(const McastBookUpdate& update) {}
		virtual void UpdateBegin(std::int64_t ts) {}
		virtual void UpdateEndImpl(std::int64_t ts) {}

		void Register(Module* feed)
		{
			snapshot_feed_ = feed;
			status_.store(McastBookStatus::RecoverWait);
		}
		
		bool Online()
		{
			switch (status_.load(std::memory_order_acquire))
			{
				case McastBookStatus::Online: return true;
				case McastBookStatus::RecoverFree: return OnlineImpl();
				default: return false;
			}
		}

		void Reset()
		{
			if (snapshot_feed_)
				status_.store(McastBookStatus::RecoverWait);	

			rpt_seq_ = 0;
			ResetImpl(time_now());
		}

		void ApplyUpdates()
		{
			std::lock_guard<spin_lock> lock(lock_);
			updates_.sort();

			while (!updates_.empty()
				&& updates_.front().rpt_seq <= rpt_seq_)
			{
				updates_.pop_front();
			}
			
			while (!updates_.empty()
				&& updates_.front().rpt_seq <= rpt_seq_ + 1)
			{
				UpdateImplProxy(updates_.front());
				updates_.pop_front();
			}

			if (status_.load() == McastBookStatus::RecoverApply)
				updates_.clear();

			if (updates_.size() > tolerance_
				&& snapshot_feed_)
			{
				log_format(
					"%s: current rpt_seq is %lld, HEAD is %lld, recover", 
					security->full_code.c_str(), 
					rpt_seq_,
					updates_.front().rpt_seq
				);
				snapshot_feed_->Start(snapshot_feed_->GetState(), {});
			}
		}
						
		void UpdateImplProxy(const McastBookUpdate& update)
		{
			switch (update.type)
			{
				case McastEntryType::Empty: 
				{  
					ResetImpl(update.ts);
					break;
				}
				default: 
				{
					UpdateImpl(update);
					break;
				}
			}

			rpt_seq_ = update.rpt_seq;
		}

		void Update(const McastBookUpdate& update)
		{
			switch (status_.load())
			{
				case McastBookStatus::RecoverFree:
				{
					if (update.rpt_seq >= rpt_seq_)
						UpdateImplProxy(update);
					break;
				}
				case McastBookStatus::Online:
				{
					if (update.rpt_seq == rpt_seq_
						|| update.rpt_seq == rpt_seq_ + 1)
					{
						UpdateImplProxy(update);
						break;
					}
					__fallthrough;
				}
				default:
				{
					std::lock_guard<spin_lock> lock(lock_);
					updates_.push_back(update);
					break;
				}
			}
		}

		void UpdateEnd(std::int64_t ts) 
		{
			if (status_.load() == McastBookStatus::Online)
				ApplyUpdates();

			UpdateEndImpl(ts);
		}

		void SnapshotUpdateBegin(std::int64_t ts)
		{
			McastBookStatus allowed = McastBookStatus::RecoverWait;
			status_.compare_exchange_strong(allowed, McastBookStatus::RecoverCollect);
		}

		void SnapshotUpdate(const McastBookUpdate& update)
		{
			if (status_.load() == McastBookStatus::RecoverCollect)
				UpdateImplProxy(update);
		}

		void SnapshotUpdateEnd(std::int64_t ts)
		{
			McastBookStatus allowed = McastBookStatus::RecoverCollect;
			if (status_.compare_exchange_strong(allowed, McastBookStatus::RecoverApply))
			{
				ApplyUpdates();
				status_.store(McastBookStatus::Online);
			}
		}
	};
}
