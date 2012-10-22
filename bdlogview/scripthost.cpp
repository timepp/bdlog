#include "stdafx.h"
#include "scripthost.h"
#include "logcenter.h"

#include "bdlogview_h.h"

const GUID CLSID_VBScript = {0xb54f3741, 0x5b07, 0x11cf, {0xa4, 0xb0, 0x00, 0xaa, 0x00, 0x4a, 0x55, 0xe8}};
const GUID CLSID_JScript  = {0xf414c260, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};

DECLARE_LIBID(LIBID_BdLogviewLib);

class ATL_NO_VTABLE CCoLogCenter
	: public CComObjectRootEx<CComSingleThreadModel>
	, public CComCoClass<CCoLogCenter, &CLSID_LogCenter>
	, public IDispatchImpl<ILogCenter, &IID_ILogCenter, &LIBID_BdLogviewLib, -1 ,-1>
{
public:
	virtual ~CCoLogCenter() {}

	BEGIN_COM_MAP(CCoLogCenter)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ILogCenter)
	END_COM_MAP()

	STDMETHOD(get_LogCount)(long* pCount)
	{
		LogRange r = SERVICE(CLogCenter)->GetLogRange();
		*pCount = r.idmax - r.idmin + 1;
		return S_OK;
	}
};


class ATL_NO_VTABLE CCoBDLogView
	: public CComObjectRootEx<CComSingleThreadModel>
	, public CComCoClass<CCoBDLogView, &CLSID_BdLogView>
	, public IDispatchImpl<IBdLogView, &IID_IBdLogView, &LIBID_BdLogviewLib, -1 ,-1>
{
private:
	CComPtr<ILogCenter> m_logCenter;
public:
	CCoBDLogView()
	{
		CComObject<CCoLogCenter>* obj = NULL;
		CComObject<CCoLogCenter>::CreateInstance(&obj);
		m_logCenter = obj;
	}

	virtual ~CCoBDLogView() {}

	BEGIN_COM_MAP(CCoBDLogView)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IBdLogView)
	END_COM_MAP()

	STDMETHOD(MsgBox)(BSTR title, BSTR text)
	{
		::MessageBoxW(NULL, text, title, MB_OK);
		return S_OK;
	}

	STDMETHOD(GetLogCenter)(IDispatch** ppObj)
	{
		return m_logCenter->QueryInterface(&ppObj);
	}
};

class ATL_NO_VTABLE CActiveScriptSite
	: public CComObjectRootEx<CComSingleThreadModel>
	, public IActiveScriptSite
{
	CComPtr<IBdLogView> m_app;
public:
	CActiveScriptSite()
	{
		CComObject<CCoBDLogView>* obj = NULL;
		CComObject<CCoBDLogView>::CreateInstance(&obj);
		m_app = obj;
	}
	virtual ~CActiveScriptSite()
	{
	}

	BEGIN_COM_MAP(CActiveScriptSite)
		COM_INTERFACE_ENTRY(IActiveScriptSite)
	END_COM_MAP()

	STDMETHOD(GetLCID)(LCID *plcid)
	{
		*plcid = 0x0804;
		return S_OK;
	}

	STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti)
	{
		if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
		{
			if (!ppti)
				return E_INVALIDARG;
			*ppti = NULL;
		}

		if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
		{
			if (!ppiunkItem)
				return E_INVALIDARG;
			*ppiunkItem = NULL;
		}

		// Global object
		if (!_wcsicmp(L"application", pstrName))
		{
			if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
			{
				m_app->GetTypeInfo(0, NULL, ppti);
				(*ppti)->AddRef();
			}

			if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
			{
				m_app->QueryInterface(ppiunkItem);
			}
			return S_OK;
		}

		return E_INVALIDARG;
	}

	STDMETHOD(GetDocVersionString)(BSTR * /*pbstrVersion*/)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnScriptTerminate)( const VARIANT * /*pvarResult*/, const EXCEPINFO * /*pexcepinfo*/)
	{
		return S_OK;
	}

	STDMETHOD(OnStateChange)(SCRIPTSTATE /*ssScriptState*/)
	{
		return S_OK;
	}

	STDMETHOD(OnScriptError)(IActiveScriptError *pscripterror)
	{
		EXCEPINFO ei;
		DWORD     dwSrcContext;
		ULONG     ulLine;
		LONG      ichError;
		BSTR      bstrLine = NULL;
		CString strError;

		pscripterror->GetExceptionInfo(&ei);
		pscripterror->GetSourcePosition(&dwSrcContext, &ulLine, &ichError);
		pscripterror->GetSourceLineText(&bstrLine);

		CString desc;
		CString src;

		desc = (LPCWSTR)ei.bstrDescription;
		src = (LPCWSTR)ei.bstrSource;

		strError.Format(L"%s\nSrc: %s\nLine:%d Error:%d Scode:%x", desc, src, ulLine, (int)ei.wCode, ei.scode);
		MessageBoxW(NULL, strError, L"error", MB_OK);
		return S_OK;
	}

	STDMETHOD(OnEnterScript)()
	{
		return S_OK;
	}

	STDMETHOD(OnLeaveScript)()
	{
		return S_OK;
	}
};


CScriptHost::CScriptHost()
{
	InitLibId();

	CoCreateInstance(CLSID_JScript, NULL, CLSCTX_SERVER, IID_IActiveScript, (void**)&m_scriptEngine);

	CComObject<CActiveScriptSite>* as;
	CComObject<CActiveScriptSite>::CreateInstance(&as);
	m_scriptSite = as;
	
	m_scriptEngine->SetScriptSite(m_scriptSite);
	m_scriptEngine->AddNamedItem(L"application", SCRIPTITEM_ISVISIBLE|SCRIPTITEM_NOCODE);

	m_scriptEngine->QueryInterface(IID_IActiveScriptParse, (void**)&m_scriptParser);
	m_scriptParser->InitNew();
}

CScriptHost::~CScriptHost()
{
}

void CScriptHost::RunScript(const wchar_t* jscode)
{
	m_scriptParser->ParseScriptText(jscode,	NULL, NULL, NULL, 0, 1, 0, NULL, NULL);
	m_scriptEngine->SetScriptState(SCRIPTSTATE_CONNECTED);
}
