#pragma once

#include "filter.h"
#include "logcenter.h"
#include "service.h"
#include "serviceid.h"

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

struct task_info
{
	std::wstring name;
	time_range life;
};
typedef std::vector<task_info> task_infos_t;

struct thread_info
{
	UINT32 tid;
	UINT32 parent_tid;
	std::wstring name;
	time_range life;
	task_infos_t m_tasks;
	thread_info(UINT32 id = 0): tid(id), parent_tid(0)
	{
	}
};
typedef std::vector<thread_info*> thread_infos_t;
typedef std::map<UINT32, thread_info*> thread_map_t;

struct process_info
{
	UINT32 pid;
	std::wstring name;
	std::wstring path;
	thread_infos_t threads;
	thread_map_t cache;
	time_range life;
	process_info(): pid(0)
	{
	}
};
typedef std::vector<process_info> process_infos_t;
typedef std::map<UINT32, process_info*> process_map_t;

class CVisualLogic : public Service<SID_VisualLogic>, public CLogCenterListener
{
public:
	CVisualLogic();
	~CVisualLogic();

	void Init();
	void Uninit();

	const process_map_t& GetProcessInfo();

private:
	virtual void OnNewLog(const LogRange& range);

	void ParseThreadInfo(const LogInfo* li);

	process_info& Process(UINT32 pid);
	void AddThread(UINT32 pid, thread_info* ti);
	thread_info* GetThread(UINT32 pid, UINT32 tid);

	void OnError(LPCWSTR desc);

private:
	filter* m_threadfilter;
	process_map_t m_processes;

	LONGLONG m_pf;
};

DEFINE_SERVICE(CVisualLogic, L"逻辑可视化信息中心");
