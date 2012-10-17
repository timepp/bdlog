#pragma once

#include <windows.h>
#include <psapi.h>
#include <TlHelp32.h>
#include "detail/bdsecurity.h"                // for "lowest rights security attribute"

namespace bdlog
{

typedef __int8  int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

struct logitem
{
	uint64_t     log_index;
	int64_t      log_time_sec;
	int32_t      log_time_msec;
	uint32_t     log_pid;
	uint32_t     log_tid;
	uint32_t     log_class;
	uint32_t     log_depth;
	std::wstring log_process_name;
	std::wstring log_tags;
	std::wstring log_content;
};

enum log_reader_notify
{
	Error_Startup,
	Error_AcceptConnect,
	Error_HighClientVersion,
	Event_SourceEnter,
	Event_SourceLeave,
};

struct log_listener
{
	virtual void on_new_log(logitem* li) = 0;
	virtual void on_notify(int notifyid, HRESULT hr) = 0;

	virtual ~log_listener(){}
};

struct log_reader
{
	// 
	virtual void set_listener(log_listener* listener) = 0;

	virtual HRESULT start() = 0;
	virtual HRESULT stop() = 0;
	virtual bool working() const = 0;

	virtual ~log_reader(){}
};

struct log_source_info
{
	uint32_t pid;
	std::wstring process_name;
	std::wstring process_path;
	time_t join_time;
};

typedef std::vector<log_source_info> lsi_vec_t;

struct log_source_support
{
	virtual size_t source_count() const = 0;
	virtual lsi_vec_t get_sources() const = 0;

	virtual ~log_source_support(){}
};

struct win32_exception
{
public:
	HRESULT hr;
	win32_exception()
	{
		DWORD errcode = ::GetLastError();
		hr = HRESULT_FROM_WIN32(errcode);
	}
	static void throw_when(bool condition)
	{
		if (condition) throw win32_exception();
	}
};

struct multithread_guard
{
	CRITICAL_SECTION& m_cs;
	multithread_guard(CRITICAL_SECTION& cs) : m_cs(cs)
	{
		::EnterCriticalSection(&m_cs);
	}
	~multithread_guard()
	{
		::LeaveCriticalSection(&m_cs);
	}
};

class pipe_connection
{
private:
	HANDLE m_event;
	HANDLE m_pipe;
	OVERLAPPED m_overlap;
	bool m_iopending;
	const wchar_t* m_pipename;

public:
	pipe_connection(const wchar_t* pipename) : m_pipename(pipename)
	{
		m_event = CreateEventW(NULL, TRUE, FALSE, NULL);
		memset(&m_overlap, 0, sizeof(m_overlap));
		m_overlap.hEvent = m_event;
	}
	~pipe_connection()
	{
		::DisconnectNamedPipe(m_pipe);
		::CloseHandle(m_pipe); m_pipe = NULL;
		::CloseHandle(m_event); m_event = NULL;
	}

	HANDLE get_pipe()
	{
		return m_pipe;
	}
	HANDLE get_event()
	{
		return m_event;
	}

	void start_new_connection()
	{
		/// 因为pipeserver是管道的读端，为了保证最低权限的进程都可以向管道中写数据，创建pipe时必须用最低权限的SA
		/// (缺省情况下，低权限进程只可以READ_UP，不可以WRITE_UP)

		m_iopending = false;
		m_pipe = ::CreateNamedPipeW(m_pipename, 
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE|PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 
			10, 10 * 1000 * 1000, 0, CLowestRightsSecurityAttribute());
		win32_exception::throw_when(m_pipe == INVALID_HANDLE_VALUE);

		::ConnectNamedPipe(m_pipe, &m_overlap);
		switch(GetLastError())
		{
		case ERROR_IO_PENDING:     
			::ResetEvent(m_overlap.hEvent); 
			m_iopending = true; 
			break;
		case ERROR_PIPE_CONNECTED: 
			::SetEvent(m_overlap.hEvent); 
			break;
		default:
			win32_exception exp;
			throw exp;
		}
	}

