#include "event.h"

namespace mirage::event
{
	MIRAGE_COFU_CPP(entt::dispatcher, dispatcher);	
	MIRAGE_COFU_CPP(std::mutex, lock);
}