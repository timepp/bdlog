// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN


#include <compilerconf.h>
BD_NO_WARNING_AREA_BEGIN()

#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <atlbase.h>
#include <atlwin.h>
#include <atlcom.h>
#include <atlstr.h>

#include <iostream>
#include <vector>
#include <list>

BD_NO_WARNING_AREA_END()
BD_TUNE_WARNINGS

#include <bdlog.h>
#include <bdlog_util.h>

#include "profiler.h"