#include "stdafx.h"
#include "filtercreator.h"
#include "helper.h"
#include "serializer.h"

filter_creator filter_creator::m_globalobj;

filter_creator::filter_creator()
{
	m_pts.push_back(component_prototype(new logical_and_filter, NULL));
	m_pts.push_back(component_prototype(new logical_or_filter, NULL));
	m_pts.push_back(component_prototype(new logical_not_filter, NULL));
	m_pts.push_back(component_prototype(new logclass_filter(0, 0), new CLogClassFilterEditDlg));
	m_pts.push_back(component_prototype(new logcontent_filter(L"开始", true, false), new CLogContentFilterEditDlg));
	m_pts.push_back(component_prototype(new logtag_filter(L""), new CLogTagFilterEditDlg));
	m_pts.push_back(component_prototype(new logprocessname_filter(L""), new CLogProcessNameFilterEditDlg));
	m_pts.push_back(component_prototype(new logpid_filter(0), new CLogPIDFilterEditDlg));
	m_pts.push_back(component_prototype(new logtid_filter(0), new CLogTIDFilterEditDlg));
}

filter_creator* filter_creator::instance()
{
	return &m_globalobj;
}

component* filter_creator::create( const std::wstring& classname )
{
	for (prototypes_t::const_iterator it = m_pts.begin(); it != m_pts.end(); ++it)
	{
		if (it->c->classname() == classname)
		{
			return it->c->clone();
		}
	}
	return NULL;
}

const component* filter_creator::get_prototype(const std::wstring& classname) const
{
	for (prototypes_t::const_iterator it = m_pts.begin(); it != m_pts.end(); ++it)
	{
		if (it->c->classname() == classname)
		{
			return it->c;
		}
	}
	return NULL;
}

bool filter_creator::configure( component* c )
{
	for (prototypes_t::const_iterator it = m_pts.begin(); it != m_pts.end(); ++it)
	{
		if (it->c->classname() == c->classname())
		{
			if (it->cmd)
			{
				return it->cmd->Do(c);
			}

			// 没有配置函数，视为配置成功
			return true;
		}
	}
	return false;
}

filter_creator::~filter_creator()
{
	for (prototypes_t::const_iterator it = m_pts.begin(); it != m_pts.end(); ++it)
	{
		delete it->c;
		delete it->cmd;
	}
	m_pts.clear();
}

filter* filter_creator::load(IXMLDOMNode* pNode)
{
	CComBSTR bstrName;
	pNode->get_nodeName(&bstrName);
	component* c = create((LPCWSTR)bstrName);
	msxml_serializer ms(pNode, NULL);
	c->load(this, &ms);
	return dynamic_cast<filter*>(c);
}

bool filter_creator::save(filter* f, IXMLDOMDocument* pDoc, IXMLDOMNode* pNode)
{
	component* c = dynamic_cast<component*>(f);
	if (c == NULL)
	{
		return false;
	}

	msxml_serializer ms(pNode, pDoc);
	return c->save(this, &ms);
}

filter* filter_creator::load(LPCWSTR pszFileName)
{
	CComPtr<IXMLDOMDocument> pDoc;
	VARIANT_BOOL ret;
	CComPtr<IXMLDOMNode> pNode;
	CComPtr<IXMLDOMNode> pFilterNode;
	HRESULT hr;

	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pDoc);
	if (FAILED(hr)) return NULL;

	hr = pDoc->load(CComVariant(pszFileName), &ret);
	if (FAILED(hr) || !pDoc) return NULL;

	hr = pDoc->get_firstChild(&pNode);
	if (FAILED(hr) || !pNode) return NULL;

	hr = pNode->get_firstChild(&pFilterNode);
	if (FAILED(hr) || !pFilterNode) return NULL;
	
	return load(pFilterNode);
}

bool filter_creator::save(filter* f, LPCWSTR pszFileName)
{
	HRESULT hr;
	CComPtr<IXMLDOMDocument> pDoc;
	CComPtr<IXMLDOMNode> pRootNode;
	CComPtr<IXMLDOMNode> pFilterNode;

	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pDoc);
	hr = pDoc->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"tplogfilter"), &pRootNode);

	save(f, pDoc, pRootNode);
	hr = pDoc->save(CComVariant(pszFileName));

	return true;
}
