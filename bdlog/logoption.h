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

