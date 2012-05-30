#pragma once

#include <set>
#include <vector>
#include <deque>
#include <bdlogprovider.h>

typedef std::set<CStringW> StrSet;

struct LogInfo
{
	UINT64 logid;
	bdlog::logitem* item;
};

struct disp_info
{
	COLORREF color;
	COLORREF bkcolor;
	disp_info()
		: color(RGB(0, 0, 0)), bkcolor(RGB(255,255,255))
	{
	}
};


#define BSP_BDXLOG_INI           L"<CSIDL:COMMON_APPDATA>\\Baidu\\<VAR:PRODUCT>\\bdlog.ini"
#define BSP_DEBUGSET_INI         L"<CSIDL:APPDATA>\\Baidu\\<VAR:PRODUCT>\\debugset.ini"

#define WM_FILTER_CHANGE     WM_USER + 21

