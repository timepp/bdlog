#pragma once

#include "outputdevice_common.h"

class CLOD_File : public CLogOutputDeviceBase
{
private:
	wchar_t m_path[MAX_PATH];
	bool m_syncMode;
	HANDLE m_file;

	char* m_buffer;
	size_t m_bufferLen;
	size_t m_pos;

	wchar_t m_cvtbuf[4096];

	// 上次日志的时间(秒数)
	FILETIME m_lastTime;

	OVERLAPPED m_overlap;

public:
	virtual HRESULT Open(ILogOption* opt)
	{
		TRUNCATED_COPY(m_path, opt->GetOption(L"path", L""));
		DWORD shareMode = opt->GetOptionAsBool(L"share_read", true) ? FILE_SHARE_READ : 0U;

		// 自定义变量和环境变量扩展
		WCHAR path[MAX_PATH];
		helper::ExpandVariable(m_path, path, _countof(path));
		::ExpandEnvironmentStringsW(path, m_path, _countof(m_path));

		helper::MakeRequiredDirectory(m_path);
		m_file = ::CreateFileW(m_path, GENERIC_WRITE, shareMode, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_file == INVALID_HANDLE_VALUE)
		{
			return helper::GetLastErrorAsHRESULT();
		}

		m_bufferLen = static_cast<size_t>(opt->GetOptionAsInt(L"buffer_size", 1000000));
		m_pos = 0;
		if (m_bufferLen > 0)
		{
			m_buffer = new char[m_bufferLen];
		}

		m_syncMode = !opt->GetOptionAsBool(L"async", false);

		LONG posL = 0;
		LONG posH = 0;
		posL = (LONG)::SetFilePointer(m_file, posL, &posH, FILE_END);
		if (posL == 0 && posH == 0)
		{
			Write("\xFF\xFE", 2);
		}

		return S_OK;
	}

	virtual HRESULT Close()
	{
		if (m_file != INVALID_HANDLE_VALUE)
		{
			Flush();
			::CloseHandle(m_file);
			m_file = INVALID_HANDLE_VALUE;
		}
		return S_OK;
	}

	virtual HRESULT Write(const LogItem* item)
	{
		const DWORD E7 = 10000000U;
		const DWORD E6 = 1000000U;

		textstream s(m_cvtbuf, _countof(m_cvtbuf));

		// 如果两次日志秒数相同，后一次日志就可以直接使用前一次格式化过的时间串，没必要再格式化一遍
		if (m_lastTime.dwLowDateTime / E7 != item->time.dwLowDateTime / E7 ||
			m_lastTime.dwHighDateTime != item->time.dwHighDateTime)
		{
			m_lastTime = item->time;
			helper::FileTimeToString(item->time, L"Y-m-d H:M:S", m_cvtbuf, _countof(m_cvtbuf));
		}

		s.advance(19);

		DWORD us = item->time.dwLowDateTime % E7 / 10;
		wchar_t tmp[128];
		wsprintfW(tmp, L".%06u %02d [X:%X:%X:%d] {", us, item->level, item->pid, item->tid, item->depth);
		s << tmp;

		const wchar_t* tag = item->tag;
		if (!tag || !*tag) tag = L" ";
		s << tag << L"} ";

		size_t contentLen = wcslen(item->content);
		s << lstr(item->content, contentLen);

		if (contentLen > 0 && item->content[contentLen-1] != L'\n')
		{
			s << L'\n';
		}

		Write(m_cvtbuf, s.len() * 2);

		return S_OK;

	}

	virtual HRESULT Flush()
	{
		if (m_pos > 0)
		{
			if (m_syncMode)
			{
				SyncWriteFile(m_file, m_buffer, m_pos);
			}
			else
			{
				AsyncWriteFile(m_file, m_buffer, m_pos);
			}
			m_pos = 0;
		}

		return S_OK;
	}

	virtual HRESULT OnConfigChange(ILogOption* opt)
	{
		Close();
		return Open(opt);
	}

	CLOD_File()
		: m_file(NULL), m_buffer(NULL), m_bufferLen(0), m_pos(0), m_syncMode(true)
	{
		m_lastTime.dwHighDateTime = m_lastTime.dwLowDateTime = 0;
		m_path[0] = L'\0';
	}

	~CLOD_File()
	{
		Close();
	}

private:

	void Write(const void* data, size_t len)
	{
		if (m_pos + len >= m_bufferLen)
		{
			Flush();
			m_pos = 0;
		}

		if (len >= m_bufferLen)
		{
			if (m_syncMode)
			{
				SyncWriteFile(m_file, data, len);
			}
			else
			{
				AsyncWriteFile(m_file, data, len);
			}
			m_pos = 0;
		}
		else
		{
			memcpy(m_buffer + m_pos, data, len);
			m_pos += len;
		}
	}

	BOOL AsyncWriteFile(HANDLE file, const void* buffer, size_t len)
	{
		m_overlap.Offset = 0xFFFFFFFF;
		m_overlap.OffsetHigh = 0xFFFFFFFF;
		::WriteFileEx(file, buffer, len, &m_overlap, &FileIOCompletionRoutine);
		return FALSE;
	}

	static VOID CALLBACK FileIOCompletionRoutine(
		__in     DWORD dwErrorCode,
		__in     DWORD /*dwNumberOfBytesTransfered*/,
		__inout  LPOVERLAPPED /*lpOverlapped*/
		)
	{
		LOG(ToStr(dwErrorCode, L"%u"));
	}

	static BOOL SyncWriteFile(HANDLE file, const void* buffer, size_t len)
	{
		DWORD bytesWrite;
		if (!::WriteFile(file, buffer, len, &bytesWrite, NULL))
		{
			LOGWINERR(L"写文件失败");
			return FALSE;
		}

		return TRUE;
	}
};
