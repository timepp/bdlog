#pragma once

#include <bdlog.h>

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

class CSimpleStr
{
public:
	CSimpleStr() : m_buffer(0)
	{

	}
	~CSimpleStr()
	{
		delete [] m_buffer;
		m_buffer = NULL;
	}
	void SetStr(const wchar_t* str)
	{
		if (m_buffer) delete [] m_buffer;
		size_t len = wcslen(str);
		m_buffer = new wchar_t[len+1];
		memcpy(m_buffer, str, sizeof(wchar_t)*(len+1));
	}
	operator const wchar_t*()
	{
		return m_buffer;
	}
private:
	wchar_t* m_buffer;
};
