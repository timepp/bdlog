#pragma once

#include <Windows.h>

class cslock
{
public:
	cslock()
	{
		::InitializeCriticalSection(&m_cs);
	}
	~cslock()
	{
		::DeleteCriticalSection(&m_cs);
	}
	void lock()
	{
		::EnterCriticalSection(&m_cs);
	}
	void unlock()
	{
		::LeaveCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION m_cs;
};

class autolocker
{
public:
	autolocker(cslock& cs) : m_cs(&cs) { cs.lock(); }
	~autolocker() { m_cs->unlock(); }
private:
	cslock* m_cs;
};

#define MULTI_TRHEAD_GUARD(cs) autolocker al_(cs)
