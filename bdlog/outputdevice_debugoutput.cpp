#include "stdafx.h"
#include "outputdevice_debugoutput.h"

HRESULT CLOD_DebugOutput::Open(ILogOption* /*opt*/)
{
	return S_OK;
}

HRESULT CLOD_DebugOutput::Close()
{
	return S_OK;
}

HRESULT CLOD_DebugOutput::Write(const LogItem* item)
{
	wchar_t timestr[32];
	helper::FileTimeToString(item->time, L"Y-m-d H:M:S ", timestr, _countof(timestr));
	
	wchar_t buffer[1024];
	textstream s(buffer, _countof(buffer));
	s << timestr << item->content << L"\n";
	
	::OutputDebugStringW(buffer);
	return S_OK;
}

HRESULT CLOD_DebugOutput::Flush()
{
	return S_OK;
}

HRESULT CLOD_DebugOutput::OnConfigChange(ILogOption* /*opt*/)
{
	return S_OK;
}