	void on_connected()
	{
		ResetEvent(m_event);

		if (m_iopending)
		{
			DWORD transfered;
			BOOL ret = ::GetOverlappedResult(m_pipe, &m_overlap, &transfered, FALSE);
			win32_exception::throw_when(!ret);
		}
	}
};

class pipe_reader : public log_reader, public log_source_support
{
	struct logsrc
	{
		OVERLAPPED    overlap;
		HANDLE        pipe;
		pipe_reader*  thisptr;
		DWORD         pid;
		std::wstring  pname;
		byte*         buffer;
		size_t        buffer_size;
		size_t        content_len;
		size_t        version;

		logsrc() : pipe(NULL), thisptr(NULL), pid(0), buffer_size(4096), content_len(0), version(0)
		{
			memset(&overlap, 0, sizeof(overlap));
			buffer = new byte[buffer_size];
		}
		~logsrc()
		{
			delete [] buffer;
		}
	};
	typedef std::map<HANDLE, logsrc*> logsrc_map_t;
	

private:
	// 并发访问
	typedef std::map<HANDLE, log_source_info> lsi_map_t;
	lsi_map_t m_lsi_map;
	mutable CRITICAL_SECTION m_lsi_lock;
	HANDLE m_exit_evt;
	bool m_stop_flag;

	// 工作线程访问
	logsrc_map_t m_logsrc_map;
	log_listener* m_listener;

	// 主线程访问
	HANDLE m_worker_thread;

public:
	pipe_reader() : m_exit_evt(NULL), m_worker_thread(NULL), m_stop_flag(false)
	{
		::InitializeCriticalSection(&m_lsi_lock);
		m_exit_evt = ::CreateEventW(NULL, TRUE, FALSE, NULL);
	}

	~pipe_reader()
	{
		stop();
		::DeleteCriticalSection(&m_lsi_lock);
		::CloseHandle(m_exit_evt);
	}

	virtual void set_listener(log_listener* listener)
	{
		m_listener = listener;
	}

	virtual HRESULT start()
	{
		stop();
		m_stop_flag = false;

		m_worker_thread = (HANDLE)_beginthreadex(NULL, 0, work_thread, this, CREATE_SUSPENDED, NULL);
		if (m_worker_thread == NULL)
		{
			return win32_exception().hr;
		}

		::ResetEvent(m_exit_evt);
		::ResumeThread(m_worker_thread);

		return S_OK;
	}

	virtual HRESULT stop()
	{
		m_stop_flag = true;

		if (m_worker_thread)
		{
			::SetEvent(m_exit_evt);
			::WaitForSingleObject(m_worker_thread, INFINITE);
			CloseHandle(m_worker_thread);
			m_worker_thread = NULL;
		}

		// 此处不需要加锁，因为工作线程已经退出了
		for (logsrc_map_t::const_iterator it = m_logsrc_map.begin(); it != m_logsrc_map.end(); ++it)
		{
			logsrc* ls = it->second;
			if (ls)
			{
				::DisconnectNamedPipe(ls->pipe);
				::CloseHandle(ls->pipe);
				delete ls;
			}
		}

		m_logsrc_map.clear();
		m_lsi_map.clear();

		return S_OK;
	}

	virtual bool working() const
	{
		return m_worker_thread != NULL;
	}

	virtual size_t source_count() const
	{
		multithread_guard guard(m_lsi_lock);
		return m_lsi_map.size();
	}

	virtual lsi_vec_t get_sources() const
	{
		lsi_vec_t ret;
		{
			multithread_guard guard(m_lsi_lock);
			for (lsi_map_t::const_iterator it = m_lsi_map.begin(); it != m_lsi_map.end(); ++it)
			{
				ret.push_back(it->second);
			}
		}
		return ret;
	}

private:
	void remove_source(HANDLE pipe)
	{
		logsrc_map_t::iterator it = m_logsrc_map.find(pipe);
		if (it != m_logsrc_map.end())
		{
			::DisconnectNamedPipe(pipe);
			::CloseHandle(pipe);
			delete it->second;
			m_logsrc_map.erase(it);
		}

		multithread_guard guard(m_lsi_lock);
		lsi_map_t::iterator it2 = m_lsi_map.find(pipe);
		if (it2 != m_lsi_map.end())
		{
			m_lsi_map.erase(it2);
		}
	}

	void add_source(logsrc* src)
	{
		m_logsrc_map[src->pipe] = src;
	}

