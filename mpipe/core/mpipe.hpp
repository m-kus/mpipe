#pragma once

#include <core/mpipe_state.hpp>
#include <util/exception.hpp>
#include <util/seh.hpp>

#include <condition_variable>
#include <mutex>
#include <chrono>

namespace mpipe 
{
	class Module
	{
	protected:

		State *state_;
		std::vector<Module*> pipe_;
		std::mutex join_mutex_;
		std::condition_variable stopped_cv_;
		bool stopped_;

	public:

		virtual void Mutate(State* state) {}
		virtual void StartImpl() {}
		virtual void StopImpl() {}
		virtual ~Module() {}

		Module()
			: state_(nullptr)
			, stopped_(false)
		{
			SehTranslationEnable();
		}

		virtual void Start(State* state, const std::vector<Module*>& pipe)
		{
			state_ = state;
			pipe_ = pipe;
			StartImpl();
		}

		void Stop()
		{
			StopImpl();
			{
				std::lock_guard<std::mutex> lock(join_mutex_);
				stopped_ = true;
			}
			stopped_cv_.notify_all();
		}

		void Join()
		{
			std::unique_lock<std::mutex> lock(join_mutex_);
			while (!stopped_)
				stopped_cv_.wait(lock);
		}
		
		State* GetState()
		{
			if (state_ == nullptr)
				throw mpipe_error_message("start has not been called");

			return state_;
		}

		void ShutdownPipe()
		{
			log_format("Shutdown pipe\r\n");
			Stop();
			for (auto& mod : pipe_)
				mod->Stop();
		}

		void ExecutePipe(std::int64_t ts, bool reset = true)
		{
			assert(ts);
			assert(state_);

			if (state_->signal.action == Action::Terminate)
				return;

			if (reset)
				state_->signal.Reset();

			state_->signal.ts = ts;
			state_->orders.clear();

			for (auto mod : pipe_)
			{
				try 
				{
					mod->Mutate(state_);
				}
				catch (std::exception& e)
				{
					log_format(e.what());
					state_->signal.action = Action::Terminate;
				}

				if (state_->signal.action == Action::Terminate)
				{
					ShutdownPipe();
					return;
				}
			}
		}
	};
}
