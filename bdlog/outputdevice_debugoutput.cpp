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
	wchar_t timestr[64];
	helper::UnixTimeToString(item->unixTime, L"Y-m-d H:M:S ", timestr, _countof(timestr));
	
	m_stream.clear();
	m_stream.store_str(timestr);
	m_stream.store_str(item->content);
	m_stream.store_buffer(L"\n", 4);
	
	::OutputDebugStringW((LPCWSTR)(LPCVOID)m_stream);
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
