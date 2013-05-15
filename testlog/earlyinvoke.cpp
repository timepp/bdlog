#include "stdafx.h"

// 测试在全局对象构造和析构过程中调用tplog是否正常

struct GlobalObj
{
	GlobalObj()
	{
		Log(LL_EVENT, NOTAG, L"全局对象构造");
	}
	~GlobalObj()
	{
		Log(LL_EVENT, NOTAG, L"全局对象析构 [0X%p]", this);
	}
}gobj;

