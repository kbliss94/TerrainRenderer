#ifndef _CPU_
#define _CPU_

#pragma once

//linking
#pragma comment(lib, "pdh.lib")

#include <pdh.h>

namespace TerrainRenderer
{
	class CPU
	{
	public:
		//!Constructor
		CPU();

		//!Copy constructor
		CPU(const CPU& rhs);

		//!Assignment operator
		CPU& operator=(const CPU& rhs);

		//!Destructor
		~CPU();

		//!Combines the usage of all the CPUs in the system; will set mCanReadCPU to false
		//!if unable to poll CPU usage. Only samples CPU usage once a second
		void Initialize();

		void Shutdown();

		//!Called each frame, but samples only once a second
		void Frame();

		int GetCPUPercentage();

	private:
		bool mCanReadCPU;
		HQUERY mQueryHandle;
		HCOUNTER mCounterHandle;
		unsigned long mLastSampleTime;
		long mCPUUsage;
	};
}

#endif
