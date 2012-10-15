/**
 *  这个文件在以头文件包式使用BDLOG的工程中包含
 */

#pragma once

#include "logcontroller.h"

#if !defined(BDLOG_SELF_BUILD) && !defined(BDLOG_USE_AS_DLL_DYNAMIC_LOAD) && !defined(BDLOG_USE_AS_DLL)

namespace bdlog
{
	struct helper
	{
		__declspec(noinline)
			static ILogController* GetLogController()
		{
#pragma warning(push)
#pragma warning(disable: 4640)
			static CLogController s_controller;  // 请确保在所有新线程启动之前调用GetLogController
#pragma warning(pop)

			return &s_controller;
		}
	};
}

BDLOGAPI inline ILogController* GetLogController()
{
	return bdlog::helper::GetLogController();
}
#endif

