#pragma once
#include <ctime>
#include <thread>
#include <chrono>
#include "ecs.h"
#include <iostream>
#include "utility.h"
#include <map>
#include <boost/core/demangle.hpp>

namespace mirage::ecs::processing
{
	struct Process
	{	
		enum class State
		{
			running,
			paused,
			failed,
			terminated,
			succeeded
		} state = State::running;
		clock_t last = 0;
		int priority = 0; // less - earlier

		struct Exit {};

		void fail(void)
		{
			state = State::failed;
			throw Exit{};
		}

		void pause(void)
		{
			state = State::paused;
			throw Exit{};
		}

		void unpause(void)
		{
			state = State::running;
		}

		void succeed(void)
		{
			state = State::succeeded;
			throw Exit{};
		}
	
		void terminate(void) noexcept
		{
			state = State::terminated;
		}

		// Not virtual, CRTP.
		void initialize(void) {}

		virtual void onFail(void)    {}	
		virtual void onSucceed(void) {}
		virtual void update(float delta) = 0;

		virtual ~Process(void) {}

		friend bool operator<(Process& l, Process& r) { return l.priority < r.priority; }
	};

	class Processor
	{
		std::map<entt::id_type, std::unique_ptr<Process>> processes;
		std::mutex mutex;

		entt::id_type i = 0;
	public:

		/*
		 * @returns descriptor of the attached process
		 */
		template<typename ProcessT, typename... Args>
		entt::id_type attach(Args&&... args)
		{
			std::lock_guard guard{mutex};

			auto& process = *processes.emplace(i, std::make_unique<ProcessT>()).first->second;
			static_cast<ProcessT&>(process).initialize(args...);

			return i++;
		}

		// terminate a process, will not stop already updating process, then wait a tick.
		void terminate(entt::id_type id)
		{
			std::lock_guard guard{mutex};

			if(!processes.contains(id))
				return;

			processes[id]->terminate();
		}

		void update(void)
		{
			if(processes.empty())
				return;

			std::lock_guard guard{mutex};

			std::erase_if(processes,
				[&](auto& pair) -> bool
				{
					auto& process = pair.second;

					switch(process->state)
					{
					case Process::State::running:
						try
						{
							clock_t now = clock();
							float passed;

							if(now < process->last) // clock_t overflow
							{
								if constexpr (sizeof(clock_t) == 4)
									passed = static_cast<float>(static_cast<uint64_t>(UINT32_MAX) + now) / (CLOCKS_PER_SEC * 1000.f);
								else
									passed = now / (CLOCKS_PER_SEC * 1000.f);
							}
							else [[likely]]
								passed = static_cast<float>(now - process->last) / (CLOCKS_PER_SEC * 1000.f);	

							process->update(passed);
							process->last = now;
						}
						catch(Process::Exit) {}
						catch(std::exception& exception)
						{
							process->fail();
							loge("Exception in process \"{}\": {}, process terminated", // TODO: boost::stacktrace
								boost::core::demangle(typeid(*process).name()),
								exception.what());
						}
						break;

					case Process::State::succeeded:
						process->onSucceed();
						return true;

					case Process::State::failed:
						process->onFail();
						return true;

					case Process::State::terminated:
						return true;

					case Process::State::paused:	
					default:
						break;	
					}

					return false;
				});
		}

		virtual void start(void) = 0;
		virtual void stop(void) = 0;

		virtual ~Processor(void) {}
	};

	template<unsigned Milliseconds>
	class PeriodMS : public Processor
	{
		PeriodMS(void)
		{
			start();
		}
		std::jthread thread;
	public:

		void start(void);	

		void stop(void) { thread.request_stop(); }

		static PeriodMS<Milliseconds>& getInstance(void)
		{
			static PeriodMS<Milliseconds> instance;
			return instance;
		}
	};	

	class EventDispatcherProcessing : public ecs::Component<EventDispatcherProcessing>
	{
		std::jthread thread;
		static std::atomic_bool stopped;
	public:
		static constexpr auto updatePeriod = 40u;
		void initialize(void);

		void onDestroy(void);
	};

	inline std::atomic_bool EventDispatcherProcessing::stopped{false};
	MIRAGE_CREATE_ON_STARTUP(EventDispatcherProcessing, eventProcessing);
}

namespace mirage::ecs
{
	template<typename T>
	struct Processing
	{
		template<typename Derived>
		struct Process : processing::Process
		{
			ComponentWrapper<T> parent;
			
			void initialize(entt::entity parent_)
			{
				parent = parent_;
			}
		};

		template<typename ProcessT, typename... Args>
			requires (!std::derived_from<ProcessT, Process<ProcessT>>)
		void startProcess(processing::Processor& processor, Args&&... args)
		{	
			processor.template attach<ProcessT>(args...);
		}

		template<typename ProcessT, typename... Args>
			requires std::derived_from<ProcessT, Process<ProcessT>>
		void startProcess(processing::Processor& processor, Args&&... args)
		{
			processor.template attach<ProcessT>(
				static_cast<T*>(this)->entity, args...);

		}
	};
}

inline void mirage::ecs::processing::EventDispatcherProcessing::initialize(void)
{
	thread = std::jthread([](std::stop_token st) -> void
	{
		while(!st.stop_requested() && !event::dispatcher.destructed)
		{	
			event::dispatcher().update();	
			lateQueueUpdate();	
			std::this_thread::sleep_for(std::chrono::milliseconds(updatePeriod));
		}
		stopped.store(true);
	});

	thread.detach();
}

inline void mirage::ecs::processing::EventDispatcherProcessing::onDestroy()
{
	thread.request_stop();
	stopped.wait(false);
}

template<unsigned Milliseconds>
inline void mirage::ecs::processing::PeriodMS<Milliseconds>::start(void)
{
	thread = std::jthread([this](std::stop_token itoken) -> void
	{
		while(!itoken.stop_requested())
		{
			auto start = std::chrono::high_resolution_clock::now();
			update();
			std::this_thread::sleep_until(start +
				std::chrono::milliseconds(Milliseconds));
		};
	});

	thread.detach();
}
