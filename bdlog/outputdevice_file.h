#pragma once

#include "outputdevice_common.h"

template <size_t SIZE>
class CCharBuffer
{
public:
	CCharBuffer()
	{

	}
	~CCharBuffer()
	{

	}

	size_t Store(wchar_t val)
	{
		if (m_len < _countof(m_buffer) - 1)
		{
			m_buffer[m_len] = val;
			return 0;
		}
		return 1;
	}
	size_t Store(const wchar_t* val)
	{
		for (size_t i = 0; val[i] && i + m_len < _countof(m_buffer)-1; i++)
		{
			m_buffer[i + m_len] = val[i];
		}
		m_len += i;
		return wcslen(val + i);
	}
	size_t Store(const wchar_t* val, size_t len = 0)
	{
		for (size_t i = 0; i < len && i + m_len < _countof(m_buffer)-1; i++)
		{
			m_buffer[i + m_len] = val[i];
		}
		m_len += i;
		return len - i;
	}
	wchar_t* Get()
	{
		m_buffer[m_len] = L'\0';
		return m_buffer;
	}

	wchar_t m_buffer[SIZE];
	size_t m_len;
};

class CLOD_File : public CLogOutputDeviceBase
{
public:
	virtual HRESULT Open(ILogOption* opt);
	virtual HRESULT Close();
	virtual HRESULT Write(const LogItem* item);
	virtual HRESULT Flush();
	virtual HRESULT OnConfigChange(ILogOption* opt);

	CLOD_File();
	~CLOD_File();

private:
	wchar_t m_path[MAX_PATH];
	bool m_syncMode;
	HANDLE m_file;

	char* m_buffer;
	size_t m_bufferLen;
	size_t m_pos;

	wchar_t m_cvtbuf[4096];

	// 上次日志的时间(秒数)
	__int64 m_lastTime;

	void Write(const void* data, size_t len);
};
