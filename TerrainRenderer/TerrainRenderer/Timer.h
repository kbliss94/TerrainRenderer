#ifndef _TIMER_
#define _TIMER_

#pragma once

#include <windows.h>

namespace TerrainRenderer
{
	class Timer
	{
	public:
		Timer();
		Timer(const Timer& rhs);
		Timer& operator=(const Timer& rhs);
		~Timer();

		bool Initialize();

		//!Called for every loop of execution by Main to determine the time it took to 
		//!execute each frame
		void Frame();
		float GetTime();

	private:
		INT64 mFrequency;
		INT64 mStartTime;
		float mTicksPerMS;
		float mFrameTime;
	};
}

#endif
