#include "stdafx.h"

#include <detail/logcontroller.h>

void * operator new (unsigned int /*size*/, void * p)
{ 
	return p ; 
}

static char Room_For_LogControler[sizeof(CLogController)];

extern "C" __declspec(dllexport)
ILogController* GetLogController()
{
	return (CLogController*)Room_For_LogControler;
}

BOOL APIENTRY DllMain( HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		new(Room_For_LogControler) CLogController;
	}
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		GetLogController()->UnInit();
	}

	return TRUE;

}

#define WIDE_STR_(x) L##x
#define WIDE_STR(x) WIDE_STR_(x)

// 使用symview检查产出是否有“不该被链接”字样的符号出现，若有，说明不小心链接了相应的函数，应该检查并消灭掉
#define SHOULD_NOT_LINK(reason)  LOG(L"不该被链接(" reason L"): " WIDE_STR(__FUNCTION__))

/// 缺省实现的new/delete比较复杂，
/// bdlog中没有几处new/delete，用不到这么复杂的功能，直接使用HeapAlloc/HeapFree代替
void * operator new(unsigned int size)
{
	return ::HeapAlloc(::GetProcessHeap(), 0, size);
}
void operator delete(void* p)
{
	::HeapFree(::GetProcessHeap(), 0, p);
}

int _purecall()
{
	return 0;
}

/// 缺省的wcschr比较复杂(占用170多个字节，见wcschr.c)
/// bdlog中的场景都是在一个比较短的字符串中进行查找，自己写一个wcschr就够了
const wchar_t* wcschr(const wchar_t* src, wchar_t ch)
{
	while (*src)
	{
		if (*src == ch) return src;
		src++;
	}
	return NULL;
}

/// 以下是不应该被链接进来的函数
#ifndef _DEBUG
int _itow_s(int /*val*/, wchar_t* /*buf*/, size_t /*buflen*/, int /*redix*/)
{
	SHOULD_NOT_LINK(L"缺省实现太大");
	return 0;
}

int wcsncpy_s(wchar_t* /*dest*/, size_t /*destlen*/, const wchar_t* /*src*/, size_t /*copylen*/)
{
	SHOULD_NOT_LINK(L"缺省实现太大");
	return 0;
}

// error LNK2005: __wtoi already defined in LIBCMT.lib(wtox.obj)
int wcsncat_s(wchar_t* /*dest*/, size_t /*destlen*/, const wchar_t* /*src*/, size_t /*copylen*/)
{
	SHOULD_NOT_LINK(L"缺省实现太大");
	return 0;
}

int _wtoi(const wchar_t* /*str*/)
{
	SHOULD_NOT_LINK(L"缺省实现太大");
	return 0;
}

int atexit ( void ( * /*func*/ ) (void) )
{
	SHOULD_NOT_LINK(L"没有运行库的初始化，不支持atexit");
	return 0;
}

#endif