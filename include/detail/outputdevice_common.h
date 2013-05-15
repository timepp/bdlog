#pragma once

#include "../tplog.h"
#include "logutil.h"

class CLogOutputDeviceBase : public ILogOutputDevice
{
public:
	CLogOutputDeviceBase() : m_refcounter(0)
	{

	}

	virtual void IncreaseRefCounter()
	{
		m_refcounter++;
	}
	virtual void DecreaseRefCounter()
	{
		m_refcounter--;
		if (m_refcounter == 0)
		{
			delete this;
		}
	}

private:
	int m_refcounter;
};

class CLOD_Null : public CLogOutputDeviceBase
{
public:
	virtual HRESULT Open(ILogOption* ) { return S_OK; }
	virtual HRESULT Close() { return S_OK; }
	virtual HRESULT Write(const LogItem* ) { return S_OK; }
	virtual HRESULT Flush() { return S_OK; }
	virtual HRESULT OnConfigChange(ILogOption* ) { return S_OK; }
};
