#pragma once

#include "tplog.h"

#ifndef TPLOG_STR_W
#define TPLOG_STR_W_2(str) L##str
#define TPLOG_STR_W(str) TPLOG_STR_W_2(str)
#endif

#define TPLOG_CONCAT_INNER(a,b) a##b
#define TPLOG_CONCAT(a,b) TPLOG_CONCAT_INNER(a,b)
#define TPLOG_NAME(prefix) TPLOG_CONCAT(prefix,__LINE__)

namespace tplog
{
	struct FunctionLogger
	{
		explicit FunctionLogger(const wchar_t* funcname, const LogTag& tag, const wchar_t* param = L"") 
			: m_funcname(funcname), m_param(param), m_tag(tag.tag)
		{
			Log(LL_DIAGNOSE, m_tag, L"%s(%s) {", m_funcname, m_param);
			GetLogController()->IncreaseCallDepth();
		}
		~FunctionLogger()
		{
			GetLogController()->DecreaseCallDepth();
			Log(LL_DIAGNOSE, m_tag, L"} %s(%s)", m_funcname, m_param);
		}

		const wchar_t* m_funcname;
		const wchar_t* m_param;
		LogTag m_tag;
	};

	struct fmter
	{
		fmter(int) : m_buffer_len(0)
		{
		}

		template <typename T1>
		fmter(int, const T1& v1) : m_buffer_len(0)
		{
			*this << v1;
		}

		template <typename T1, typename T2>
		fmter(int, const T1& v1, const T2& v2) : m_buffer_len(0)
		{
			*this << v1 << v2;
		}

		template <typename T1, typename T2, typename T3>
		fmter(int, const T1& v1, const T2& v2, const T3& v3) : m_buffer_len(0)
		{
			*this << v1 << v2 << v3;
		}

