#include "stdafx.h"
#include "helper.h"
#include "filter.h"
#include "concretefilter.h"
#include "logpropdb.h"

#define SAVE_UNSTABLE_FILTER

namespace 
{

	template <typename T> int value_compare(const T& v1, const T& v2)
	{
		if (v1 < v2) return -1;
		if (v2 < v1) return 1;
		return 0;
	}

	int value_compare(bool v1, bool v2)
	{
		if (v1 == v2) return 0;
		if (!v1) return -1;
		return 1;
	}
	int value_compare(const std::wstring& v1, const std::wstring& v2)
	{
		return v1.compare(v2);
	}

	template <typename A, typename B>
	int value_compare(const A& a1, const A& a2, const B& b1, const B& b2)
	{
		int a = value_compare(a1, a2); if (a) return a;
		int b = value_compare(b1, b2); if (b) return b;
		return 0;
	}

	template <typename A, typename B, typename C>
	int value_compare(const A& a1, const A& a2, const B& b1, const B& b2, const C& c1, const C& c2)
	{
		int a = value_compare(a1, a2); if (a) return a;
		int b = value_compare(b1, b2); if (b) return b;
		int c = value_compare(c1, c2); if (c) return c;
		return 0;
	}
}


std::wstring logclass_filter::name() const
{
	return L"日志级别过滤规则";
}
std::wstring logclass_filter::desc() const
{
	if (m_class_low == 0 && m_class_high == 0)
	{
		return L"所有日志级别";
	}
	else if (m_class_low == m_class_high)
	{
		return L"日志级别为" + quote(helper::GetLogLevelDescription(m_class_high));
	}
	else if (m_class_low == 0)
	{
		return L"日志级别不高于" + quote(helper::GetLogLevelDescription(m_class_high));
	}
	else if (m_class_high == 0)
	{
		return L"日志级别不低于" + quote(helper::GetLogLevelDescription(m_class_low));
	}
	else
	{
		return std::wstring(L"日志级别在")
			+  quote(helper::GetLogLevelDescription(m_class_low))
			+  L"和"
			+  quote(helper::GetLogLevelDescription(m_class_high))
			+  L"之间";
	}
}
component* logclass_filter::clone(bool) const
{
	return new logclass_filter(m_class_low, m_class_high);
}
bool logclass_filter::meet(const LogInfo& log) const
{
	return log.item->log_class >= m_class_low && (m_class_high == 0 || log.item->log_class <= m_class_high);
}

bool logclass_filter::load(component_creator* /*cc*/, serializer* s)
{
	swscanf_s(s->get_property(L"class_low").c_str(), L"%u", &m_class_low);
	swscanf_s(s->get_property(L"class_high").c_str(), L"%u", &m_class_high);
	return true;
}
bool logclass_filter::save(component_creator* /*cc*/, serializer* s) const
{
	wchar_t buf[32];
	serializer* mys = s->add_child(classname());
	swprintf_s(buf, L"%u", m_class_low);
	mys->set_property(L"class_low", buf);

	swprintf_s(buf, L"%u", m_class_high);
	mys->set_property(L"class_high", buf);

	return true;
}

component* logclass_filter::simplify() const
{
	return clone();
}

int logclass_filter::compare(const leaf* l) const
{
	const logclass_filter* rhs = dynamic_cast<const logclass_filter*>(l);
	return value_compare(m_class_low, rhs->m_class_low, m_class_high, rhs->m_class_high);
}

logclass_filter::logclass_filter(UINT low, UINT high) : m_class_high(high), m_class_low(low)
{
}

std::wstring logcontent_filter::name() const
{
	return L"日志内容过滤规则";
}	
std::wstring logcontent_filter::desc() const
{
	std::wstring str = L"日志内容包含" + quote(m_matcher);
	if (m_ignore_case) str += L"(忽略大小写)";
	return str;
}
component* logcontent_filter::clone(bool) const
{
	return new logcontent_filter(m_matcher, m_ignore_case);
}
bool logcontent_filter::meet(const LogInfo& log) const
{
	if (m_ignore_case)
	{
		return helper::wcsistr(log.item->log_content.c_str(), m_matcher.c_str()) != NULL;
	}
	else
	{
		return wcsstr(log.item->log_content.c_str(), m_matcher.c_str()) != NULL;
	}
}

bool logcontent_filter::load(component_creator* /*cc*/, serializer* s)
{
	int cs = 0;
	m_matcher = s->get_property(L"matcher");
	swscanf_s(s->get_property(L"ignorecase").c_str(), L"%d", &cs);
	m_ignore_case = (cs == 1);

	m_matcher_lowercase = lowercase(m_matcher);
	return true;
}
bool logcontent_filter::save(component_creator* /*cc*/, serializer* s) const
{
	wchar_t buf[32];
	serializer* mys = s->add_child(classname());

	mys->set_property(L"matcher", m_matcher.c_str());

	swprintf_s(buf, L"%d", m_ignore_case? 1: 0);
	mys->set_property(L"ignorecase", buf);

	return true;
}

component* logcontent_filter::simplify() const
{
	return clone();
}

int logcontent_filter::compare(const leaf* f) const
{
	const logcontent_filter* rhs = dynamic_cast<const logcontent_filter*>(f);

	return value_compare(m_ignore_case, rhs->m_ignore_case, m_matcher, rhs->m_matcher);
}

void logcontent_filter::setfilter(const std::wstring& matcher, bool cs)
{
	m_matcher = matcher;
	m_ignore_case = cs;
	m_matcher_lowercase = lowercase(matcher);
}

logcontent_filter::logcontent_filter(const std::wstring& matcher, bool cs) : m_matcher(matcher), m_ignore_case(cs)
{
	m_matcher_lowercase = lowercase(matcher);
}


