#pragma once


#include <bdlog.h>
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
	virtual bool Meet(const LogItem* item) const;

	int m_relation;  // == > <
	int m_val;
};

class CTagFilter : public CLogFilter
{
public:
	virtual bool Meet(const LogItem* item) const;

	wchar_t m_tag[16];
};

class CAndFilter : public CLogFilter
{
public:
	virtual bool Meet(const LogItem* rec) const;

	CLogFilter* m_child1;
	CLogFilter* m_child2;
};

class COrFilter : public CLogFilter
{
public:
	virtual bool Meet(const LogItem* rec) const;

	CLogFilter* m_child1;
	CLogFilter* m_child2;
};

class CNotFilter : public CLogFilter
{
public:
	virtual bool Meet(const LogItem* item) const;

	CLogFilter* m_child;
};


class CLogFilterCreator
{
public:
	CLogFilter* CreateFilter(const wchar_t* filterstr, size_t len);
};
