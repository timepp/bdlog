#pragma once

#include <tplib/auto_release.h>

class cslock
{
public:
	cslock()
	{
		::InitializeCriticalSection(&m_cs);
	}
	~cslock()
	{
		::DeleteCriticalSection(&m_cs);
	}
	void lock()
	{
		::EnterCriticalSection(&m_cs);
	}
	void unlock()
	{
		::LeaveCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION m_cs;
};

class autolocker
{
public:
	autolocker(cslock& cs) : m_cs(&cs) { cs.lock(); }
	~autolocker() { m_cs->unlock(); }
private:
	cslock* m_cs;
};

#define MULTI_TRHEAD_GUARD(cs) autolocker UNIQUE_NAME(al_)(cs)


struct ToStr
{
	template <typename T>
	explicit ToStr(T val, const wchar_t* fmt)
	{
		wsprintfW(m_buffer, fmt, val);
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
	static void FileTimeToString(FILETIME ft, const wchar_t* fmt, wchar_t* buf, size_t buflen);
	static FILETIME UnixTimeToFileTime(__int64 t);

	static wchar_t* IntToStr_Padding0(wchar_t* str, size_t strlen, size_t len, int val);

	/// be careful!!
	static wchar_t* IntToStr(int val, wchar_t* str);
	static wchar_t* IntToHexStr(int val, wchar_t* str);

	static int StrToInt(const wchar_t* src);

	static BOOL MakeRequiredDirectory(const wchar_t* path);

	static HRESULT GetLastErrorAsHRESULT();
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

struct lstr
{
	lstr(const wchar_t*s, size_t l) : str(s), len(l)
	{
	}
	const wchar_t* str;
	size_t len;
};

#define LSTR(s) lstr(s, _countof(s)-1)

class textstream
{
private:
	wchar_t* m_buffer;
	wchar_t* m_p;
	size_t m_avail;
public:
	textstream(wchar_t* buffer, size_t len)
		: m_buffer(buffer), m_avail(len), m_p(buffer)
	{
	}

	operator wchar_t* ()
	{
		return m_buffer;
	}

	textstream& operator <<(const wchar_t* str)
	{
		store(str, wcslen(str));
		return *this;
	}
	textstream& operator <<(lstr str)
	{
		store(str.str, str.len);
		return *this;
	}
	textstream& operator <<(wchar_t ch)
	{
		if (m_avail > 1)
		{
			*m_p = ch;
			advance(1);
		}
		return *this;
	}

	void advance(size_t len)
	{
		if (len >= m_avail) len = m_avail - 1;
		m_p += len;
		m_avail -= len;
		*m_p = L'\0';
	}

	size_t len() const
	{
		return static_cast<size_t>(m_p - m_buffer);
	}

private:
	void store(const wchar_t* str, size_t len)
	{
		if (m_avail <= 1) return;
		if (len >= m_avail) len = m_avail - 1;
		memcpy(m_p, str, len * 2);
		advance(len);
	}
};

#define TRUNCATED_COPY(d, s) textstream(d, _countof(d)) << s

#define LOG(...) InternalLogClass::InternalLog(__VA_ARGS__)
#define LOGWINERR(str) InternalLogClass::InternalLogWinError(str)
#define LOGFUNC LOG(WIDESTRING(__FUNCTION__))
#define CHECK_HR(s)	if (FAILED(s)) {ATLASSERT(FALSE);}
#define ENSURE_SUCCEED(s) {HRESULT hr_ = s; if (FAILED(hr_)) { return hr_; } }
