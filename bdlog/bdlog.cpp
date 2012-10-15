#include "stdafx.h"

#include <bdlog.h>
#include "logcontroller.h"

#ifdef BDLOG_BUILD_STATIC_LIB

// 在static lib的情况下，因为全局变量的初始化顺序无法保证，所以也就无法保证LogController在足够早的时间初始化。
// 于是在static lib的情况下，LogController被实现为：在第一次获取的时候初始化
// 但这不是线程安全的，所以请确保在新线程启动之前初始化LogController

BDLOGAPI ILogController* GetLogController()
{
#pragma warning(push)
#pragma warning(disable: 4640)
	static CLogController s_controller;  // 请确保在所有新线程启动之前调用GetLogController
#pragma warning(pop)

	return &s_controller;
}

#else // no BDLOG_BUILD_STATIC_LIB

static char Room_For_LogControler[sizeof(CLogController)];

BDLOGAPI ILogController* GetLogController()
{
	return (CLogController*)Room_For_LogControler;
}

BOOL APIENTRY DllMain( HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		new(Room_For_LogControler) CLogController;
	}

	return TRUE;
}

#endif
