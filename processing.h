#pragma once
#include <ctime>
#include <ratio>
#include <thread>
#include <chrono>
#include "ecs.h"
#include <iostream>
#include "utility.h"
#include <map>
#include <boost/core/demangle.hpp>
#include "mirage.h"

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
		uint8_t priority = 0; // greater - earlier

		struct Exit {};

		void fail(void);

		void pause(void);

		void unpause(void);

		void succeed(void);

		void terminate(void) noexcept;

		// Not virtual, CRTP.
		void initialize(void);

		virtual void onFail(void);
		virtual void onSucceed(void);
		virtual void update(float delta) = 0;

		virtual ~Process(void) {}

		friend bool operator<(Process& l, Process& r) { return l.priority < r.priority; }
	};

	class Processor
	{
		std::shared_ptr<std::map<entt::id_type, std::unique_ptr<Process>>> processes;
		std::shared_ptr<std::mutex> mutex;

		entt::id_type i = 0;
	public:

		Processor(void);

		/*
		 * @returns descriptor of the attached process
		 */
		template<typename ProcessT, typename... Args>
		entt::id_type attach(Args&&... args)
		{
			std::lock_guard guard{*mutex};

			auto ptr = std::make_unique<ProcessT>();
			ptr->initialize(args...);

			auto index = (ptr->priority << 24) | i++;
			processes->emplace(index, std::move(ptr));

			return index;
		}

		// terminate a process, will not stop already updating process, then wait a tick.
		void terminate(entt::id_type id);

		void update(void);

		//virtual void start(void) = 0;
		virtual void stop(void) = 0;

		virtual ~Processor(void);
		};

	template<unsigned Milliseconds>
	class PeriodMS 
		: public Component<PeriodMS<Milliseconds>>,
		  public Processor,
		  public Singleton<PeriodMS<Milliseconds>>
	{	
		std::shared_ptr<std::jthread> thread;
	public:

		void initialize(void);	
		void stop(void) { thread->request_stop(); }	
	};


	// Processor, which period depends on tickrate.	
	template<typename Ratio=std::ratio<1,1>> // C++20 allows float parameter, but just gcc support that.
	class TickProcessor 
		: public Component<TickProcessor<Ratio>>,
		  public Processor,
		  public Singleton<TickProcessor<Ratio>>
	{
		std::shared_ptr<std::jthread> thread;
	public:

		void initialize(void);
		void stop(void) { thread->request_stop(); }
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

template<unsigned Milliseconds>
void mirage::ecs::processing::PeriodMS<Milliseconds>::initialize(void)
{
	thread = std::make_shared<std::jthread>([this](std::stop_token itoken) -> void
	{
		while(!itoken.stop_requested())
		{
			auto start = std::chrono::high_resolution_clock::now();
			update();
			std::this_thread::sleep_until(start +
				std::chrono::milliseconds(Milliseconds));
		};
	});

	thread->detach();
}

template<typename Ratio>
void mirage::ecs::processing::TickProcessor<Ratio>::initialize(void)
{
	thread = std::make_shared<std::jthread>([this](std::stop_token itoken) -> void
	{
		while(!itoken.stop_requested())
		{
			auto start = std::chrono::high_resolution_clock::now();
			update();
			std::this_thread::sleep_until(start +
				std::chrono::milliseconds(static_cast<long>(tickrate().getPeriod() * static_cast<double>(Ratio::den))));
		};
	});

	thread->detach();
}