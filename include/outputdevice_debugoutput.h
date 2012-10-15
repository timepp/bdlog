#pragma once

#include "outputdevice_common.h"
#include "logutil.h"

class CLOD_DebugOutput : public CLogOutputDeviceBase
{
public:
	virtual HRESULT Open(ILogOption* /*opt*/)
	{
		return S_OK;
	}

	virtual HRESULT Close()
	{
		return S_OK;
	}

	virtual HRESULT Write(const LogItem* item)
	{
		wchar_t timestr[32];
		helper::FileTimeToString(item->time, L"Y-m-d H:M:S ", timestr, _countof(timestr));

		wchar_t buffer[1024];
		textstream s(buffer, _countof(buffer));
		s << timestr << item->content << L'\n';

		::OutputDebugStringW(buffer);
		return S_OK;
	}
	virtual HRESULT Flush()
	{
		return S_OK;
	}
	virtual HRESULT OnConfigChange(ILogOption* /*opt*/)
	{
		return S_OK;
	}
};
