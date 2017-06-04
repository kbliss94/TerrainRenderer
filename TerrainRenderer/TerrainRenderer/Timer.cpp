#include "Timer.h"

namespace TerrainRenderer
{
	Timer::Timer():
		mFrequency(0), mTicksPerMS(0), mStartTime(0), mFrameTime(0)
	{

	}

	Timer::Timer(const Timer& rhs)
	{

	}

	Timer& Timer::operator=(const Timer& rhs)
	{
		return *this;
	}

	Timer::~Timer()
	{

	}

	bool Timer::Initialize()
	{
		//checking to see if this system supports high performance timers
		QueryPerformanceFrequency((LARGE_INTEGER*)&mFrequency);
		if (mFrequency == 0)
		{
			return false;
		}

		//finding out how many times the frequency counter ticks every millisecond
		mTicksPerMS = static_cast<float>(mFrequency / 1000);
		QueryPerformanceCounter((LARGE_INTEGER*)&mStartTime);

		return true;
	}

	void Timer::Frame()
	{
		INT64 currentTime;
		float timeDifference;

		QueryPerformanceCounter((LARGE_INTEGER*)& currentTime);

		timeDifference = static_cast<float>(currentTime - mStartTime);

		mFrameTime = timeDifference / mTicksPerMS;
		mStartTime = currentTime;
	}

	float Timer::GetTime()
	{
		return mFrameTime;
	}
}