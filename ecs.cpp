#include "ecs.h"
#include <mutex>
#include <entt/entt.hpp>

namespace mirage::ecs
{
	MIRAGE_COFU_CPP(entt::registry, registry);
	MIRAGE_COFU_CPP(std::mutex, lateQueueLock);
	MIRAGE_COFU_CPP(std::deque<boost::function<void(void)>>, lateQueue);	
}

void mirage::ecs::lateQueueUpdate(void)
{
	if(lateQueue().empty())
		return;

	std::lock_guard lock(lateQueueLock());
	for(auto&& f : lateQueue())
		f();
	lateQueue().clear();
}