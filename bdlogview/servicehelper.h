#pragma once

#include "serviceid.h"

#include "logpropdb.h"
#include "logcenter.h"
#include "logicvis.h"

struct ServiceHelper
{
	static CLogPropertyDB* GetLogPropertyDB()
	{
		return tp::servicemgr::get<CLogPropertyDB>();
	}
	static CLogCenter* GetLogCenter()
	{
		return SERVICE(CLogCenter);
	}
	static CVisualLogic* GetVisualLogic()
	{
		return SERVICE(CVisualLogic);
	}
};