	void update_source_info(HANDLE pipe, const log_source_info& lsi)
	{
		multithread_guard guard(m_lsi_lock);
		m_lsi_map[pipe] = lsi;
	}

	static logitem* read_log_item(logsrc* ls, size_t* bytes_required)
	{
#define CHECK_SIZE(x) if (ls->content_len < x) { *bytes_required = x - ls->content_len; return NULL; }
		if (ls->pid == 0)
		{
			CHECK_SIZE(4);
			size_t ver = *(uint32_t*)ls->buffer;
			if (ver != 3) 
			{
				ls->thisptr->m_listener->on_notify(Error_HighClientVersion, E_FAIL);
				*bytes_required = 0;
				return NULL;
			}
			CHECK_SIZE(8);
			size_t hdrlen = *(uint32_t*)(ls->buffer + 4);
			CHECK_SIZE(8 + hdrlen);
			ls->pid = *(uint32_t*)(ls->buffer + 8);
			log_source_info lsi = get_source_info(ls->pid);
			ls->thisptr->update_source_info(ls->pipe, lsi);
			ls->pname = lsi.process_name;
			ls->content_len = 0;
			*bytes_required = 4;
			return NULL;
		}
		else
		{
			CHECK_SIZE(4);
			size_t len = *(uint32_t*)ls->buffer;
			CHECK_SIZE(4 + len);
			byte* buf = ls->buffer + 4;

			logitem* li = new logitem;
			FILETIME ft;
			li->log_pid = ls->pid;
			li->log_process_name = ls->pname;
			li->log_index = *(uint64_t*)(buf);
			ft.dwHighDateTime = *(UINT32*)(buf + 8);
			ft.dwLowDateTime = *(UINT32*)(buf + 12);
			li->log_tid = *(uint32_t*)(buf + 16);
			li->log_class = *(uint32_t*)(buf + 20);
			li->log_depth = *(uint32_t*)(buf + 24);
			size_t taglen = *(uint32_t*)(buf + 28);
			li->log_tags.assign((LPCWSTR)(buf + 32), taglen / 2);
			size_t contentLen = *(uint32_t*)(buf + 32 + taglen);
			li->log_content.assign((LPCWSTR)(buf + 36 + taglen), contentLen / 2);

			DWORD ns = ft.dwLowDateTime % 10000000;
			li->log_time_msec = static_cast<int>(ns / 10);
			INT64 t = ((INT64)ft.dwHighDateTime << 32) | (ft.dwLowDateTime - ns);
			t = (t - 116444736000000000LL) / 10000000;
			li->log_time_sec = t;

			ls->content_len = 0;
			*bytes_required = 4;

			return li;
		}
	}

	static unsigned int __stdcall work_thread(void* param)
	{
		pipe_reader* reader = reinterpret_cast<pipe_reader*>(param);
		try
		{
			reader->work_thread_internal();
		}
		catch(win32_exception& e)
		{
			reader->m_listener->on_notify(Error_Startup, e.hr);
		}
		
		return 0;
	}

	void work_thread_internal()
	{
		pipe_connection conn(L"\\\\.\\pipe\\bdlog_data_channel");
		conn.start_new_connection();

		for (;;)
		{
			HANDLE evts[] = {m_exit_evt, conn.get_event()};
			DWORD dwWait = ::WaitForMultipleObjectsEx(_countof(evts), evts, FALSE, INFINITE, TRUE);
			if (dwWait == WAIT_IO_COMPLETION)
			{
			}
			else if (dwWait == WAIT_OBJECT_0) // exit request
			{
				return;
			}
			else if (dwWait == WAIT_OBJECT_0 + 1) // pipe connect
			{
				conn.on_connected();

				logsrc* ls = new logsrc;
				ls->pipe = conn.get_pipe();
				ls->thisptr = this;
				ls->version = 2;
				add_source(ls);

				complete_read_routine(0, 0, (LPOVERLAPPED)ls);

				conn.start_new_connection();
			}
			else
			{
				throw win32_exception();
			}
		}
	}

