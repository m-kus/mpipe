#pragma once

#include <mcast/mcast_book.hpp>

#include <unordered_map>
#include <functional>
#include <atomic>
#include <memory>

namespace mpipe
{	
	typedef std::unordered_map<std::string, McastBook*> BookMap;

	class McastFeed : public Module
	{
	protected:

		BookMap books_;
		bool snapshot_;
		std::atomic_bool recover_;

	public:

		McastFeed(const std::vector<McastBook*>& books, bool snapshot = false)
			: snapshot_(snapshot)
			, recover_(false)
		{
			for (auto book : books)
			{
				books_[book->security->full_code] = book;
				if (snapshot_)
					book->Register(this);
			}
		}

		virtual ~McastFeed() {}

		void Start(State* state, const std::vector<Module*>& pipe) override
		{
			pipe_ = pipe;
			state_ = state;
			assert(state_);
			
			if (snapshot_)
			{
				bool allowed = false;
				if (recover_.compare_exchange_strong(allowed, true))
				{
					for (auto& book : books_)
						book.second->Reset();

					StartImpl();
				}
			}
			else
			{
				StartImpl();
			}
		}
	
		void UpdateBegin(std::int64_t ts)
		{
			for (auto& book : books_)
			{
				if (snapshot_)
				{
					book.second->SnapshotUpdateBegin(ts);
				}
				else
				{
					book.second->UpdateBegin(ts);
				}
			}
		}

		void Update(const McastBookUpdate& update)
		{
			auto book = books_.find(update.full_code);
			if (book != books_.end())
			{
				if (snapshot_)
				{
					book->second->SnapshotUpdate(update);
				}
				else
				{
					book->second->Update(update);
				}
			}
		}

		void UpdateEnd(std::int64_t ts)
		{
			for (auto& book : books_)
			{
				if (snapshot_)
				{
					book.second->SnapshotUpdateEnd(ts);
				}
				else
				{
					book.second->UpdateEnd(ts);
				}
			}

			if (recover_.load())
			{
				if (std::all_of(books_.begin(), books_.end(), 
					[](typename BookMap::const_reference book) { return book.second->Online(); }))
				{
					Stop();
					recover_.store(false);
					log_format("Recover end");
				}				
			}
		}

		void ApplyUpdates(const std::vector<McastBookUpdate>& updates)
		{
			ScopedState state(GetState());
			UpdateBegin(time_now());
			
			for (auto& update : updates)
				Update(update);

			UpdateEnd(time_now());
			ExecutePipe(time_now());
		}
	};
}
