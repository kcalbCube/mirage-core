#pragma once
#include <thread>
#include <chrono>
#include "ecs.h"
#include <iostream>
namespace mirage::ecs::processing
{
	template<typename DeltaType_>
	class Processor
	{
	public:	
		using DeltaType = DeltaType_;	
		entt::scheduler<DeltaType_> scheduler;	
		
		virtual ~Processor(void);
	};

	template<unsigned Milliseconds>
	class PeriodMS : public Processor<unsigned>
	{
		PeriodMS(void) { start(); }
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

	template<unsigned Delim=100>
	void doAfter(unsigned milliseconds, std::function<void(void)> function);

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
		template<typename Derived, typename DeltaType>
		using Process = entt::process<Derived, DeltaType>;	

		template<typename ProcessT, typename DeltaType, typename... Args>
		void startProcess(processing::Processor<DeltaType>& processor, Args&&... args)
		{	
			processor.scheduler.template attach<ProcessT>(args...);
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

template<unsigned Delim>
inline void mirage::ecs::processing::doAfter(
	unsigned milliseconds, 
	std::function<void(void)> function)
{
	if(milliseconds < Delim)
	{
		if(Delim <= 1)
		{
			abort();
		}

		doAfter<Delim / 10>(milliseconds, function);
		return;
	}

	class Process : public entt::process<Process, unsigned>
	{
		std::function<void(void)> function;
		unsigned forSucceed = 0, counter = 0;
	public:
		Process(std::function<void(void)> function_, unsigned forSucceed_)
			: function { std::move(function_) },
			forSucceed { forSucceed_ } {}

		void update(unsigned delta, void*)
		{
			if(counter++ > forSucceed)
				entt::process<Process, unsigned>::succeed();
		}

		void succeeded(void)
		{
			function();
		}
	};

	PeriodMS<Delim>::getInstance().scheduler
		.template attach<Process>(function, milliseconds / Delim);
}

template<typename DeltaType_>
inline mirage::ecs::processing::Processor<DeltaType_>::~Processor(void) 
{
	scheduler.clear();
}

template<unsigned Milliseconds>
inline void mirage::ecs::processing::PeriodMS<Milliseconds>::start(void)
{
	thread = std::jthread([this](std::stop_token itoken) -> void
	{
		while(!itoken.stop_requested())
		{
			auto start = std::chrono::high_resolution_clock::now();
			scheduler.update(Milliseconds);
			std::this_thread::sleep_until(start +
				std::chrono::milliseconds(Milliseconds));
		};
	});

	thread.detach();
}
