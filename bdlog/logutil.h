#pragma once

#include "stream.h"
#include "lock.h"

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
	static wchar_t* IntToStr(int val, wchar_t* str);
	static wchar_t* IntToHexStr(int val, wchar_t* str);
	static int StrToInt(const wchar_t* src);

	static BOOL MakeRequiredDirectory(const wchar_t* path);

	static HRESULT helper::GetLastErrorAsHRESULT()
	{
		DWORD dwErr = ::GetLastError();
		return HRESULT_FROM_WIN32(dwErr);
	}

	__declspec(noinline) static void InternalLog(const wchar_t* buffer1, const wchar_t* buffer2 = L"");
	__declspec(noinline) static void InternalLogWinError(const wchar_t* buffer);

	static bool StringEqual(const wchar_t* ss, const wchar_t* se, const wchar_t* dest)
	{
		size_t len = wcslen(dest);
		if (ss + len != se) return false;
		return wcsncmp(ss, dest, len) == 0;
	}

	static wchar_t* SafeCopyString(const wchar_t* src, size_t srclen, wchar_t* dest, size_t destlen)
	{
		size_t copylen = min(srclen, destlen);
		memcpy(dest, src, copylen * sizeof(wchar_t));
		return dest + copylen;
	}

private:
	static bool SelfLogEnabled();
};


#define LOG(...) helper::InternalLog(__VA_ARGS__)
#define LOGWINERR(str) helper::InternalLogWinError(str)
#define LOGFUNC LOG(WIDESTRING(__FUNCTION__))
#define CHECK_HR(s)	if (FAILED(s)) {ATLASSERT(FALSE);}
#define ENSURE_SUCCEED(s) {HRESULT hr_ = s; if (FAILED(hr_)) { return hr_; } }

inline bool helper::SelfLogEnabled()
{
	static int confval = -1;
	if (confval == -1)
	{
		wchar_t buf[32];
		if (!::GetEnvironmentVariableW(L"BDLOGENV", buf, _countof(buf)))
		{
			return 0;
		}
		confval = helper::StrToInt(buf) & 1;
	}
	return confval? true : false;
}

inline void helper::InternalLog(const wchar_t* buffer1, const wchar_t* buffer2)
{
	if (!SelfLogEnabled()) return;
	
	wchar_t buffer[1024];
	textstream s(buffer, _countof(buffer));
	s << L"BDLOG:" << buffer1 << buffer2 << L"\n";

	OutputDebugStringW(buffer);
}

inline void helper::InternalLogWinError(const wchar_t* buffer)
{
	if (!SelfLogEnabled()) return;

	DWORD dwError = ::GetLastError();
	InternalLog(buffer, ToStr(dwError, L"%u"));
}


inline void helper::ExpandVariable(const wchar_t* src, wchar_t* dest, size_t destlen)
{
	const wchar_t* p = src;
	wchar_t* e = dest + destlen - 1;
	wchar_t* q = dest;
	wchar_t cvtbuf[4096];

	while (*p)
	{
		if (*p != L'$' || p[1] != L'{')
		{
			*q = *p;
			p++;
			if (q < e) q++;
		}
		else
		{
			p += 2;
			const wchar_t* r = wcschr(p, L'}');
			if (r)
			{
				cvtbuf[0] = L'\0';
				FILETIME t;
				if (StringEqual(p, r, L"PID"))
				{
					helper::IntToStr(static_cast<int>(::GetCurrentProcessId()), cvtbuf);
				}
				else if (StringEqual(p, r, L"T"))
				{
					::GetSystemTimeAsFileTime(&t);
					helper::FileTimeToString(t, L"YmdHMS", cvtbuf, _countof(cvtbuf));
				}
				else if (StringEqual(p, r, L"DATE"))
				{
					::GetSystemTimeAsFileTime(&t);
					helper::FileTimeToString(t, L"Ymd", cvtbuf, _countof(cvtbuf));
				}
				else if (StringEqual(p, r, L"TIME"))
				{
					::GetSystemTimeAsFileTime(&t);
					helper::FileTimeToString(t, L"HMS", cvtbuf, _countof(cvtbuf));
				}

				size_t buflen = wcslen(cvtbuf);
				if (buflen > 0)
				{
					q = SafeCopyString(cvtbuf, buflen, q, static_cast<size_t>(e - q));
				}
				p = r + 1;
			}
		}
	}

	*q = L'\0';
}


inline wchar_t* helper::IntToStr_Padding0(wchar_t* str, size_t strlen, size_t len, int val)
{
	wchar_t* e = str + min(strlen, len);
	wchar_t* p = e - 1;
	for (size_t i = len; i > 0 && p >= str; i--, p--)
	{
		*p = static_cast<wchar_t>(val % 10 + L'0');
		val /= 10;
	}

	return e;
}

inline FILETIME helper::UnixTimeToFileTime(__int64 t)
{
	FILETIME ft;
	t += 3600 * 8;
	t = t * 10000000 + 116444736000000000;
	ft.dwLowDateTime = static_cast<DWORD>(t);
	ft.dwHighDateTime = static_cast<DWORD>(t >> 32);
	return ft;
}

inline void helper::FileTimeToString(FILETIME ft, const wchar_t* fmt, wchar_t* buf, size_t buflen)
{
	SYSTEMTIME st;
	if (!::FileTimeToSystemTime(&ft, &st))
	{
		LOGWINERR(L"时间转换失败:");
	}

	wchar_t* e = buf + buflen - 1;
	for (const wchar_t* p = fmt; *p; p++)
	{
		size_t len = static_cast<size_t>(e - buf);
		switch (*p)
		{
		case 'Y': buf = IntToStr_Padding0(buf, len, 4, st.wYear); break;
		case 'm': buf = IntToStr_Padding0(buf, len, 2, st.wMonth); break;
		case 'd': buf = IntToStr_Padding0(buf, len, 2, st.wDay); break;
		case 'H': buf = IntToStr_Padding0(buf, len, 2, st.wHour); break;
		case 'M': buf = IntToStr_Padding0(buf, len, 2, st.wMinute); break;
		case 'S': buf = IntToStr_Padding0(buf, len, 2, st.wSecond); break;
		default: 
			if (len > 0) *buf++ = *p;
		}
	}

	*buf = L'\0';
}

inline BOOL helper::MakeRequiredDirectory(const wchar_t* p)
{
	wchar_t path[MAX_PATH];
	SafeCopyString(p, wcslen(p), path, _countof(path) - 1);
	path[_countof(path)-1] = L'\0';

	wchar_t* q = path;
	for (;;)
	{
		q = wcschr(q, L'\\');
		if (!q) break;
		*q = L'\0';
		::CreateDirectoryW(path, NULL);
		*q++ = L'\\';
	}

	return TRUE;
}



inline wchar_t* helper::IntToStr(int val, wchar_t* str)
{
	wsprintfW(str, L"%d", val);
	return str;
}

inline wchar_t* helper::IntToHexStr(int val, wchar_t* str)
{
	wsprintfW(str, L"%x", val);
	return str;
}

inline int helper::StrToInt(const wchar_t* str)
{
	int val = 0;
	while (*str >= L'0' && *str <= L'9')
	{
		val = val * 10 + (*str - L'0');
		str++;
	}
	return val;
}
