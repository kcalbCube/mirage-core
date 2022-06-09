#pragma once

#define MIRAGE_ON_STARTUP(id, f) \
	namespace __mirage_static \
	{ \
	inline struct id##onStart { id##onStart(void) { f; } } id##onStartupInstance; \
	};

#define MIRAGE_CREATE_ON_STARTUP(_T_, id) \
	MIRAGE_ON_STARTUP(_create##id, (void)::mirage::ecs::create<_T_>())
