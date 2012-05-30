#include "stdafx.h"
#include "logicvis.h"
#include "concretefilter.h"
#include "servicehelper.h"

struct inplace_str
{
	const wchar_t* str;
	size_t len;
	inplace_str() : str(L""), len(0)
	{
	}
	template <typename T>
	T to_int() const
	{
		T v = 0;
		for (size_t i = 0; i < len; i++)
		{
			v = v * 10 + (str[i] - L'0');
		}
		return v;
	}
	
	std::wstring to_str() const
	{
		return std::wstring(str, len);
	}
};
struct named_value
{
	inplace_str key;
	inplace_str val;
};

struct vllog_info
{
	inplace_str hdr;

	size_t value_count;
	named_value values[32];

	inplace_str find_value(const wchar_t* key)
	{
		size_t len = wcslen(key);
		for (size_t i = 0; i < value_count; i++)
		{
			named_value& nv = values[i];
			if (nv.key.len == len && wcsncmp(key, nv.key.str, nv.key.len) == 0)
			{
				return nv.val;
			}
		}

		return inplace_str();
	}
};

template <typename T>
static size_t ptrdiff(const T* p1, const T* p2)
{
	return static_cast<size_t>(p1 > p2 ? (p1-p2) : (p2-p1));
}

static bool ParseVLLog(const wchar_t* log, vllog_info& vli)
{
	const wchar_t* p = wcschr(log, L'|');
	if (!p) return false;

	const wchar_t* q = p;
	while (q > log && (*(q-1) == L' ' || *(q-1) == L'|')) q--;

	vli.hdr.str = log;
	vli.hdr.len = ptrdiff(q, log);

	const wchar_t* logend = log + wcslen(log);

	size_t index = 0;
	const wchar_t* r = p + 1;
	while (*r)
	{
		// ignore spaces
		if (*r == L' ') { r++; continue; }

		const wchar_t* s = wcschr(r, L':');
		const wchar_t* t = wcschr(r, L' ');
		if (!t)
		{
			t = logend;
		}

		if (s && s < t)
		{
			named_value& nv = vli.values[index++];
			nv.key.str = r;
			nv.key.len = ptrdiff(s, r);
			nv.val.str = s + 1;
			nv.val.len = ptrdiff(t, s) - 1;
		}

		r = t;
	}

	vli.value_count = index;
	return true;
}

CVisualLogic::CVisualLogic()
{
	logtag_filter* f = new logtag_filter(L"thread");
	m_threadfilter = f;

	LARGE_INTEGER li;
	::QueryPerformanceFrequency(&li);
	m_pf = li.QuadPart;
}

CVisualLogic::~CVisualLogic()
{
	
}

void CVisualLogic::Init()
{
	ServiceHelper::GetLogCenter()->AddListener(this);
}

void CVisualLogic::Uninit()
{
	ServiceHelper::GetLogCenter()->RemoveListener(this);
}

void CVisualLogic::OnNewLog(const LogRange& range)
{
	for (UINT64 i = range.idmin; i < range.idmax; i++)
	{
		const LogInfo* li = ServiceHelper::GetLogCenter()->GetLog(i);
		if (m_threadfilter->meet(*li))
		{
			ParseThreadInfo(li);
		}

		// update process life
		accutime t(li->item->log_time_sec, li->item->log_time_msec);
		process_info& pi = Process(li->item->log_pid);
		if (pi.life.t1.sec == 0 || t < pi.life.t1)
		{
			pi.life.t1 = t;
		}
		if (pi.life.t2.sec == 0 || pi.life.t2 < t)
		{
			pi.life.t2 = t;
		}
	}
}

process_info& CVisualLogic::Process(UINT32 pid)
{
	process_map_t::const_iterator it = m_processes.find(pid);
	if (it == m_processes.end())
	{
		process_info* pinfo = new process_info;
		pinfo->pid = pid;
		m_processes[pid] = pinfo;
		return *pinfo;
	}
	else
	{
		return *(it->second);
	}
}

