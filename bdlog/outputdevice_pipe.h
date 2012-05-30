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
	time_t m_lastTryConnectTime;
	CSimpleStr m_pipeName;
	bool m_notifyOld;
	bool m_notified;

	binarystream m_stream;

	static HANDLE OpenPipe(const wchar_t* name);
	void WriteLogToPipe(const LogItem& item);

	void WriteData_NotifyOld(bool newpipe, const LogItem& item);
	void WriteData(bool newpipe, const LogItem& item);
};
