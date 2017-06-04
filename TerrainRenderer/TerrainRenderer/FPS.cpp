#include "FPS.h"

namespace TerrainRenderer
{
	FPS::FPS():
		mFPS(0), mCount(0), mStartTime(0)
	{

	}

	FPS::FPS(const FPS& rhs)
	{

	}

	FPS& FPS::operator=(const FPS& rhs)
	{
		return *this;
	}

	FPS::~FPS()
	{

	}

	void FPS::Initialize()
	{
		mFPS = 0;
		mCount = 0;
		mStartTime = timeGetTime();
	}

	void FPS::Frame()
	{
		mCount++;

		if (timeGetTime() >= (mStartTime + 1000))
		{
			mFPS = mCount;
			mCount = 0;

			mStartTime = timeGetTime();
		}
	}

	int FPS::GetFPS()
	{
		return mFPS;
	}
}