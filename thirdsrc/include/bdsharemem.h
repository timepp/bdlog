#pragma once

#include "bdsecurity.h"
#include <stdio.h>

class CSharingMemory
{
	HANDLE m_hFileMapping;
	LPVOID m_pData;
	BOOL m_bOpen;
	HANDLE m_hMutex;

public:
	CSharingMemory() 
		: m_hFileMapping(NULL), m_pData(NULL), m_bOpen(FALSE), m_hMutex(NULL)
	{
	}

	~CSharingMemory()
	{
		Close();
	}

	HRESULT GetLastErrorAsHRESULT()
	{
		DWORD dwError = GetLastError();
		return HRESULT_FROM_WIN32(dwError);
	}

	HRESULT Open(LPCWSTR name, size_t len, LPSECURITY_ATTRIBUTES sa)
	{
		HRESULT ret = S_OK;
		Close();

		m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, sa, PAGE_READWRITE, 0, len, name);
		if(!m_hFileMapping)
		{
			return GetLastErrorAsHRESULT();
		}

		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			ret = S_FALSE;
		}

		m_pData = MapViewOfFile(m_hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, len);
		if(!m_pData)
		{
			ret = GetLastErrorAsHRESULT();
			CloseHandle(m_hFileMapping);
			return ret;
		}

		WCHAR mutexName[1024] = L"locker_";
		WCHAR* p = mutexName + 7;
		while (*name && p <= mutexName + 256) *p++ = *name++;
		*p = L'\0';
		m_hMutex = CreateMutexW(NULL, FALSE, mutexName);

		m_bOpen = TRUE;
		return ret;
	}

	void Close()
	{
		if(m_pData)
		{
			UnmapViewOfFile(m_pData);
			m_pData = NULL;
		}

		if(m_hFileMapping)
		{
			CloseHandle(m_hFileMapping);
			m_hFileMapping = NULL;
		}

		if (m_hMutex)
		{
			CloseHandle(m_hMutex);
			m_hMutex = NULL;
		}

		m_bOpen = FALSE;
	}

	BOOL IsOpen() const
	{
		return m_bOpen;
	}

	BOOL Lock()
	{
		if (!m_hMutex) return FALSE;

		if (::WaitForSingleObject(m_hMutex, INFINITE) == WAIT_FAILED)
		{
			return FALSE;
		}
		
		return TRUE;
	}

	BOOL UnLock()
	{
		if (!m_hMutex) return FALSE;

		return ReleaseMutex(m_hMutex);
	}

	LPVOID GetData() const
	{
		return m_pData;
	}

	template <typename T>
	T& GetDataAs() const
	{
		return *reinterpret_cast<T*>(m_pData);
	}
};
