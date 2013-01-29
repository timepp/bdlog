#pragma once

/// pase js code into syntax segments

#include <vector>
#include <map>

class jsparser
{
public:
	// textrange: [begin, end)
	struct textrange
	{
		int begin;
		int end;
	};
	enum codetype
	{
		ctBlockComment,
		ctLineComment,
		ctKeywordLoop, // do while for continue break
		ctKeywordFlow, // switch case default return if else throw try catch finally
		ctKeywordDecl, // var function 
		ctKeywordBuiltin, // new delete in instanceof typeof with
		ctKeywordConst, // null true false this
		ctFunctionArgument,
		ctSeprator,
		ctStringLiteral,
		ctStringEscape,
		ctNumberLiteral,
		ctNormal,
		ctWhiteSpace,
		ctMax,
	};
	
	struct segment
	{
		textrange range;
		int type;

		bool operator<(const segment& s) const
		{
			if (range.begin < s.range.begin) return true;
			if (range.begin > s.range.begin) return false;
			if (range.end < s.range.end) return true;
			if (range.end > s.range.end) return false;
			return type < s.type;
		}
	};

	typedef std::vector<segment> segments_t;

	bool parse(const wchar_t* code, segments_t& segs);

	// t必须大于ctMax
	bool add_user_keyword(const wchar_t* name, int t);
	bool add_user_keyword(const wchar_t** name, size_t len, int t);

private:
	typedef std::map<std::wstring, int> typemap_t;
	typemap_t m_utm;
	std::wstring m_str;

	static const typemap_t& system_type_map()
	{
#pragma warning(push)
#pragma warning(disable: 4640)
		static typemap_t stm;
#pragma warning(pop)
		if (stm.empty())
		{
#define ADD_KEYWORD(t, ...) { const wchar_t* names[] = {__VA_ARGS__}; add_name_batch(stm, names, _countof(names), t); }
			ADD_KEYWORD(ctKeywordLoop,      L"do", L"while", L"for", L"continue", L"break");
			ADD_KEYWORD(ctKeywordFlow,      L"if", L"else", L"switch", L"case", L"default", L"return", L"try", L"catch", L"finally");
			ADD_KEYWORD(ctKeywordDecl,      L"var", L"function");
			ADD_KEYWORD(ctKeywordBuiltin,   L"new", L"delete", L"in", L"instanceof", L"typeof", L"with");
			ADD_KEYWORD(ctKeywordConst,     L"null", L"true", L"false", L"this");
#undef ADD_KEYWORD
		}
		return stm;
	}

	static bool is_id_char(wchar_t ch)
	{
		if (ch >= 'A' && ch <= 'Z') return true;
		if (ch >= 'a' && ch <= 'z') return true;
		if (ch == '_') return true;
		return false;
	}
	static bool is_num(wchar_t ch)
	{
		return ch >= L'0' && ch <= L'9';
	}
	static bool is_hex_num(wchar_t ch)
	{
		return is_num(ch) || (ch >= L'A' && ch <= L'F') || (ch >= L'a' && ch <= L'f');
	}

	static const wchar_t* lookup_number(const wchar_t* p)
	{
		// 1. 0X/0x
		if (p[0] == L'0' && (p[1] == L'X' || p[1] == L'x'))
		{
			p += 2;
			while (is_hex_num(*p)) p++;
			return p;
		}

		// 2. [digits][.digits][(E|e)[(+|-)]digits]
		while (is_num(*p)) p++;
		if (*p == L'.')
		{
			if (!is_num(p[1])) return p;
			p++;
			while (is_num(*p)) p++;
		}
		if (*p == L'E' || *p == L'e')
		{
			const wchar_t* q = p;
			p++;
			if (*p == L'+' || *p == L'-') p++;
			if (!is_num(*p)) return q;
			while (is_num(*p)) p++;
		}
		return p;
	}

	static const wchar_t* lookup_terminate(const wchar_t* p, wchar_t ch)
	{
		while (*p)
		{
			if (p[0] == ch)
			{
				p++;
				break;
			}
			p++;
		}
		return p;
	}

	static const wchar_t* lookup_terminate(const wchar_t* p, wchar_t ch1, wchar_t ch2)
	{
		while (*p)
		{
			if (p[0] == ch1 && p[1] == ch2)
			{
				p += 2;
				break;
			}
			p++;
		}
		return p;
	}

	static void add_name_batch(typemap_t& tm, const wchar_t** names, size_t len, int t)
	{
		for (size_t i = 0; i < len; i++) tm[names[i]] = t;
	}

	void add_segment(segments_t& segs, const wchar_t* code, const wchar_t* start, const wchar_t* end, int t = -1)
	{
		if (start == end) return;

		segment s;
		if (t == -1)
		{
			s.type = ctNormal;
			m_str.assign(start, end);
			const typemap_t& m1 = system_type_map();
			typemap_t::const_iterator it = m1.find(m_str);
			if (it != m1.end())
			{
				s.type = it->second;
			}

			const typemap_t& m2 = m_utm;
			it = m2.find(m_str);
			if (it != m2.end())
			{
				s.type = it->second;
			}
		}
		else
		{
			s.type = t;
		}

		s.range.begin = start - code;
		s.range.end = end - code;
		segs.push_back(s);
	}
};

inline bool jsparser::add_user_keyword(const wchar_t* name, int t)
{
	if (t <= ctMax) return false;
	m_utm[name] = t;
	return true;
}

inline bool jsparser::add_user_keyword(const wchar_t** name, size_t len, int t)
{
	if (t <= ctMax) return false;
	for (size_t i = 0; i < len; i++)
	{
		add_user_keyword(name[i], t);
	}
	return true;
}

inline bool jsparser::parse(const wchar_t* code, segments_t& segs)
{
	segs.clear();

	// 当前的scope
	// enum {scopeNormal, scopeBlockComment, scopeLineComment, scopeFunctionParamDecl, scopeFunctionBody} scope = scopeNormal;
	const wchar_t* start = code;
	const wchar_t* p = code;
	for (;;)
	{
		wchar_t ch = *p;
		if (is_id_char(ch)) {p++; continue;}
		if (start < p && ch >= L'0' && ch <= L'9') {p++; continue;}

		add_segment(segs, code, start, p);
		start = p;

		if (!*p) break;
		if (p[0] == L'/' && p[1] == L'*')
		{
			p = lookup_terminate(p, L'*', L'/');
			add_segment(segs, code, start, p, ctBlockComment);
			start = p;
		}
		else if (p[0] == L'/' && p[1] == L'/')
		{
			p = lookup_terminate(p, L'\n');
			add_segment(segs, code, start, p, ctLineComment);
			start = p;
		}
		else if (ch == L'\"' || ch == L'\'')
		{
			// TODO 解析字符串中的转义符
			const wchar_t* q = p+1;
			for (; *q; q++)
			{
				if (q[0] == L'\\' && q[1]) q++;
				if (q[0] == ch) {q++; break;}
			}
			p = q;
			add_segment(segs, code, start, p, ctStringLiteral);
			start = p;
		}
		else if (wcschr(L" \t\r\n", ch))
		{
			p++;
			add_segment(segs, code, start, p, ctWhiteSpace);
			start = p;
		}
		else if (ch >= L'0' && ch <= L'9')
		{
			// TODO
			p = lookup_number(p);
			add_segment(segs, code, start, p, ctNumberLiteral);
			start = p;
		}
		else
		{
			p++;
			add_segment(segs, code, start, p, ctSeprator);
			start = p;
		}
	}

	return true;
}
