#pragma once

class CProfiler
{
public:
	explicit CProfiler(bool autostart = false) : m_counter()
	{
		if (autostart)
		{
			Start();
		}
	}

	void Start()
	{
		::QueryPerformanceCounter(&m_counter);
	}

	int GetElapsedMicroSeconds()
	{
		LARGE_INTEGER li;
		::QueryPerformanceCounter(&li);
		return static_cast<int>((li.QuadPart - m_counter.QuadPart) * 1000000 / m_freq.QuadPart);
	}

private:
	LARGE_INTEGER m_counter;
	static LARGE_INTEGER m_freq;
	static BOOL m_avaliable;
};
