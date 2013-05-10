/**
 *  这个文件在以头文件形式使用BDLOG的工程中包含
 */

#pragma once

#include "detail/logcontroller.h"


namespace
{
		__declspec(noinline)
	static ILogController* RealGetLogController()
		{
#pragma warning(push)
#pragma warning(disable: 4640)
			static CLogController s_controller;  // 请确保在所有新线程启动之前调用GetLogController
#pragma warning(pop)

			return &s_controller;
		}
}

BDLOGAPI inline ILogController* GetLogController()
{
	return ::RealGetLogController();
}


