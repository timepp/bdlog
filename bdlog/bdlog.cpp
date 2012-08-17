#include "stdafx.h"

#include <bdlog.h>
#include "logcontroller.h"

#ifdef BDLOG_USE_AS_STATIC_LIB
// 在LIB模式下，必须保证多个二进制模块引用的是同一个log controller对象，
// 这在DLL方式下是没有问题的，但是在LIB方式下就不行了，因为多个二进制模块可能都静态集成了bdlog lib

CLogController* g_logControllerPtr = NULL;
BDLOGAPI ILogController* GetLogController()
{
	if (g_logControllerPtr == NULL)
	{
		wchar_t envname[] = L"BDLOG_V2_ADDR_000000000000";
		{
			// 生成一个进程唯一的环境变量名
			DWORD pid = ::GetCurrentProcessId();
			wchar_t* p = envname + 14;
			while (pid > 0)
			{
				*p++ = pid % 10 + L'0';
				pid /= 10;
			}
		}

		wchar_t buf[256] = L"0";
		if (::GetEnvironmentVariableW(envname, buf, _countof(buf)) > 0)
		{
			swscanf_s(buf, L"%p", &g_logControllerPtr);
		}
		if (g_logControllerPtr == NULL)
		{
			static CLogController s_ctrl;
			g_logControllerPtr = &s_ctrl;
			_snwprintf_s(buf, _TRUNCATE, L"%p", g_logControllerPtr);
			::SetEnvironmentVariableW(envname, buf);
		}
	}

	return g_logControllerPtr;
}

#else // no BDLOG_USE_AS_LIB
CLogController g_logController;
BDLOGAPI ILogController* GetLogController()
{
	return &g_logController;
}
#endif // BDLOG_USE_AS_LIB


