#pragma once
#include <entt/entt.hpp>
#include "utility.h"
#include <mutex>
#include <cstdio>

/*
 * on event _event_ create component _T_ with event as argument
 */

#define MIRAGE_CREATE_WITH_EVENT(_event_, _T_) \
	namespace __static__##_T_##_event_{ \
	MIRAGE_ON_STARTUP(createWithEvent, ( \
	::mirage::event::dispatcher().sink<_event_>() \
	.connect<&::mirage::ecs::_createStubArg<_T_, _event_>>()))};
/*
 * on event _event_ create component _T_.
 */
#define MIRAGE_CREATE_ON_EVENT(_event_, _T_) \
	namespace __static__##_T_##_event_{ \
	MIRAGE_ON_STARTUP(createOnEvent, ( \
	::mirage::event::dispatcher().sink<_event_>() \
	.connect<&::mirage::ecs::_createStub<_T_>>()))};

namespace mirage::event
{
	MIRAGE_COFU(entt::dispatcher, dispatcher);	
	MIRAGE_COFU(std::mutex, lock);

	template<typename T, typename... Args>
	inline void enqueueEvent(Args&&... args)
	{	
		dispatcher().enqueue<T>(args...);	
	}

	template<typename T, typename... Args>
	inline void triggerEvent(Args&&... args)
	{	
		dispatcher().trigger(T{args...});
	}
}

