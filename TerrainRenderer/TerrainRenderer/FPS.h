#ifndef _FPS_
#define _FPS_

#pragma once

//linking
#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <mmsystem.h>

namespace TerrainRenderer
{
	class FPS
	{
	public:
		//!Constructor
		FPS();

		//!Copy constructor
		FPS(const FPS& rhs);

		//!Assignment operator
		FPS& operator=(const FPS& rhs);

		//!Destructor
		~FPS();

		void Initialize();
		void Frame();
		int GetFPS();

	private:
		int mFPS, mCount;
		unsigned long mStartTime;
	};
}

#endif
