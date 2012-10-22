#pragma once

#include <tplib/include/composite.h>
#include "common.h"

/// helper
static std::wstring quote(const wchar_t* str, const std::wstring border = L"\'")
{
	return border + str + border;
}
static std::wstring quote(const std::wstring& str, const std::wstring& border = L"\'")
{
	return border + str + border;
}

#define DEFINE_CLASS_NAME(x) virtual std::wstring classname() const { return WIDESTRING(#x); }

class filter
{
public:
	virtual bool meet(const LogInfo& log) const = 0;
	virtual ~filter(){}
};

class logical_and_filter : public filter, public composite
{
	DEFINE_CLASS_NAME(logical_and_filter);

public:
	virtual std::wstring name() const
	{
		return L"逻辑与";
	}
	virtual std::wstring desc() const
	{
		std::wstring str;
		size_t n = child_count();
		if (n == 1)
		{
			str = get_child(0)->desc();
		}
		else
		{
			for (size_t i = 0; i < n; i++)
			{
				component* child = get_child(i);
				std::wstring child_desc = child->desc();
				if (child_desc.empty()) continue;
				if (!str.empty()) str += L" 并且 ";
				if (child->child_count() > 1) str += L"( ";
				str += child_desc;
				if (child->child_count() > 1) str += L" )";
			}
		}

		return str;
	}
	virtual component* clone(bool deep = true) const
	{
		logical_and_filter* f = new logical_and_filter;
		if (deep) clone_childs(&f->m_childs);
		return f;
	}
	virtual bool meet(const LogInfo& log) const
	{
		size_t n = child_count();
		for (size_t i = 0; i < n; i++)
		{
			filter* f = dynamic_cast<filter*>(get_child(i));
			if (!f->meet(log)) return false;
		}
		return true;
	}
	virtual component* simplify() const
	{
		composite* c = dynamic_cast<composite*>(clone(false));

		// child first
		for (size_t i = 0; i < child_count(); i++)
		{
			component* child = get_child(i)->simplify();
			if (child) c->add_child(child);
		}

		size_t n = c->child_count();

		//
		if (n == 0)
		{
			delete c;
			return NULL;
		}
		// pass direct
		if (n == 1)
		{
			component* ret = c->get_child(0)->clone();
			delete c;
			return ret;
		}

		// merge
		for (size_t i = 0; i < n;)
		{
			component* child = c->get_child(i);
			if (child->classname() == c->classname())
			{
				for (size_t j = 0; j < child->child_count(); j++)
				{
					c->add_child(child->get_child(j)->clone());
				}
				c->del_child(i);
				n--;
			}
			else
			{
				i++;
			}
		}
		
		// sort&uniq
		c->sort();
		c->uniq();

		// 
		if (compare(c) != 0)
		{
			component* ret = c->simplify();
			delete c;
			return ret;
		}

		return c;
	}
};

class logical_or_filter : public filter, public composite
{
	DEFINE_CLASS_NAME(logical_or_filter);

public:
	virtual std::wstring name() const
	{
		return L"逻辑或";
	}
	virtual std::wstring desc() const
	{
		std::wstring str;
		size_t n = child_count();
		if (n == 1)
		{
			str = get_child(0)->desc();
		}
		else
		{
			for (size_t i = 0; i < n; i++)
			{
				component* child = get_child(i);
				std::wstring child_desc = child->desc();
				if (child_desc.empty()) continue;
				if (!str.empty()) str += L" 或者 ";
				if (child->child_count() > 1) str += L"( ";
				str += child_desc;
				if (child->child_count() > 1) str += L" )";
			}
		}

		return str;
	}
	virtual component* clone(bool deep = true) const
	{
		logical_or_filter* f = new logical_or_filter;
		if (deep) clone_childs(&f->m_childs);
		return f;
	}
	virtual bool meet(const LogInfo& log) const
	{
		size_t n = child_count();
		for (size_t i = 0; i < n; i++)
		{
			filter* f = dynamic_cast<filter*>(get_child(i));
			if (f->meet(log)) return true;
		}
		return false;
	}
	virtual component* simplify() const
	{
		composite* c = dynamic_cast<composite*>(clone(false));

		// child first
		for (size_t i = 0; i < child_count(); i++)
		{
			component* child = get_child(i)->simplify();
			if (child) c->add_child(child);
		}

		size_t n = c->child_count();

		//
		if (n == 0)
		{
			delete c;
			return NULL;
		}
		// pass direct
		if (n == 1)
		{
			component* ret = c->get_child(0)->clone();
			delete c;
			return ret;
		}

		// merge
		for (size_t i = 0; i < n;)
		{
			component* child = c->get_child(i);
			if (child->classname() == c->classname())
			{
				for (size_t j = 0; j < child->child_count(); j++)
				{
					c->add_child(child->get_child(j)->clone());
				}
				c->del_child(i);
				n--;
			}
			else
			{
				i++;
			}
		}

		// sort&uniq
		c->sort();
		c->uniq();

		// 
		if (compare(c) != 0)
		{
			component* ret = c->simplify();
			delete c;
			return ret;
		}

		return c;
	}
};

class logical_not_filter : public filter, public composite
{
	DEFINE_CLASS_NAME(logical_not_filter);

public:
	virtual std::wstring name() const
	{
		return L"逻辑非";
	}
	virtual std::wstring desc() const
	{
		std::wstring str;
		size_t n = child_count();
		if (n == 1)
		{
			component* child = get_child(0);
			std::wstring child_desc = child->desc();
			if (!child_desc.empty())
			{
				str += L"非_";
				if (child->child_count() > 1) str += L"( ";
				str += child_desc;
				if (child->child_count() > 1) str += L" )";
			}
		}
		return str;
	}
	virtual component* clone(bool deep = true) const
	{
		logical_not_filter* f = new logical_not_filter;
		if (deep) clone_childs(&f->m_childs);
		return f;
	}
	virtual bool meet(const LogInfo& log) const
	{
		if (child_count() == 1)
		{
			filter* f = dynamic_cast<filter*>(get_child(0));
			if (f->meet(log)) return false;
		}
		return true;
	}
	virtual bool can_add_child(component* /*child*/) const
	{
		return child_count() == 0;
	}

	virtual bool equal_to_child() const
	{
		return false;
	}
};
