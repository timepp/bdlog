#pragma once


#include "bdlog.h"
#include "logutil.h"

class CLogFilter
{
public:
	virtual bool Meet(const LogItem* rec) const = 0;
	virtual ~CLogFilter() {}
};

class CLevelFilter : public CLogFilter
{
public:
	virtual bool Meet(const LogItem* item) const
	{
		int level = static_cast<int>(item->level);
		if (m_relation < 0) return level < m_val;
		if (m_relation > 0) return level > m_val;
		return level == m_val;
	}

	int m_relation;  // == > <
	int m_val;
};

class CTagFilter : public CLogFilter
{
public:
	virtual bool Meet(const LogItem* item) const
	{
		const wchar_t* p = m_tag;
		const wchar_t* q = item->tag;
		for (;;)
		{
			if (!*p && (!*q || *q == L';')) return true;
			if (*p == *q)
			{
				p++; q++;
			}
			else
			{
				if (!*q) return false;
				p = m_tag;
				while (*q && *q != L';') q++;
				if (!*q) return false;
				q++;
			}
		}
	}

	wchar_t m_tag[16];
};

class CAndFilter : public CLogFilter
{
public:
	virtual bool Meet(const LogItem* item) const
	{	
		if (m_child1 && !m_child1->Meet(item)) return false;
		if (m_child2 && !m_child2->Meet(item)) return false;
		return true;
	}

	CLogFilter* m_child1;
	CLogFilter* m_child2;
};

class COrFilter : public CLogFilter
{
public:
	virtual bool Meet(const LogItem* item) const
	{
		if (m_child1 && m_child1->Meet(item)) return true;
		if (m_child2 && m_child2->Meet(item)) return true;
		return false;
	}

	CLogFilter* m_child1;
	CLogFilter* m_child2;
};

class CNotFilter : public CLogFilter
{
public:
	virtual bool Meet(const LogItem* item) const
	{
		if (m_child && !m_child->Meet(item)) return true;
		return false;
	}

	CLogFilter* m_child;
};


class CLogFilterCreator
{
public:
	CLogFilter* CreateFilter(const wchar_t* filterstr, size_t len)
	{
		if (len == 0) return NULL;

		// 1. &&  or  ||
		int c = 0;
		for (size_t j = 0; j < len; j++)
		{
			size_t i = len - 1 - j;
			if (filterstr[i] == L'(') c++;
			else if (filterstr[i] == L')') c--;
			else if (filterstr[i] == L'&' && i+1 < len && filterstr[i+1] == L'&' && c == 0)
			{
				CLogFilter* f1 = CreateFilter(filterstr, i);
				CLogFilter* f2 = CreateFilter(filterstr + i + 2, len - i - 2);
				if (!f1) return f2;
				if (!f2) return f1;
				CAndFilter* filter = new CAndFilter;
				filter->m_child1 = f1;
				filter->m_child2 = f2;
				return filter;
			}
			else if (filterstr[i] == L'|' && i+1 < len && filterstr[i+1] == L'|' && c == 0)
			{
				CLogFilter* f1 = CreateFilter(filterstr, i);
				CLogFilter* f2 = CreateFilter(filterstr + i + 2, len - i - 2);
				if (!f1) return f2;
				if (!f2) return f1;
				COrFilter* filter = new COrFilter;
				filter->m_child1 = f1;
				filter->m_child2 = f2;
				return filter;
			}
		}

		// 2. spaces
		size_t i = 0, j = 0;
		for (; filterstr[i] == L' ' || filterstr[i] == L'\t'; i++);
		for (; filterstr[len-1-j] == L' ' || filterstr[len-1-j] == L'\t'; j++);
		filterstr += i;
		len -= i + j;
		if (len == 0) return NULL;

		// 3. !(A) or (A)
		if (filterstr[0] == L'(' && filterstr[len-1] == L')')
		{
			return CreateFilter(filterstr + 1, len - 2);
		}
		if (filterstr[0] == L'!' && filterstr[1] == L'(' && filterstr[len-1] == L')')
		{
			CNotFilter* filter = new CNotFilter;
			filter->m_child = CreateFilter(filterstr + 2, len - 3);
			return filter;
		}

		// 4. relation
		size_t rp = 0;
		size_t vp = 0;
		for (size_t i = 0; i < len; i++)
		{
			if (filterstr[i] == L'<' || filterstr[i] == L'>' || filterstr[i] == L'=')
			{
				rp = i;
				size_t j;
				for (j = i+1; j < len && (filterstr[j] == L'=' || filterstr[j] == L' ' || filterstr[j] == L'\t'); j++);
				vp = j;
			}
		}
		if (!vp)
		{
			return NULL;
		}

		if (wcsncmp(filterstr, L"level", 5) == 0)
		{
			CLevelFilter* filter = new CLevelFilter();
			if (filterstr[rp] == L'>') filter->m_relation = 1;
			if (filterstr[rp] == L'<') filter->m_relation = -1;
			if (filterstr[rp] == L'=') filter->m_relation = 0;
			filter->m_val = helper::StrToInt(filterstr + vp);
			return filter;
		}

		if (wcsncmp(filterstr, L"tag", 3) == 0)
		{
			CTagFilter* filter = new CTagFilter;
			TRUNCATED_COPY(filter->m_tag, lstr(filterstr + vp, len - vp));
			return filter;
		}

		return NULL;
	}
};
