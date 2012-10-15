#pragma once

#include "outputdevice_common.h"

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
	FILETIME m_lastTime;

	void Write(const void* data, size_t len);
};
