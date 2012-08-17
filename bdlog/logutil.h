#pragma once

#include <tplib/auto_release.h>

#define MULTI_TRHEAD_GUARD(cs) \
	CRITICAL_SECTION* pcs = &cs; \
	::EnterCriticalSection(pcs); \
	ON_LEAVE_1(::LeaveCriticalSection(pcs), CRITICAL_SECTION*, pcs);

inline HRESULT LastError_HRESULT()
{
	DWORD dwErr = ::GetLastError();
	return HRESULT_FROM_WIN32(dwErr);
}

#include <stdlib.h>

struct tostr
{
	tostr(int val)
	{
		_itow_s(val, m_buffer, 10);
	}
	tostr(unsigned long val)
	{
		_itow_s(static_cast<int>(val), m_buffer, 10);
	}
	operator const wchar_t*()
	{
		return m_buffer;
	}
	wchar_t m_buffer[32];
};

struct InternalLogClass
{
	__declspec(noinline) static void InternalLog(const wchar_t* buffer1, const wchar_t* buffer2 = L"");
	__declspec(noinline) static void InternalLogWinError(const wchar_t* buffer);
};

struct helper
{
	/** 变量扩展
	 *  ${PID} ==> 当前进程的PID
	 */
	static void ExpandVariable(const wchar_t* src, wchar_t* dest, size_t destlen);

	/** 时间转成字符串
	 *  不使用wcsftime, 丧失了少许灵活性，但可以使程序大小减少20K，性能提升3倍
	 *
	 *  \param fmt 特殊字符: Y=4位年 m=2位月 d=2位日 H=2位小时 M=2位分钟 S=2位秒
	 */
	static void UnixTimeToString(__int64 t, const wchar_t* fmt, wchar_t* buf, size_t buflen);

	static wchar_t* IntToStr_Padding0(wchar_t* str, size_t strlen, size_t len, int val);

	static BOOL MakeRequiredDirectory(const wchar_t* path);
};

class binarystream
{
public:
	binarystream() : m_buffer(m_static_buffer), m_capacity(sizeof(m_static_buffer)), m_len(0)
	{
	}
	~binarystream()
	{
		if (m_buffer != m_static_buffer)
			delete [] m_buffer;
	}

	void clear() {	m_len = 0; }
	size_t len() { return m_len; }

	operator const void* () { return m_buffer; }

	void store_buffer(const void* buffer, size_t len)
	{
		size_t cap = m_capacity;
		while (m_len + len > cap)
		{
			cap *= 2;
		}
		enlarge(cap);

		memcpy(m_buffer + m_len, buffer, len);
		m_len += len;
	}

	void store_str(const wchar_t* str, size_t len = 0)
	{
		if (len == 0) len = wcslen(str);
		len *= 2;
		store_buffer(str, len);
	}

	void store_str_prefix_len(const wchar_t* str, size_t len = 0)
	{
		if (len == 0) len = wcslen(str);
		len *= 2;
		store(static_cast<UINT32>(len));
		store_buffer(str, len);
	}

	template <typename T>
	void store(const T& val)
	{
		store_buffer(&val, sizeof(val));
	}

private:
	char* m_buffer;
	char m_static_buffer[256];
	size_t m_capacity;
	size_t m_len;
	size_t m_read_pos;

	void enlarge(size_t cap)
	{
		if (m_capacity >= cap) return;

		char* buffer = new char[cap];
		memcpy(buffer, m_buffer, m_len);
		if (m_buffer != m_static_buffer)
		{
			delete [] m_buffer;
		}
		m_buffer = buffer;
		m_capacity = cap;
	}
};

#define LOG(...) InternalLogClass::InternalLog(__VA_ARGS__)
#define LOGWINERR(str) InternalLogClass::InternalLogWinError(str)
#define LOGFUNC LOG(WIDESTRING(__FUNCTION__))
#define CHECK_HR(s)	if (FAILED(s)) {ATLASSERT(FALSE);}
#define ENSURE_SUCCEED(s) {HRESULT hr_ = s; if (FAILED(hr_)) { return hr_; } }
