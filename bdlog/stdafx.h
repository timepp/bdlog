#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include <compilerconf.h>

BD_NO_WARNING_AREA_BEGIN()

#include <windows.h>

BD_NO_WARNING_AREA_END()
BD_CLEAR_WARNING_STACK()

#pragma warning(disable: 4514) // warning C4514: 'HRESULT_FROM_WIN32' : unreferenced inline function has been removed
#pragma warning(disable: 4710) // warning C4710: 'void BDLog::Log(const wchar_t *,...)' : function not inlined
#pragma warning(disable: 4711) // warning C4711: function 'xxx' selected for automatic inline expansion
#pragma warning(disable: 4820) // warning C4820: 'LogItem' : '4' bytes padding added after data member 'LogItem::depth'

#include "logutil.h"