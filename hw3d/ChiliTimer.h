#pragma once
#include <chrono>

class ChiliTimer
{
public:
	ChiliTimer() noexcept;
	float Mark() noexcept;					//推进时间轴（重置起点）
	float Peek() const noexcept;			//观察时间
private:
	std::chrono::steady_clock::time_point last;
};