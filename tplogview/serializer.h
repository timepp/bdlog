#pragma once
#include "helper.h"

class msxml_serializer : public serializer
{
public:
	msxml_serializer(IXMLDOMNode* pNode, IXMLDOMDocument* pDoc)
		: m_pNode(pNode), m_pDoc(pDoc)
	{

	}

	virtual std::wstring get_property(const std::wstring& name) const
	{
		return (LPCWSTR)helper::XML_GetAttributeAsString(m_pNode, name.c_str(), L"");
	}
	virtual std::wstring get_name() const
	{
		CComBSTR bstrName;
		m_pNode->get_nodeName(&bstrName);
		return (LPCWSTR)bstrName;
	}
	virtual serializer* get_first_child() const
	{
		CComPtr<IXMLDOMNode> child;
		HRESULT hr = m_pNode->get_firstChild(&child);
		if (FAILED(hr) || !child)
		{
			return NULL;
		}

		return new msxml_serializer(child, m_pDoc);
	}
	virtual serializer* get_next_sibling() const
	{
		CComPtr<IXMLDOMNode> pNode;
		HRESULT hr = m_pNode->get_nextSibling(&pNode);
		if (FAILED(hr) || !pNode)
		{
			return NULL;
		}

		return new msxml_serializer(pNode, m_pDoc);
	}

	virtual bool set_property(const std::wstring& name, const std::wstring& val)
	{
		return helper::XML_AddAttribute(m_pDoc, m_pNode, name.c_str(), val.c_str());
	}
	virtual serializer* add_child(const std::wstring& name)
	{
		CComPtr<IXMLDOMNode> pNode;
		m_pNode->appendChild(helper::XML_CreateNode(m_pDoc, NODE_ELEMENT, name.c_str()), &pNode);

		return new msxml_serializer(pNode, m_pDoc);
	}

private:
	CComPtr<IXMLDOMDocument> m_pDoc;
	CComPtr<IXMLDOMNode> m_pNode;
};