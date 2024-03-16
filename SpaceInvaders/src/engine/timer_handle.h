#pragma once

struct TimerHandle
{
	friend class TimerManager;

	TimerHandle() : handle(0) {}

	bool is_valid()
	{
		return handle != 0;
	}

	void invalidate()
	{
		handle = 0;
	}

	bool operator==(const TimerHandle& other) const
	{
		return handle == other.handle;
	}

	bool operator!=(const TimerHandle& other) const
	{
		return handle != other.handle;
	}

private:
	static constexpr uint32_t index_bits = 24;
	static constexpr uint32_t generation_bits = 40;

	static_assert(index_bits + generation_bits == 64);

	static constexpr int32_t max_index = (int32_t)1 << index_bits;
	static constexpr uint64_t max_generation = (uint64_t)1 << generation_bits;

	int32_t get_index() const
	{
		return (int32_t)(handle & (uint64_t)max_generation - 1);
	}

	uint64_t get_generation() const
	{
		return handle >> index_bits;
	}

	void set_index_and_generation_number(int32_t index, uint64_t generation)
	{
		assert(index >= 0 && index < max_index);
		assert(generation < max_generation);

		handle = (generation << index_bits) | (uint64_t)index;
	}

	uint64_t handle;
};