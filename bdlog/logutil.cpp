#include "stdafx.h"
#include "logutil.h"

static int g_enableLog = -1;

static int GetSelfLogSetting()
{
	wchar_t buf[32];
	if (!::GetEnvironmentVariableW(L"BDLOGENV", buf, _countof(buf)))
	{
		return 0;
	}
	return helper::StrToInt(buf);
}

void InternalLogClass::InternalLog(const wchar_t* buffer1, const wchar_t* buffer2)
{
	if (g_enableLog == 0) return;

	if (g_enableLog == -1)
	{
		g_enableLog = GetSelfLogSetting() & 1;
		if (g_enableLog == 0) return;
	}
	
	wchar_t buffer[1024];
	textstream s(buffer, _countof(buffer));
	s << L"BDLOG:" << buffer1 << buffer2 << L"\n";

	OutputDebugStringW(buffer);
}

void InternalLogClass::InternalLogWinError(const wchar_t* buffer)
{
	if (g_enableLog == 0) return;
	DWORD dwError = ::GetLastError();
	InternalLog(buffer, ToStr(dwError, L"%u"));
}

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

static const size_t ResolveVariable(const wchar_t* vs, const wchar_t* ve, wchar_t (&buf)[4096])
{
	if (StringEqual(vs, ve, L"PID"))
	{
		helper::IntToStr(static_cast<int>(::GetCurrentProcessId()), buf);
		return wcslen(buf);
	}
	else if (StringEqual(vs, ve, L"T"))
	{
		FILETIME t;
		::GetSystemTimeAsFileTime(&t);
		helper::FileTimeToString(t, L"YmdHMS", buf, _countof(buf));
		return wcslen(buf);
	}
	else if (StringEqual(vs, ve, L"DATE"))
	{
		FILETIME t;
		::GetSystemTimeAsFileTime(&t);
		helper::FileTimeToString(t, L"Ymd", buf, _countof(buf));
		return wcslen(buf);
	}
	else if (StringEqual(vs, ve, L"TIME"))
	{
		FILETIME t;
		::GetSystemTimeAsFileTime(&t);
		helper::FileTimeToString(t, L"HMS", buf, _countof(buf));
		return wcslen(buf);
	}

	return 0;
}

void helper::ExpandVariable(const wchar_t* src, wchar_t* dest, size_t destlen)
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
				size_t buflen = ResolveVariable(p, r, cvtbuf);
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


wchar_t* helper::IntToStr_Padding0(wchar_t* str, size_t strlen, size_t len, int val)
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

FILETIME helper::UnixTimeToFileTime(__int64 t)
{
	FILETIME ft;
	t += 3600 * 8;
	t = t * 10000000 + 116444736000000000;
	ft.dwLowDateTime = static_cast<DWORD>(t);
	ft.dwHighDateTime = static_cast<DWORD>(t >> 32);
	return ft;
}

void helper::FileTimeToString(FILETIME ft, const wchar_t* fmt, wchar_t* buf, size_t buflen)
{
	SYSTEMTIME st;
	if (!::FileTimeToSystemTime(&ft, &st))
	{
		LOGWINERR(L"Ê±¼ä×ª»»Ê§°Ü:");
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

BOOL helper::MakeRequiredDirectory(const wchar_t* p)
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

HRESULT helper::GetLastErrorAsHRESULT()
{
	DWORD dwErr = ::GetLastError();
	return HRESULT_FROM_WIN32(dwErr);
}

wchar_t* helper::IntToStr(int val, wchar_t* str)
{
	wsprintfW(str, L"%d", val);
	return str;
}

wchar_t* helper::IntToHexStr(int val, wchar_t* str)
{
	wsprintfW(str, L"%x", val);
	return str;
}

int helper::StrToInt(const wchar_t* str)
{
	int r = 0;
	while (*str)
	{
		r = r * 10 + (*str - L'0');
	}
	return r;
}