	static void WINAPI complete_read_routine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED overlap)
	{
		logsrc* src = (logsrc*)overlap;
		if (src->thisptr->m_stop_flag)
		{
			return;
		}
		if (dwErr != 0 && dwErr != ERROR_MORE_DATA)
		{
			src->thisptr->remove_source(src->pipe);
			return;
		}

		src->content_len += cbBytesRead;
		size_t bytes_required = 0;
		logitem* li = read_log_item(src, &bytes_required);
		if (li)
		{
			src->thisptr->m_listener->on_new_log(li);
		}
		if (bytes_required == 0)
		{
			return;
		}

		if (src->buffer_size <= src->content_len + bytes_required)
		{
			size_t new_size = src->buffer_size * 2;
			byte* new_buffer = new byte[new_size];
			memcpy(new_buffer, src->buffer, src->content_len);
			delete [] src->buffer;
			src->buffer = new_buffer;
			src->buffer_size = new_size;
		}
		
		BOOL ret = ::ReadFileEx(src->pipe, src->buffer + src->content_len, bytes_required, &src->overlap, &pipe_reader::complete_read_routine);
		if (!ret)
		{
			src->thisptr->remove_source(src->pipe);
			return;
		}
	}

	static log_source_info get_source_info(DWORD pid)
	{
		log_source_info lsi;
		wchar_t path[MAX_PATH] = {0};

		lsi.join_time = time(NULL);
		lsi.pid = pid;

		HANDLE process = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if (process)
		{
			::GetModuleFileNameExW(process, NULL, path, _countof(path));
			::CloseHandle(process);
		}

		if (wcslen(path) == 0)
		{
			process = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
			if (process)
			{
				DWORD len = ::GetProcessImageFileNameW(process, path, _countof(path));
				path[len] = L'\0';
				::CloseHandle(process);
			}
		}

		if (wcslen(path) == 0)
		{
			HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (snap != INVALID_HANDLE_VALUE)
			{
				PROCESSENTRY32 pe32;
				pe32.dwSize = sizeof(pe32);
				for (BOOL ret = Process32FirstW(snap, &pe32); ret; ret = Process32NextW(snap, &pe32))
				{
					if (pe32.th32ProcessID == pid)
					{
						wcsncpy_s(path, pe32.szExeFile, _TRUNCATE);
						break;
					}
				}
				CloseHandle(snap);
			}
		}

		lsi.process_path = path;
		const wchar_t* p = wcsrchr(path, L'\\');
		lsi.process_name = p? p+1 : path;

		return lsi;
	}
};


class file_reader : public log_reader, public log_source_support
{
private:
	HANDLE m_worker_thread;
	bool m_stop_flag;
	std::wstring m_path;
	bdlog::log_listener* m_listener;

	typedef std::map<DWORD, log_source_info> lsi_map_t;
	lsi_map_t m_lsi_map;

public:
	file_reader() : m_stop_flag(true), m_worker_thread(NULL)
	{

	}

	~file_reader()
	{
		stop();
	}

	void setpath(const wchar_t* filename)
	{
		m_path = filename;
	}

	virtual void set_listener(log_listener* listener)
	{
		m_listener = listener;
	}

	virtual HRESULT start()
	{
		if (!m_stop_flag)
		{
			return S_FALSE;
		}

		m_lsi_map.clear();
		m_stop_flag = false;
		m_worker_thread = (HANDLE)_beginthreadex(NULL, 0, work_thread, this, 0, NULL);
		return S_OK;
	}

	virtual HRESULT stop()
	{
		m_stop_flag = true;
		if (m_worker_thread)
		{
			::WaitForSingleObject(m_worker_thread, INFINITE);
			::CloseHandle(m_worker_thread);
			m_worker_thread = NULL;
		}
		return S_OK;
	}

	virtual bool working() const
	{
		return !m_stop_flag;
	}

	virtual size_t source_count() const
	{
		if (!m_stop_flag) return 0;
		return m_lsi_map.size();
	}

	virtual lsi_vec_t get_sources() const
	{
		lsi_vec_t ret;
		if (!m_stop_flag) return ret;

		for (lsi_map_t::const_iterator it = m_lsi_map.begin(); it != m_lsi_map.end(); ++it)
		{
			ret.push_back(it->second);
		}
		return ret;
	}

	static unsigned int __stdcall work_thread(void* param)
	{
		file_reader* thisptr = reinterpret_cast<file_reader*>(param);
		thisptr->work_thread_internal();
		thisptr->m_stop_flag = true;
		return 0;
	}

