#pragma once

#include "serviceid.h"
#include "service.h"

#include "logpropdb.h"
#include "logcenter.h"
#include "logicvis.h"

struct ServiceHelper
{
	static CServicePtr<CLogPropertyDB> GetLogPropertyDB()
	{
		return ServiceMgr::GetService<CLogPropertyDB>();
	}
	static CLogCenter* GetLogCenter()
	{
		return ServiceMgr::GetService<CLogCenter>().operator->();
	}
	static CServicePtr<CVisualLogic> GetVisualLogic()
	{
		return ServiceMgr::GetService<CVisualLogic>();
	}
};
