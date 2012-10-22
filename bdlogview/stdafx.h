#pragma once

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif						

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#define WIN32_LEAN_AND_MEAN
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define _WTL_NO_WTYPES
#define _WTL_NO_CSTRING

#define ATL_DEBUG_INTERFACES

#include <compilerconf.h>

BD_NO_WARNING_AREA_BEGIN()

#include <windows.h>
#include <shellapi.h>

#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atltypes.h>
#include <atlwin.h>
#include <atlsync.h>
#include <atltime.h>

#include <wtl/atlapp.h>
#include <wtl/atlctrls.h>
#include <wtl/atlctrlx.h>
#include <wtl/atlframe.h>
#include <wtl/atlcrack.h>
#include <wtl/atlctrlw.h>
#include <wtl/atldlgs.h>
#include <wtl/atlscrl.h>
#include <wtl/atlmisc.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <deque>

BD_NO_WARNING_AREA_END()
BD_TUNE_WARNINGS

#pragma warning(disable: 4100)

#include <tplib/include/auto_release.h>
#include <tplib/include/format_shim.h>
#include <tplib/include/service.h>

#define SERVICE(x) tp::servicemgr::get<x>()
