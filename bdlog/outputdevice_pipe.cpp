#include "stdafx.h"
#include "outputdevice_pipe.h"
#include <time.h>

static void WritePipe(HANDLE pipe, LPCVOID buffer, DWORD len)
{
	DWORD bytesWrite;
	if (!WriteFile(pipe, buffer, len, &bytesWrite, NULL))
	{
		throw 1;
	}
}

CLOD_Pipe::CLOD_Pipe()
: m_pipe(INVALID_HANDLE_VALUE)
, m_lastTryConnectTime(0)
, m_notifyOld(false)
, m_notified(false)
{

}

CLOD_Pipe::~CLOD_Pipe()
{
	Close();
}

HRESULT CLOD_Pipe::Open(ILogOption* opt)
{
	Close();
	m_pipeName.SetStr(opt->GetOption(L"name", L"bdlog_receiver"));
	m_notifyOld = opt->GetOptionAsBool(L"notify_old", false);
	m_notified = false;

	return S_OK;
}

HRESULT CLOD_Pipe::Close()
{
	if (m_pipe != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_pipe);
		m_pipe = INVALID_HANDLE_VALUE;
	}
	return S_OK;
}

HRESULT CLOD_Pipe::Flush()
{
	return S_OK;
}

HRESULT CLOD_Pipe::OnConfigChange(ILogOption* opt)
{
	const wchar_t* newname = opt->GetOption(L"name", m_pipeName);
	if (wcscmp(m_pipeName, newname) != 0)
	{
		m_pipeName.SetStr(newname);
		Close();
	}

	return S_OK;
}

HRESULT CLOD_Pipe::Write(const LogItem* item)
{
	if (m_notifyOld && m_notified)
	{
		return S_FALSE;
	}

	bool newpipe = false;
	if (m_pipe == INVALID_HANDLE_VALUE)
	{
		time_t currTime = time(NULL);
		if (currTime >= m_lastTryConnectTime + 10)
		{
			m_pipe = OpenPipe(m_pipeName);
			m_lastTryConnectTime = time(NULL);
			newpipe = true;
		}
	}
	if (m_pipe != INVALID_HANDLE_VALUE)
	{
		try
		{
			if (m_notifyOld)
			{
				WriteData_NotifyOld(newpipe, *item);
			}
			else
			{
				WriteData(newpipe, *item);
			}
		}
		catch(int)
		{
			CloseHandle(m_pipe);
			m_pipe = INVALID_HANDLE_VALUE;
		}
	}
	return S_OK;
}

HANDLE CLOD_Pipe::OpenPipe(const wchar_t* name)
{
	wchar_t pipename[1024] = L"\\\\.\\pipe\\";
	wcsncat_s(pipename, name, _TRUNCATE);
	HANDLE pipe = ::CreateFileW(pipename, 
		GENERIC_WRITE, 0, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (pipe == INVALID_HANDLE_VALUE)
	{
		DWORD dwErr = GetLastError();
		LOG(L"管道打开失败:", tostr(dwErr));
	}
	return pipe;
}

void CLOD_Pipe::WriteData(bool newpipe, const LogItem& item)
{
	m_stream.clear();

	if (newpipe)
	{
		// 头信息，连上管道后传输一次
		m_stream.store((UINT32)2);                              // version
		m_stream.store((UINT32)4);                              // header content len
		m_stream.store((UINT32)::GetCurrentProcessId());        // pid
	}

	size_t taglen = wcslen(item.tag);
	size_t contentlen = wcslen(item.content);
	size_t uclen = wcslen(item.userContext);
	size_t totallen = 32 + taglen * 2 + contentlen * 2 + uclen * 2 + 4*3;

	m_stream.store((UINT32)totallen);
	m_stream.store((UINT64)item.id);
	m_stream.store((UINT64)item.unixTime);
	m_stream.store((UINT32)item.microSecond);
	m_stream.store((UINT32)item.tid);
	m_stream.store((UINT32)item.level);
	m_stream.store((UINT32)item.depth);
	m_stream.store_str_prefix_len(item.tag, taglen);
	m_stream.store_str_prefix_len(item.content, contentlen);
	m_stream.store_str_prefix_len(item.userContext, uclen);

	WritePipe(m_pipe, m_stream, m_stream.len());
}

void CLOD_Pipe::WriteData_NotifyOld(bool newpipe, const LogItem& /*item*/)
{
	m_stream.clear();

	if (newpipe)
	{
		m_stream.store((UINT32)GetCurrentProcessId());
	}

	char buffer[256] = {0};
	m_stream.store_buffer(buffer, 24);
	m_stream.store((UINT32)64);
	const wchar_t* tag = L"error";
	const wchar_t* msg = L"您目前的日志查看器版本较旧，日志内容无法显示，请获取最新的日志查看器版本使用。";
	m_stream.store_str_prefix_len(tag);
	m_stream.store_str_prefix_len(msg);

	WritePipe(m_pipe, m_stream, m_stream.len());


	m_notified = true;
	Close();
}
