#pragma once

#include "outputdevice_common.h"

class CLOD_Pipe : public CLogOutputDeviceBase
{
private:
	HANDLE m_pipe;
	DWORD m_lastTryConnectTime;
	wchar_t m_pipeName[256];

	binarystream m_stream;

public:
	virtual HRESULT Open(ILogOption* opt)
	{
		Close();
		TRUNCATED_COPY(m_pipeName, opt->GetOption(L"name", L"tplog_data_channel"));

		return S_OK;
	}
	virtual HRESULT Close()
	{
		if (m_pipe != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(m_pipe);
			m_pipe = INVALID_HANDLE_VALUE;
		}
		return S_OK;
	}
	virtual HRESULT Write(const LogItem* item)
	{
		bool newpipe = false;
		if (m_pipe == INVALID_HANDLE_VALUE)
		{
			DWORD currTime = ::GetTickCount();
			if (currTime >= m_lastTryConnectTime + 10)
			{
				m_pipe = OpenPipe(m_pipeName);
				m_lastTryConnectTime = currTime;
				newpipe = true;
			}
		}
		if (m_pipe != INVALID_HANDLE_VALUE)
		{
			HRESULT hr = S_OK;
			hr = WriteLogToPipe(*item);

			if (FAILED(hr))
			{
				CloseHandle(m_pipe);
				m_pipe = INVALID_HANDLE_VALUE;
			}
		}
		return S_OK;
	}
	virtual HRESULT Flush()
	{
		return S_OK;
	}
	virtual HRESULT OnConfigChange(ILogOption* opt)
	{
		const wchar_t* newname = opt->GetOption(L"name", m_pipeName);
		if (wcscmp(m_pipeName, newname) != 0)
		{
			TRUNCATED_COPY(m_pipeName, newname);
			Close();
		}

		return S_OK;
	}

	CLOD_Pipe()
		: m_pipe(INVALID_HANDLE_VALUE)
		, m_lastTryConnectTime(0)
	{

	}
	~CLOD_Pipe()
	{
		Close();
	}

private:
	static HANDLE OpenPipe(const wchar_t* name)
	{
		wchar_t pipename[1024];
		textstream s(pipename, _countof(pipename));
		s << LSTR(L"\\\\.\\pipe\\") << name;
		HANDLE pipe = ::CreateFileW(pipename, 
			GENERIC_WRITE, 0, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (pipe == INVALID_HANDLE_VALUE)
		{
			DWORD dwErr = GetLastError();
			LOG(L"管道打开失败:", ToStr(dwErr, L"%u"));
		}

		// 发送初始数据
		binarystream bs;
		bs.store((UINT32)3);                              // version
		bs.store((UINT32)4);                              // header content len
		bs.store((UINT32)::GetCurrentProcessId());        // pid
		HRESULT hr = WritePipe(pipe, bs, bs.len());
		if (FAILED(hr))
		{
			::CloseHandle(pipe);
			pipe = NULL;
		}

		return pipe;
	}
	static HRESULT WritePipe(HANDLE pipe, LPCVOID buffer, DWORD len)
	{
		DWORD bytesWrite;
		if (!WriteFile(pipe, buffer, len, &bytesWrite, NULL))
		{
			return helper::GetLastErrorAsHRESULT();
		}
		return S_OK;
	}
	HRESULT WriteLogToPipe(const LogItem& item)
	{
		m_stream.clear();

		size_t taglen = wcslen(item.tag);
		size_t contentlen = wcslen(item.content);
		size_t totallen = 4 * 7 + (4 + taglen * 2) + (4 + contentlen * 2);

		m_stream.store((UINT32)totallen);
		m_stream.store((UINT64)item.id);
		m_stream.store((UINT32)item.time.dwHighDateTime);
		m_stream.store((UINT32)item.time.dwLowDateTime);
		m_stream.store((UINT32)item.tid);
		m_stream.store((UINT32)item.level);
		m_stream.store((UINT32)item.depth);
		m_stream.store_str_prefix_len(item.tag, taglen);
		m_stream.store_str_prefix_len(item.content, contentlen);

		return WritePipe(m_pipe, m_stream, m_stream.len());
	}
};
