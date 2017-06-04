#include "CPU.h"

namespace TerrainRenderer
{
	CPU::CPU() :
		mCanReadCPU(false), mLastSampleTime(0), mCPUUsage(0)
	{

	}

	CPU::CPU(const CPU& rhs)
	{

	}

	CPU& CPU::operator=(const CPU& rhs)
	{
		return *this;
	}

	CPU::~CPU()
	{

	}

	void CPU::Initialize()
	{
		PDH_STATUS status;

		mCanReadCPU = true;

		//creating a query object to poll CPU usage
		status = PdhOpenQuery(nullptr, 0, &mQueryHandle);
		if (status != ERROR_SUCCESS)
		{
			mCanReadCPU = false;
		}

		//setting query object to poll all CPUs in the system
		status = PdhAddCounter(mQueryHandle, TEXT("\\Processor(_Total)\\% processor time"), 0, &mCounterHandle);
		if (status != ERROR_SUCCESS)
		{
			mCanReadCPU = false;
		}

		mLastSampleTime = GetTickCount();
		mCPUUsage = 0;
	}

	void CPU::Shutdown()
	{
		if (mCanReadCPU)
		{
			PdhCloseQuery(mQueryHandle);
		}
	}

	void CPU::Frame()
	{
		PDH_FMT_COUNTERVALUE value;

		if (mCanReadCPU)
		{
			if ((mLastSampleTime + 1000) < GetTickCount())
			{
				mLastSampleTime = GetTickCount();
				PdhCollectQueryData(mQueryHandle);
				PdhGetFormattedCounterValue(mCounterHandle, PDH_FMT_LONG, nullptr, &value);
				mCPUUsage = value.longValue;
			}
		}
	}

	int CPU::GetCPUPercentage()
	{
		int usage;

		if (mCanReadCPU)
		{
			usage = static_cast<int>(mCPUUsage);
		}
		else
		{
			usage = 0;
		}

		return usage;
	}
}