#include "processing.h"

mirage::ecs::processing::Processor::Processor(void)
{
	mutex = std::make_shared<std::mutex>();
	processes = std::make_shared<std::map<entt::id_type, std::unique_ptr<Process>>>();
}

void mirage::ecs::processing::Process::fail(void)
{
	state = State::failed;
	throw Exit{};
}

void mirage::ecs::processing::Process::pause(void)
{
	state = State::paused;
	throw Exit{};
}
void mirage::ecs::processing::Process::unpause(void) 
{ 
	state = State::running; 
}

void mirage::ecs::processing::Process::succeed(void) 
{
	state = State::succeeded;
	throw Exit{};
}
void mirage::ecs::processing::Process::terminate(void) noexcept 
{
	state = State::terminated;
}
void mirage::ecs::processing::Process::initialize(void) {}
void mirage::ecs::processing::Process::onFail(void) {}
void mirage::ecs::processing::Process::onSucceed(void) {}

void mirage::ecs::processing::Processor::terminate(entt::id_type id) 
{
	std::lock_guard guard{*mutex};

	if (!processes->contains(id))
		return;

	processes->at(id)->terminate();
}

void mirage::ecs::processing::Processor::update(void) 
{
	if (processes->empty())
		return;	

	std::lock_guard guard{*mutex};

	std::erase_if(*processes,
				[&](auto &pair) -> bool 
				{
					auto &process = pair.second;
					switch (process->state) 
					{
					case Process::State::running:
						try 
						{
							clock_t now = clock();
							float passed;

							if (now < process->last) // clock_t overflow
							{
								if constexpr (sizeof(clock_t) == 4)
									passed = static_cast<float>(
											static_cast<uint64_t>(UINT32_MAX) + now) /
										(CLOCKS_PER_SEC * 1000.f);
								else
									passed = now / (CLOCKS_PER_SEC * 1000.f);
							} 
							else [[likely]]
								passed = static_cast<float>(now - process->last) /
									(CLOCKS_PER_SEC * 1000.f);

							process->update(passed);
							process->last = now;
						} 
						catch (Process::Exit) {} 
						catch (std::exception &exception) 
						{
							process->fail();
							loge("Exception in process \"{}\": {}, process "
								"terminated", // TODO: boost::stacktrace
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

mirage::ecs::processing::Processor::~Processor(void) {}

void mirage::ecs::processing::EventDispatcherProcessing::initialize(void)
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

void mirage::ecs::processing::EventDispatcherProcessing::onDestroy()
{
	thread.request_stop();
	stopped.wait(false);
}

