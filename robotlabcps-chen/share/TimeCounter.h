#pragma once
#include <chrono>
#include "Logger.h"

// ͨ�ü�ʱ�������ڼ�����������֮���ʱ����
class TimeCounter
{
public:
	TimeCounter()
	{
		m_tick_start = std::chrono::high_resolution_clock::now();
	}
	// return milliseconds
	long long Tick(const char* operation=NULL)
	{
		auto t = std::chrono::high_resolution_clock::now();
		auto d = std::chrono::duration_cast<std::chrono::milliseconds>(t - m_tick_start).count();
		if (operation)
		{
			LOG_INFO("[%s] takes %lld ms!", operation, d);
		}
		return d;
	}
protected:
	std::chrono::high_resolution_clock::time_point m_tick_start;
};