void CVisualLogic::AddThread(UINT32 pid, thread_info* ti)
{
	process_info& pinfo = Process(pid);
	if (pinfo.threads.size() == 0)
	{
		pinfo.threads.push_back(ti);
	}
	else
	{
		size_t len = pinfo.threads.size();
		while (len > 0)
		{
			if (pinfo.threads[len-1]->life.t1 < ti->life.t1) break;
			len--;
		}
		pinfo.threads.insert(pinfo.threads.begin() + static_cast<int>(len), ti);
	}
	pinfo.cache[ti->tid] = ti;
}

thread_info* CVisualLogic::GetThread(UINT32 pid, UINT32 tid)
{
	process_info& pinfo = Process(pid);

	thread_map_t::iterator it = pinfo.cache.find(tid);
	if (it != pinfo.cache.end())
	{
		return it->second;
	}

	return NULL;
}

void CVisualLogic::ParseThreadInfo(const LogInfo* li)
{
	vllog_info vli;
	if (!ParseVLLog(li->item->log_content.c_str(), vli)) return;

	bdlog::logitem& l = *(li->item);

	// thread create
	if (wcsncmp(L"thread start", vli.hdr.str, vli.hdr.len) == 0)
	{
		inplace_str stid = vli.find_value(L"TID");
		UINT32 tid = stid.to_int<UINT32>();
		thread_info* ti = GetThread(l.log_pid, tid);
		if (!ti)
		{
			ti = new thread_info(tid);
			ti->life.t1.sec = l.log_time_sec;
			ti->life.t1.usec = l.log_time_msec;
			AddThread(l.log_pid, ti);
		}

		inplace_str addr = vli.find_value(L"ADDR");
		if (ti->name.empty())
		{
			ti->name = addr.to_str();
		}
	}
	else if (wcsncmp(L"thread end", vli.hdr.str, vli.hdr.len) == 0)
	{
		thread_info* ti = GetThread(l.log_pid, l.log_tid);
		if (!ti)
		{
			OnError(L"cannot get thread info");
		}
		else
		{
			ti->life.t2.sec = l.log_time_sec;
			ti->life.t2.usec = l.log_time_msec;
		}
	}
	else if (wcsncmp(L"thread create", vli.hdr.str, vli.hdr.len) == 0)
	{
		inplace_str stid = vli.find_value(L"S_TID");
		inplace_str dtid = vli.find_value(L"D_TID");
		UINT32 tid = dtid.to_int<UINT32>();

		thread_info* ti = GetThread(l.log_pid, tid);
		if (!ti)
		{
			ti = new thread_info(tid);
			ti->life.t1.sec = l.log_time_sec;
			ti->life.t1.usec = l.log_time_msec;
			AddThread(l.log_pid, ti);
		}

		ti->parent_tid = stid.to_int<UINT32>();
	}
	else if (wcsncmp(L"thread info", vli.hdr.str, vli.hdr.len) == 0)
	{
		inplace_str stid = vli.find_value(L"TID");
		inplace_str name = vli.find_value(L"NAME");
		UINT32 tid = stid.to_int<UINT32>();
		thread_info* ti = GetThread(l.log_pid, tid);
		if (!ti)
		{
			ti = new thread_info(tid);
			ti->life.t1.sec = l.log_time_sec;
			ti->life.t1.usec = l.log_time_msec;
			AddThread(l.log_pid, ti);
		}

		ti->name = name.to_str();
	}
	else if (wcsncmp(L"task run", vli.hdr.str, vli.hdr.len) == 0)
	{
		thread_info* ti = GetThread(l.log_pid, l.log_tid);
		if (!ti)
		{
			OnError(L"cannot get thread info");
		}
		else
		{
			inplace_str interval = vli.find_value(L"TIME");
			task_info task;
			task.life.t2.sec = l.log_time_sec;
			task.life.t2.usec = l.log_time_msec;
			LONGLONG pc = interval.to_int<LONGLONG>();
			INT64 us = pc * M / m_pf;
			task.life.t1 = task.life.t2 - us;
			ti->m_tasks.push_back(task);
		}
	}
}

const process_map_t& CVisualLogic::GetProcessInfo()
{
	return m_processes;
}

void CVisualLogic::OnError(LPCWSTR desc)
{
	OutputDebugStringW(desc);
}
