#pragma once

#include <defines.h>
#include <engine/timer_handle.h>

class Game;
struct TimerHandle;

struct TimerData
{
	// Movable only.
	TimerData() = default;
	TimerData(TimerData&&) = default;
	TimerData(const TimerData&) = delete;
	TimerData& operator=(TimerData&&) = default;
	TimerData& operator=(const TimerData&) = delete;

	bool loop{ false }; // If this timer should repeat or not.
	float rate; // Time between set and fire or repeat frequency.
	float expire_time; // Time remaining until the callback is fired.
	std::function<void()> func;
	TimerHandle handle;
};

class TimerManager
{
public:
	explicit TimerManager();
	void update(float delta_time);
	void set_timer(TimerHandle& handle, float rate, std::function<void()> function, bool is_loop = false, float delay = 0.0f);
	void clear_timer(TimerHandle& handle);

private:
	TimerHandle add_timer(TimerData&& data);
	TimerHandle generate_handle(uint32_t index);

	static uint64_t last_generation;
	std::vector<TimerData> timers;
};