		template <typename T1, typename T2, typename T3, typename T4>
		fmter(int, const T1& v1, const T2& v2, const T3& v3, const T4& v4) : m_buffer_len(0)
		{
			*this << v1 << v2 << v3 << v4;
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5>
		fmter(int, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) : m_buffer_len(0)
		{
			*this << v1 << v2 << v3 << v4 << v5;
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		fmter(int, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) : m_buffer_len(0)
		{
			*this << v1 << v2 << v3 << v4 << v5 << v6;
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
		fmter(int, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) : m_buffer_len(0)
		{
			*this << v1 << v2 << v3 << v4 << v5 << v6 << v7;
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
		fmter(int, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) : m_buffer_len(0)
		{
			*this << v1 << v2 << v3 << v4 << v5 << v6 << v7 << v8;
		}

		operator const wchar_t* ()
		{
			m_buffer[m_buffer_len] = L'\0';
			return m_buffer;
		}

		fmter& operator<<(bool val)
		{
			appendparam(val? L"true" : L"false");
			return *this;
		}
		fmter& operator<<(long val)
		{
			wchar_t buf[32];
			_ltow_s(val, buf, 10);
			appendparam(buf);
			return *this;
		}
		fmter& operator<<(unsigned long val)
		{
			wchar_t buf[32];
			_ultow_s(val, buf, 10);
			appendparam(buf);
			return *this;
		}
		fmter& operator<<(long long val)
		{
			wchar_t buf[32];
			_i64tow_s(val, buf, _countof(buf), 10);
			appendparam(buf);
			return *this;
		}
		fmter& operator<<(unsigned long long val)
		{
			wchar_t buf[32];
			_ui64tow_s(val, buf, _countof(buf), 10);
			appendparam(buf);
			return *this;
		}

		fmter& operator<<(int val)                     { return operator<<(static_cast<long>(val)); }
		fmter& operator<<(unsigned int val)            { return operator<<(static_cast<unsigned long>(val)); }
		fmter& operator<<(short val)                   { return operator<<(static_cast<long>(val)); }
		fmter& operator<<(unsigned short val)          { return operator<<(static_cast<unsigned long>(val)); }

		fmter& operator<<(const void* val)
		{
			wchar_t buf[32];
			_snwprintf_s(buf, _TRUNCATE, L"0X%p", val);
			appendparam(buf);
			return *this;
		}
		fmter& operator<<(const wchar_t* val)
		{
			if (!val)
			{
				appendparam(L"(null)");
			}
			else
			{
				appendparam(val);
			}
			return *this;
		}
		fmter& operator<<(const char* val)
		{
			if (!val) 
			{
				appendparam(L"(null)");
			}
			else
			{
				wchar_t buf[1024];
				mbstowcs_s(NULL, buf, val, _TRUNCATE);
				appendparam(buf);
			}
			return *this;
		}

		void appendparam(const wchar_t* val)
		{
			if (m_buffer_len > 0)
			{
				append(L", ");
			}
			append(val);
		}

		void append(const wchar_t* val)
		{
			while (*val && m_buffer_len < _countof(m_buffer) - 1)
			{
				m_buffer[m_buffer_len++] = *val++;
			}
			m_buffer[m_buffer_len] = L'\0';
		}
		void append(const wchar_t* val, size_t len)
		{
			const wchar_t* ve = val + len;
			while (val < ve && m_buffer_len < _countof(m_buffer) - 1)
			{
				m_buffer[m_buffer_len++] = *val++;
			}
			m_buffer[m_buffer_len] = L'\0';
		}

		wchar_t m_buffer[2048];
		size_t m_buffer_len;
	};

	struct helper
	{
		static HRESULT RecordError(const wchar_t* funcname, int line, HRESULT hr, const wchar_t* msg = NULL)
		{
			DWORD lasterror = ::GetLastError();
			if (!msg)
			{
				Log(LL_ERROR, TAG(L"assert"), L"error @ %s:%d (lasterror=%u, hr=0x%X)", funcname, line, lasterror, hr);
			}
			else
			{
				Log(LL_ERROR, TAG(L"assert"), L"error @ %s:%d (lasterror=%u, hr=0x%X): %s", funcname, line, lasterror, hr, msg);
			}
			//_ASSERT_EXPR(false, funcname);
			if (hr == S_OK)
			{
				hr = HRESULT_FROM_WIN32(lasterror);
			}
			::SetLastError(lasterror);
			return hr;
		}
	};
}

#if defined(TPLOG_DISABLE_ALL)
#define TPLOG_DISABLE_FUNCTION
#endif

#if defined(TPLOG_DISABLE_FUNCTION)
#define LOG_FUNCTION(...)
#else
#define LOG_FUNCTION(...) \
	tplog::fmter bda_f_(1, __VA_ARGS__); \
	tplog::FunctionLogger TPLOG_NAME(fl_)(TPLOG_STR_W(__FUNCTION__), TAG(L"function"), bda_f_)
#endif

#define TPLOG_SAFESTR(str) (str?str:L"<NULL>")

/************************************************************************/
/*                                                                      */
/************************************************************************/

#define TPLOG_RECORD_ERROR(hr)                tplog::helper::RecordError(TPLOG_STR_W(__FUNCTION__), __LINE__, hr)
#define TPLOG_RECORD_ERROR_WITH_MSG(hr, msg)  tplog::helper::RecordError(TPLOG_STR_W(__FUNCTION__), __LINE__, hr, msg)

/** 
 *   TP_CHECK(ret == 0, IGNORE_FAIL)
 *   TP_CHECK(!m_inited, RETURN_ON_FAIL)
 *   TP_CHECK(hFile != INVALID_HANDLE_VALUE, RETURN_LASTERROR_ON_FAIL)
 */
#define TP_CHECK(expr, operation)                                             \
	do {                                                                      \
		if (!!(expr) == false) {                                              \
			HRESULT tpa_hr_ = TPLOG_RECORD_ERROR(S_OK);                       \
			(tpa_hr_);                                                        \
			operation;                                                        \
		}                                                                     \
	} while (0)

#define TP_CHECK_WITH_MSG(expr, operation, msg)                               \
	do {                                                                      \
		if (!!(expr) == false) {                                              \
			HRESULT tpa_hr_ = TPLOG_RECORD_ERROR_WITH_MSG(S_OK, msg);         \
			(tpa_hr_);                                                        \
			operation;                                                        \
		}                                                                     \
	} while (0)

/** 
 *   TP_CHECK_HR(hr, RETURN_HR_ON_FAIL)
 *   TP_CHECK_HR(hr, IGNORE_FAIL)
 *   TP_CHECK_HR(hr, RETURN_ON_FAIL)
 */
#define TP_CHECK_HR(expr, operation)                                          \
	do {                                                                      \
		HRESULT tpa_hr_ = (expr);                                             \
		if (FAILED(tpa_hr_)) {                                                \
			TPLOG_RECORD_ERROR(tpa_hr_);                                      \
			operation;                                                        \
		}                                                                     \
	} while (0)

#define TP_CHECK_HR_WITH_MSG(expr, operation, msg)                            \
	do {                                                                      \
		HRESULT tpa_hr_ = (expr);                                             \
		if (FAILED(tpa_hr_)) {                                                \
			TPLOG_RECORD_ERROR_WITH_MSG(tpa_hr_, msg);                        \
			operation;                                                        \
		}                                                                     \
	} while (0)

/// 失败时打印日志（DEBUG下弹ASSERT窗口），程序流程继续执行
#define IGNORE_FAIL 

/// 失败时打印日志（DEBUG下弹ASSERT窗口），函数返回
#define RETURN_ON_FAIL return

/// 失败时打印日志（DEBUG下弹ASSERT窗口），函数返回
#define RETURN_VAL_ON_FAIL(val) return val;

/// 失败时打印日志（DEBUG下弹ASSERT窗口），函数返回，把出错的HRESULT值传递给上层调用
#define RETURN_HR_ON_FAIL return tpa_hr_;


#define TP_ASSERT(expr) TP_CHECK(expr, IGNORE_FAIL)
#define TP_CHECK_RETURN_HR_ON_FAIL(expr) TP_CHECK(expr, RETURN_HR_ON_FAIL)
#define TP_CHECK_HR_PASS_HR_ON_FAIL(expr) TP_CHECK_HR(expr, RETURN_HR_ON_FAIL)
#define TP_CHECK_RETURN_VAL_ON_FAIL(expr, val) TP_CHECK(expr, RETURN_VAL_ON_FAIL(val))
#define TP_CHECK_HR_RETURN_VAL_ON_FAIL(expr, val) TP_CHECK_HR(expr, RETURN_VAL_ON_FAIL(val))
#define TP_CHECK_PARAM(expr) TP_CHECK(expr, RETURN_VAL_ON_FAIL(E_INVALIDARG))

