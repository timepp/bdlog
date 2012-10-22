#pragma once

#include <tplib/include/composite.h>
#include "concretefiltereditdlg.h"
#include <vector>

class filter_creator : public component_creator
{
	struct component_prototype
	{
		component* c;
		CComponentConfigCommand* cmd;
		component_prototype(component* cc, CComponentConfigCommand* ccm) : c(cc), cmd(ccm)
		{

		}
	};
	typedef std::vector<component_prototype> prototypes_t;

public:
	static filter_creator* instance();

	virtual component* create(const std::wstring& classname);
	virtual const component* get_prototype(const std::wstring& classname) const;
	virtual bool configure(component* c);

	~filter_creator();

	filter* load(LPCWSTR pszFileName);
	bool save(filter* f, LPCWSTR pszFileName);
	filter* load(IXMLDOMNode* pNode);
	bool save(filter* f, IXMLDOMDocument* pDoc, IXMLDOMNode* pNode);

private:
	filter_creator();
	static filter_creator m_globalobj;

	prototypes_t m_pts;
};