std::wstring logcontent_filter::lowercase(const std::wstring str)
{
	std::wstring ret = str;
	for (size_t i = 0; i < ret.length(); i++)
	{
		ret[i] = static_cast<wchar_t>(tolower(ret[i]));
	}
	return ret;
}

logtag_filter::logtag_filter(const std::wstring& tag)
{
	setfilter(tag);
}
std::wstring logtag_filter::name() const
{
	return L"日志标签过滤规则";
}
std::wstring logtag_filter::desc() const
{
	return std::wstring(L"日志标签匹配") + quote(m_tag); 
}
component* logtag_filter::clone(bool) const
{
	return new logtag_filter(m_tag);
}
bool logtag_filter::meet(const LogInfo &log) const
{
	return helper::TagMatch(log.item->log_tags.c_str(), m_tag.c_str());
}

bool logtag_filter::load(component_creator *, serializer *s)
{
	setfilter(s->get_property(L"tag"));
	return true;
}

bool logtag_filter::save(component_creator *, serializer *s) const
{
	serializer* mys = s->add_child(classname());
	mys->set_property(L"tag", m_tag);
	return true;
}

component* logtag_filter::simplify() const
{
	return clone();
}

void logtag_filter::setfilter(const std::wstring& tag)
{
	m_tag = tag;
	m_tag_matcher = m_tag + L";";
}

int logtag_filter::compare(const leaf* l) const
{
	const logtag_filter* rhs = dynamic_cast<const logtag_filter*>(l);
	return value_compare(m_tag, rhs->m_tag);
}


logpid_filter::logpid_filter(unsigned int pid)
{
	setfilter(pid);
}
std::wstring logpid_filter::name() const
{
	return L"进程ID过滤规则";
}
std::wstring logpid_filter::desc() const
{
	return (const wchar_t *)tp::cz(L"进程ID为%u", m_pid);
}
component* logpid_filter::clone(bool) const
{
	return new logpid_filter(m_pid);
}
bool logpid_filter::meet(const LogInfo &log) const
{
	return log.item->log_pid == m_pid;
}

bool logpid_filter::load(component_creator *, serializer *s)
{
	swscanf_s(s->get_property(L"pid").c_str(), L"%u", &m_pid);
	return true;
}

bool logpid_filter::save(component_creator *, serializer *s) const
{
#ifdef SAVE_UNSTABLE_FILTER
	serializer* mys = s->add_child(classname());
	mys->set_property(L"pid", (const wchar_t*)tp::cz(L"%u", m_pid));
	return true;
#else
	return false;
#endif
}

component* logpid_filter::simplify() const
{
	return clone();
}

void logpid_filter::setfilter(unsigned int pid)
{
	m_pid = pid;
}

int logpid_filter::compare(const leaf* l) const
{
	const logpid_filter* rhs = dynamic_cast<const logpid_filter*>(l);
	return value_compare(m_pid, rhs->m_pid);
}


logtid_filter::logtid_filter(unsigned int tid)
{
	setfilter(tid);
}
std::wstring logtid_filter::name() const
{
	return L"线程ID过滤规则";
}
std::wstring logtid_filter::desc() const
{
	return (const wchar_t *)tp::cz(L"线程ID为%u", m_tid);
}
component* logtid_filter::clone(bool) const
{
	return new logtid_filter(m_tid);
}
bool logtid_filter::meet(const LogInfo &log) const
{
	return log.item->log_tid == m_tid;
}

bool logtid_filter::load(component_creator *, serializer *s)
{
	swscanf_s(s->get_property(L"tid").c_str(), L"%u", &m_tid);
	return true;
}

bool logtid_filter::save(component_creator *, serializer *s) const
{
#ifdef SAVE_UNSTABLE_FILTER
	serializer* mys = s->add_child(classname());
	mys->set_property(L"tid", (const wchar_t*)tp::cz(L"%u", m_tid));
	return true;
#else
	return false;
#endif
}

component* logtid_filter::simplify() const
{
	return clone();
}

void logtid_filter::setfilter(unsigned int tid)
{
	m_tid = tid;
}

int logtid_filter::compare(const leaf* l) const
{
	const logtid_filter* rhs = dynamic_cast<const logtid_filter*>(l);
	return value_compare(m_tid, rhs->m_tid);
}


logprocessname_filter::logprocessname_filter(const std::wstring& process_name)
{
	setfilter(process_name);
}
std::wstring logprocessname_filter::name() const
{
	return L"进程过滤规则";
}
std::wstring logprocessname_filter::desc() const
{
	return std::wstring(L"进程为") + quote(m_process_name); 
}
component* logprocessname_filter::clone(bool) const
{
	return new logprocessname_filter(m_process_name);
}
bool logprocessname_filter::meet(const LogInfo &log) const
{
	return _wcsicmp(log.item->log_process_name.c_str(), m_process_name.c_str()) == 0;
}

bool logprocessname_filter::load(component_creator *, serializer *s)
{
	setfilter(s->get_property(L"process_name"));
	return true;
}

bool logprocessname_filter::save(component_creator *, serializer *s) const
{
	serializer* mys = s->add_child(classname());
	mys->set_property(L"process_name", m_process_name);
	return true;
}

component* logprocessname_filter::simplify() const
{
	return clone();
}

void logprocessname_filter::setfilter(const std::wstring& process_name)
{
	m_process_name = process_name;
}

int logprocessname_filter::compare(const leaf* l) const
{
	const logprocessname_filter* rhs = dynamic_cast<const logprocessname_filter*>(l);
	return value_compare(m_process_name, rhs->m_process_name);
}
