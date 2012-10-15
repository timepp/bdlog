#pragma once

#include "outputdevice_common.h"
#include "logutil.h"

class CLOD_Pipe : public CLogOutputDeviceBase
{
public:
	virtual HRESULT Open(ILogOption* opt);
	virtual HRESULT Close();
	virtual HRESULT Write(const LogItem* item);
	virtual HRESULT Flush();
	virtual HRESULT OnConfigChange(ILogOption* opt);

	CLOD_Pipe();
	~CLOD_Pipe();

private:
	HANDLE m_pipe;
	DWORD m_lastTryConnectTime;
	wchar_t m_pipeName[256];

	binarystream m_stream;

	static HANDLE OpenPipe(const wchar_t* name);
	HRESULT WriteLogToPipe(const LogItem& item);
};
