#pragma once

#include <set>
#include <vector>
#include <list>
#include <deque>
#include <bdlogprovider.h>

typedef std::set<CStringW> StrSet;
typedef std::list<std::wstring> strlist_t;

// 日志信息
struct LogInfo
{
	UINT64 logid;                      // 全局唯一的日志ID
	bdlog::logitem* item;              // 通过管道接收到的日志信息
	INT64 occupytime;                  // 日志占用时间（本条日志到同一线程的下一条日志的时间间隔）
};

// 列表项的定制
struct disp_info
{
	COLORREF color;
	COLORREF bkcolor;
	disp_info()
		: color(RGB(0, 0, 0)), bkcolor(RGB(255,255,255))
	{
	}
};

#define M 1000000

struct accutime
{
	INT64 sec;
	INT32 usec;

	accutime(INT64 s = 0, INT32 us = 0): sec(s), usec(us)
	{
	}
	accutime(const accutime& rhs): sec(rhs.sec), usec(rhs.usec)
	{
	}
	accutime& operator=(const accutime& rhs)
	{
		sec = rhs.sec;
		usec = rhs.usec;
		return *this;
	}

	accutime& operator+=(INT64 us)
	{
		us += usec;
		sec += us / M;
		usec = us % M;
		return *this;
	}
	accutime& operator-=(INT64 t)
	{
		INT64 s = t / M;
		INT32 us = static_cast<INT32>(t % M);
		sec -= s;
		if (us > usec)
		{
			sec--;
			usec = M + usec - us;
		}
		else
		{
			usec -= us;
		}
		return *this;
	}
	accutime operator+(INT64 us)
	{
		accutime ret(*this);
		ret += us;
		return ret;
	}
	accutime operator-(INT64 us)
	{
		accutime ret(*this);
		ret -= us;
		return ret;
	}

	INT64 operator-(const accutime& rhs) const
	{
		if (*this < rhs) return -(rhs-*this);
		return (sec - rhs.sec) * M + usec - rhs.usec;
	}

	bool operator<(const accutime& rhs) const
	{
		if (sec < rhs.sec) return true;
		if (sec > rhs.sec) return false;
		return usec < rhs.usec;
	}
	bool operator>(const accutime& rhs) const
	{
		if (sec > rhs.sec) return true;
		if (sec < rhs.sec) return false;
		return usec > rhs.usec;
	}
	bool operator==(const accutime& rhs) const
	{
		return sec == rhs.sec && usec == rhs.usec;
	}

};

struct time_range
{
	accutime t1;
	accutime t2;
};


#define BSP_BDXLOG_INI           L"<CSIDL:COMMON_APPDATA>\\Baidu\\<VAR:PRODUCT>\\bdlog.ini"
#define BSP_DEBUGSET_INI         L"<CSIDL:APPDATA>\\Baidu\\<VAR:PRODUCT>\\debugset.ini"

#define WM_FILTER_CHANGE     WM_USER + 21

