#pragma once

#include <windows.h>
#include "detail/bdsharemem.h"

// TODO 休眠后，用基准时间+计数器算出来的时间落后于当前时间

#pragma pack(push, 1)
struct TimingInfo
{
	FILETIME baseTime;                  // 基准时间(unix time)

	INT64 basePerformanceCounter;       // 基准高精度计时器计数
	INT64 performanceFrequency;         // 高精度计时器频率
};
#pragma pack(pop)

class CLogAccurateTime
{
public:
	CLogAccurateTime()
		: m_timingInfo()
	{
	}

	void Init()
	{
		HRESULT hr = m_sm.Open(L"bdlog_timing_info_V3", sizeof(TimingInfo), NULL);
		if (FAILED(hr))
		{
			m_timingInfo = CreateTimingInfo();
			return;
		}

		m_sm.Lock();
		TimingInfo& info = m_sm.GetDataAs<TimingInfo>();
		{
			if (hr == S_OK)
			{
				info = CreateTimingInfo();
			}

			m_timingInfo = info;
		}
		m_sm.UnLock();
	}

	FILETIME GetTime() const
	{
		FILETIME t; // FILETIME的单位为 100纳秒， 即 1/E7 秒， 所以下面计算时， 要乘以E7
		if (m_timingInfo.performanceFrequency > 0)
		{
			LARGE_INTEGER li;
			::QueryPerformanceCounter(&li);
			const int E7 = 10000000;
			INT64 counterDelta = li.QuadPart - m_timingInfo.basePerformanceCounter;
			INT64 counterUnit = counterDelta * E7 / m_timingInfo.performanceFrequency; 
			counterUnit += m_timingInfo.baseTime.dwLowDateTime;
			t.dwHighDateTime = m_timingInfo.baseTime.dwHighDateTime + static_cast<DWORD>(counterUnit >> 32);
			t.dwLowDateTime = static_cast<DWORD>(counterUnit & UINT_MAX);
		}
		else
		{
			::GetSystemTimeAsFileTime(&t);
		}

		return t;
	}

private:
	TimingInfo m_timingInfo;
	CSharingMemory m_sm;

	static TimingInfo CreateTimingInfo()
	{
		TimingInfo info;
		::GetSystemTimeAsFileTime(&info.baseTime);

		LARGE_INTEGER freq;
		LARGE_INTEGER counter;
		if (::QueryPerformanceFrequency(&freq) && ::QueryPerformanceCounter(&counter))
		{
			info.basePerformanceCounter = counter.QuadPart;
			info.performanceFrequency = freq.QuadPart;
		}
		else
		{
			info.basePerformanceCounter = 0;
			info.performanceFrequency = 0;
		}

		return info;
	}
};
