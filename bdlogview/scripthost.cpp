#include "stdafx.h"
#include <activscp.h>
#include "bdlogview_h.h"

const GUID CLSID_VBScript = {0xb54f3741, 0x5b07, 0x11cf, {0xa4, 0xb0, 0x00, 0xaa, 0x00, 0x4a, 0x55, 0xe8}};
const GUID CLSID_JScript  = {0xf414c260, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};

class ATL_NO_VTABLE CActiveScriptSite
	: public CComObjectRootEx<CComSingleThreadModel>
	, public CComCoClass<CActiveScriptSite, &CLSID_BdLogView>
	, public IDispatchImpl<IBdLogView, &IID_IBdLogView, &LIBID_BdLogviewLib, -1 ,-1>
	, public IActiveScriptSite
{
public:
	virtual ~CActiveScriptSite()
	{

	}

	DECLARE_LIBID(LIBID_BdLogviewLib)

	BEGIN_COM_MAP(CActiveScriptSite)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IActiveScriptSite)
		COM_INTERFACE_ENTRY(IBdLogView)
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
				GetTypeInfo(0, NULL, ppti);
				(*ppti)->AddRef();
			}

			if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
			{
				*ppiunkItem = static_cast<IBdLogView*>(this);
				(*ppiunkItem)->AddRef();
			}
			return S_OK;
		}

		return E_INVALIDARG;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDocVersionString( 
		/* [out] */ BSTR * /*pbstrVersion*/)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE OnScriptTerminate( 
		/* [in] */ const VARIANT * /*pvarResult*/,
		/* [in] */ const EXCEPINFO * /*pexcepinfo*/)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnStateChange( 
		/* [in] */ SCRIPTSTATE /*ssScriptState*/)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnScriptError( 
		/* [in] */ IActiveScriptError *pscripterror)
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

	virtual HRESULT STDMETHODCALLTYPE OnEnterScript( void)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnLeaveScript( void)
	{
		return S_OK;
	}

	STDMETHOD(MsgBox)()
	{
		::MessageBoxW(NULL,L"aaa", L"bbb", MB_OK);
		return S_OK;
	}
};

void RunScript()
{
	HRESULT hr = S_OK;
	HRESULT hrInit;
	//IClassFactory * pfactory = NULL;
	IActiveScript * pscript = NULL;
	IActiveScriptParse * pparse = NULL;
	IActiveScriptSite * psite = NULL;

	hr = hrInit = OleInitialize(NULL);
	if (FAILED(hr))
		goto LError;

	CoCreateInstance(CLSID_JScript, NULL, CLSCTX_SERVER, IID_IActiveScript, (void**)&pscript);

	psite = new CComObject<CActiveScriptSite>;

	CActiveScriptSite::InitLibId();

	hr = pscript->SetScriptSite(psite);
	if (FAILED(hr))
		goto LError;

	hr = pscript->AddNamedItem(L"application", SCRIPTITEM_ISVISIBLE|SCRIPTITEM_NOCODE);

	hr = pscript->QueryInterface(IID_IActiveScriptParse, (void**)&pparse);
	if (FAILED(hr))
		goto LError;

	hr = pparse->InitNew();

	hr = pparse->ParseScriptText(L"application.MsgBox(); \n",
		NULL, NULL, NULL, 0, 1, 0, NULL, NULL);

	if (FAILED(hr))
		goto LError;

	hr = pscript->SetScriptState(SCRIPTSTATE_CONNECTED);

	if (FAILED(hr))
		goto LError;

LError:
	return;
}