#pragma once

#include "common.h"
#include <stdarg.h>

struct formatstr
{
	explicit formatstr(LPCWSTR fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vswprintf_s(m_buffer, fmt, ap);
		va_end(ap);
	}
	operator LPCWSTR()
	{
		return m_buffer;
	}
private:
	WCHAR m_buffer[1024];
};

struct str_segment
{
	const wchar_t* start;
	const wchar_t* end;
};

struct helper
{
	static void helper::SplitString(const CStringW& str, LPCWSTR pszSpliter, StrSet& ss);
	static CStringW helper::ConcatString(const StrSet& ss, LPCWSTR pszSpliter);
	static CStringW GetModuleFilePath();
	static CStringW GetModuleDir();
	static CStringW GetDir(const CStringW& strPath);
	static CStringW GetFileName(LPCWSTR pszFilePath);
	static CStringW GetErrorDesc(DWORD dwErr);
	static CStringW GetProductName();
	
	static CStringW XML_GetAttributeAsString(IXMLDOMNode* pNode, LPCWSTR pszAttr, LPCWSTR pszDefault);
	static COLORREF XML_GetAttributeAsColor(IXMLDOMNode* pNode, LPCWSTR pszAttr, COLORREF crDefault);
	static UINT XML_GetAttributeAsUint(IXMLDOMNode* pNode, LPCWSTR pszAttr, UINT uDefault);
	static int XML_GetAttributeAsInt(IXMLDOMNode* pNode, LPCWSTR pszAttr, int uDefault);
	static CComPtr<IXMLDOMNode> XML_CreateNode(IXMLDOMDocument* pDoc, int type, LPCWSTR pszName);

	static bool XML_AddAttribute(IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, LPCWSTR pszAttrName, LPCWSTR pszValue);
	static bool XML_AddAttributeUint(IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, LPCWSTR pszAttrName, UINT nVal);
	static bool XML_AddAttributeInt(IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, LPCWSTR pszAttrName, int nVal);
	static bool XML_AddAttributeColor(IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, LPCWSTR pszAttrName, COLORREF cr);


	static size_t str_distance(const std::wstring& source, const std::wstring& target);

	static __time64_t GetExpireTime();
	static CStringW GetHelpText();
	static bool NeedNotifyExpire();
	static HRESULT WriteFileFromResource(LPCWSTR filename, LPCWSTR name, LPCWSTR type);
	static HRESULT WriteFileIfNotExist(LPCWSTR path, LPCWSTR rcname, LPCWSTR rctype);

	static CStringW DecryptXLogFile(LPCWSTR pszFileName);

	static void GUIReportError(LPCWSTR pszError);
	static void GUIReportError(LPCWSTR pszError, HRESULT hr);

	static void RunOffline(LPCWSTR pszFilePath = NULL);

	static CStringW GetVersion();
	static CStringW GetLatestVersion();
	static CStringW UpdateSelf();

	static bool FtpDownloadFile(LPCSTR pszHost, LPCSTR user, LPCSTR pass, LPCSTR remote_path, LPCSTR remote_file, LPCSTR local_file);

	static int parse_num(const wchar_t * start, int count, int radix);

	static const wchar_t* wcsistr(const wchar_t * str, const wchar_t * substr);

	// %appdata%\xxx
	// <CSIDL:COMMON_APP_DATA>\xxx
	// <REG:HKLM\software\xxx>\yyy
	// <FE:d:\symbols|e:\symbols>
	static CStringW ExpandPath(LPCWSTR pszPath);

	static CStringW GetLogLevelDescription(UINT level);

	static void bool_reverse(bool& val);

	static bool TagMatch(const wchar_t* tags, const wchar_t* matcher);
	static void SplitTag(const wchar_t* tags, str_segment (&segs)[256]);

	static bool MakeRequiredPath(LPCWSTR pcszPath);
};

struct GlobalData
{
	HMODULE module;
	CString strXLogFile;

	GlobalData() : module(NULL)
	{

	}
};

extern GlobalData GD;