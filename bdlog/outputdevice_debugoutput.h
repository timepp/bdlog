#pragma once

#include "outputdevice_common.h"
#include "logutil.h"

class CLOD_DebugOutput : public CLogOutputDeviceBase
{
public:
	virtual HRESULT Open(ILogOption* opt);
	virtual HRESULT Close();
	virtual HRESULT Write(const LogItem* item);
	virtual HRESULT Flush();
	virtual HRESULT OnConfigChange(ILogOption* opt);

private:
	binarystream m_stream;
};
