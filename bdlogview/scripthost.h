#pragma once

#include "serviceid.h"
#include <activscp.h>

class CScriptHost : public tp::service_impl<SID_ScriptHost>
{
public:
	CScriptHost();
	~CScriptHost();

	void RunScript(const wchar_t* jscode);
private:
	CComPtr<IActiveScript> m_scriptEngine;
	CComPtr<IActiveScriptParse> m_scriptParser;
	CComPtr<IActiveScriptSite> m_scriptSite;
};

DEFINE_SERVICE(CScriptHost, L"脚本运行支持");
