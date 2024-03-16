#include "timer_manager.h"

uint64_t TimerManager::last_generation = 0;

TimerManager::TimerManager()
{
	timers.reserve(100);
}

void TimerManager::update(float delta_time)
{
	std::vector<TimerHandle*> timers_to_remove;
	for (auto& t : timers)
	{
		t.expire_time -= delta_time;
		if (t.expire_time < 0.0)
		{
			t.func();

			if (t.loop)
			{
				t.expire_time = t.rate;
			}
			else
			{
				timers_to_remove.push_back(&t.handle);
			}
		}
	}

	for (auto& handler : timers_to_remove)
	{
		clear_timer(*handler);
	}
}

void TimerManager::set_timer(TimerHandle& handle, float rate, std::function<void()> function, bool is_loop, float delay)
{
	// Reset the handle
	if (handle.is_valid())
	{
		handle.invalidate();
	}

	if (rate > 0.0f)
	{
		TimerData timer_data;
		timer_data.rate = rate;
		timer_data.loop = is_loop;
		timer_data.func = std::move(function);
		timer_data.expire_time = rate + delay;

		handle = add_timer(std::move(timer_data));
	}
	else
	{
		handle.invalidate();
	}
}

void TimerManager::clear_timer(TimerHandle& handle)
{
	timers.erase(std::next(timers.begin(), handle.get_index()));
}

TimerHandle TimerManager::add_timer(TimerData&& data)
{
	timers.push_back(std::move(data));
	const auto index = timers.size() - 1;

	TimerHandle handle = generate_handle(static_cast<uint32_t>(index));
	timers.at(index).handle = handle;

	return handle;
}

TimerHandle TimerManager::generate_handle(uint32_t index)
{
	uint64_t generation = ++last_generation;

	if (generation == TimerHandle::max_generation)
	{
		printf("[WARNING]: Timer generation has looped around!");
		generation = 1;
	}

	TimerHandle handle;
	handle.set_index_and_generation_number(index, generation);
	return handle;
}
