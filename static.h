#pragma once

#define MIRAGE_ON_STARTUP(id, f) \
	namespace __mirage_static \
	{ \
	inline struct id##onStart { id##onStart(void) { f; } } id##onStartupInstance; \
	};

#define MIRAGE_CREATE_ON_STARTUP(_T_, id) \
	MIRAGE_ON_STARTUP(_create##id, (void)::mirage::ecs::create<_T_>())
	
#define MIRAGE_COFU(T, name, ...) \
	inline struct _##name##cofu { T* instance = nullptr; T& operator()(void) { if(!instance) instance = new T(__VA_ARGS__); \
	return *instance; }; static bool destructed; ~_##name##cofu(void) { destructed = true; if(instance) delete instance;} \
	static bool isDestructed(void) { return destructed; } } name; inline bool _##name##cofu::destructed = false

#define MIRAGE_COFU_H(T, name) \
	extern struct _##name##cofu { T* instance = nullptr; T& operator()(void); static bool destructed; ~_##name##cofu(void); \
	static bool isDestructed(void);} name

#define MIRAGE_COFU_CPP(T, name, ...) \
	_##name##cofu name; bool _##name##cofu::destructed = false; \
	T& _##name##cofu::operator()(void) { if(!instance) instance = new T(__VA_ARGS__); return *instance; } \
	_##name##cofu::~_##name##cofu(void) { destructed = true; delete instance; } \
	bool _##name##cofu::isDestructed(void) { return destructed; }