	void work_thread_internal()
	{
		FILE* fp = _wfsopen(m_path.c_str(), L"rt, ccs=UNICODE", _SH_DENYNO);
		if (fp == NULL)
		{
			return;
		}

		logitem* li = new logitem;
		UINT64 id = 0;
		li->log_index = id;

		/** 日志行格式:
		yyyy-mm-dd HH:MM:SS.ms c [pname:pid@hex:tid@hex] {tags} content...
		*/

		size_t last_prefix_len = 0;
		struct tm tt = {};
		WCHAR szLine[4096];
		while (!m_stop_flag && fgetws(szLine, _countof(szLine), fp))
		{
			//::Sleep(100);
			size_t line_len = wcslen(szLine);
			for (LPWSTR p = szLine + line_len - 1; p >= szLine && (*p == L'\r' || *p == L'\n'); p--)
			{
				*p = L'\0';
				line_len--;
			}

			bool is_title_line = false;
			do 
			{
				if (szLine[0] == L' ') break; // 以空格开头的一定不是标题行

				int millisec;
				unsigned int c;
				wchar_t tags[256];
				wchar_t ptbuf[1024];
				if (swscanf_s(szLine, L"%d-%d-%d %d:%d:%d.%d %d [%[^]]] {%[^}]}", 
					&tt.tm_year, &tt.tm_mon, &tt.tm_mday, &tt.tm_hour, &tt.tm_min, &tt.tm_sec, &millisec, 
					&c, 
					ptbuf, _countof(ptbuf), 
					tags, _countof(tags)) != 10)
				{
					// 解析日期错误,非标题行
					break;
				}
				tt.tm_year -= 1900;
				tt.tm_mon -= 1;
				LPCWSTR te = wcschr(szLine, L'}');
				if (!te) break;
				LPCWSTR content_start = te + 2;

				// 是标题行了
				li->log_index = ++id;
				is_title_line = true;
				last_prefix_len = static_cast<size_t>(content_start - szLine);
				if (id > 1) // 输出上一条日志
				{
					m_listener->on_new_log(li);
					li = new logitem();
				}

				li->log_pid = 0;
				li->log_tid = 0;
				li->log_depth = 0;
				wchar_t* p = wcschr(ptbuf, L':');
				if (p)
				{
					*p++ = L'\0';
					li->log_process_name = ptbuf;
					wchar_t* q = wcschr(p, L':');
					if (q)
					{
						*q++ = L'\0';
						swscanf_s(p, L"%x", &li->log_pid);
						wchar_t* r = wcschr(q, L':');
						if (r)
						{
							*r++ = L'\0';
							swscanf_s(q, L"%x", &li->log_tid);
							swscanf_s(r, L"%d", &li->log_depth);
						}
						else
						{
							swscanf_s(q, L"%x", &li->log_tid);
						}
					}
					else
					{
						swscanf_s(p, L"%x", &li->log_tid);
					}
				}
				else
				{
					swscanf_s(ptbuf, L"%x", &li->log_tid);
				}

				for (p = tags + wcslen(tags) - 1; p >= tags && *p == L' '; p--)
				{
					*p = L'\0';
				}

				li->log_time_sec = mktime(&tt);
				li->log_time_msec = millisec;
				li->log_class = c;
				li->log_tags = tags;
				li->log_content = content_start;

				lsi_map_t::const_iterator it = m_lsi_map.find(li->log_pid);
				if (it == m_lsi_map.end())
				{
					log_source_info lsi;
					lsi.join_time = li->log_time_sec;
					lsi.pid = li->log_pid;
					lsi.process_name = li->log_process_name;
					lsi.process_path = li->log_process_name;
					m_lsi_map[lsi.pid] = lsi;
				}

			} while (false);

			if (!is_title_line)
			{
				li->log_content += L"\n";
				size_t i;
				for (i = 0; i < last_prefix_len && szLine[i]; i++)
				{
					if (szLine[i] != L' ') break;
				}
				if (i == last_prefix_len)
				{
					li->log_content += szLine + last_prefix_len;
				}
				else
				{
					li->log_content += szLine;
				}
			}
		}

		if (li->log_index > 0)
		{
			m_listener->on_new_log(li);
		}

		fclose(fp);
	}
};



} // namespace bdlog