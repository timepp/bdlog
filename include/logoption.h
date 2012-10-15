#pragma once

#include <bdlog.h>


class CLogOption : public ILogOption
{
public:
	virtual const wchar_t* GetOption(const wchar_t* key, const wchar_t* defaultValue);
	virtual int GetOptionAsInt(const wchar_t* key, int defaultValue);
	virtual bool GetOptionAsBool(const wchar_t* key, bool defaultValue);

	CLogOption();
	void SetOptionString(const wchar_t* optstr);
	void Append(const CLogOption& opt);

private:
	wchar_t m_optstr[1024];

	struct Field
	{
		const wchar_t* key;
		const wchar_t* val;
	};
	Field m_fields[256];
	size_t m_fieldLen;

	void AppendField(const wchar_t* key, const wchar_t* val);
};

inline CLogOption::CLogOption() : m_fieldLen(0)
{
}

inline const wchar_t* CLogOption::GetOption(const wchar_t* key, const wchar_t* defaultValue)
{
	for (size_t i = 0; i < m_fieldLen; i++)
	{
		if (wcscmp(m_fields[i].key, key) == 0) 
		{
			return m_fields[i].val;
		}
	}

	return defaultValue;
}

inline int CLogOption::GetOptionAsInt(const wchar_t* key, int defaultValue)
{
	const wchar_t* val = GetOption(key, NULL);
	if (!val)
	{
		return defaultValue;
	}

	return helper::StrToInt(val);
}

inline bool CLogOption::GetOptionAsBool(const wchar_t* key, bool defaultValue)
{
	const wchar_t* val = GetOption(key, NULL);
	if (!val)
	{
		return defaultValue;
	}

	if (val[0] == L'1' || wcscmp(val, L"true") == 0)
	{
		return true;
	}

	return false;
}

inline void CLogOption::AppendField(const wchar_t* key, const wchar_t* val)
{
	if (m_fieldLen < _countof(m_fields))
	{
		Field f = {key, val};
		m_fields[m_fieldLen] = f;
		m_fieldLen++;
	}
}

inline void CLogOption::Append(const CLogOption& opt)
{
	for (size_t i = 0; i < opt.m_fieldLen; i++)
	{
		AppendField(opt.m_fields[i].key, opt.m_fields[i].val);
	}
}

inline void CLogOption::SetOptionString(const wchar_t* optstr)
{
	m_fieldLen = 0;
	TRUNCATED_COPY(m_optstr, optstr);

	enum {sBlank, sKey, sVal} state = sKey;

	Field f = {m_optstr, NULL};
	wchar_t quote = L'\0';
	wchar_t* optend = m_optstr + wcslen(m_optstr);
	for (wchar_t* p = m_optstr; p <= optend; p++)
	{
		switch (state)
		{
		case sBlank:
			if (*p != L' ' && *p != L'\t')
			{
				state = sKey;
				f.key = p;
			}
			break;
		case sKey: 
			if (*p == L':')
			{
				*p = L'\0';
				if (p[1] == L'\"' || p[1] == L'\'')
				{
					quote = p[1];
					p++;
				}
				else
				{
					quote = L'\0';
				}
				state = sVal;
				f.val = p + 1;
			}
			break;
		case sVal:
			if (*p == quote ||
				wcschr(L" \t", *p) != NULL && quote == L'\0')
			{
				*p = L'\0';
				AppendField(f.key, f.val);
				f.key = NULL;
				f.val = NULL;
				state = sBlank;
			}
			break;
		}
	}
}
