

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0595 */
/* at Fri Oct 19 18:53:15 2012
 */
/* Compiler settings for ..\bdlogview\bdlogview.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0595 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __bdlogview_h_h__
#define __bdlogview_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IBdLogView_FWD_DEFINED__
#define __IBdLogView_FWD_DEFINED__
typedef interface IBdLogView IBdLogView;

#endif 	/* __IBdLogView_FWD_DEFINED__ */


#ifndef __ILogCenter_FWD_DEFINED__
#define __ILogCenter_FWD_DEFINED__
typedef interface ILogCenter ILogCenter;

#endif 	/* __ILogCenter_FWD_DEFINED__ */


#ifndef __BdLogView_FWD_DEFINED__
#define __BdLogView_FWD_DEFINED__

#ifdef __cplusplus
typedef class BdLogView BdLogView;
#else
typedef struct BdLogView BdLogView;
#endif /* __cplusplus */

#endif 	/* __BdLogView_FWD_DEFINED__ */


#ifndef __LogCenter_FWD_DEFINED__
#define __LogCenter_FWD_DEFINED__

#ifdef __cplusplus
typedef class LogCenter LogCenter;
#else
typedef struct LogCenter LogCenter;
#endif /* __cplusplus */

#endif 	/* __LogCenter_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IBdLogView_INTERFACE_DEFINED__
#define __IBdLogView_INTERFACE_DEFINED__

/* interface IBdLogView */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IBdLogView;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("160C353C-ED83-4dd3-B39F-A0C73E54B8A1")
    IBdLogView : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MsgBox( 
            BSTR title,
            BSTR text) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetLogCenter( 
            /* [retval][out] */ IDispatch **ppObj) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IBdLogViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBdLogView * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBdLogView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBdLogView * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBdLogView * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBdLogView * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBdLogView * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBdLogView * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *MsgBox )( 
            IBdLogView * This,
            BSTR title,
            BSTR text);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetLogCenter )( 
            IBdLogView * This,
            /* [retval][out] */ IDispatch **ppObj);
        
        END_INTERFACE
    } IBdLogViewVtbl;

    interface IBdLogView
    {
        CONST_VTBL struct IBdLogViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBdLogView_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IBdLogView_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IBdLogView_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IBdLogView_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IBdLogView_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IBdLogView_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IBdLogView_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IBdLogView_MsgBox(This,title,text)	\
    ( (This)->lpVtbl -> MsgBox(This,title,text) ) 

#define IBdLogView_GetLogCenter(This,ppObj)	\
    ( (This)->lpVtbl -> GetLogCenter(This,ppObj) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IBdLogView_INTERFACE_DEFINED__ */


#ifndef __ILogCenter_INTERFACE_DEFINED__
#define __ILogCenter_INTERFACE_DEFINED__

/* interface ILogCenter */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ILogCenter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("718BD1AA-4B70-45D1-8D49-B9018DDFA18C")
    ILogCenter : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_LogCount( 
            /* [retval][out] */ long *pCount) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ILogCenterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILogCenter * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILogCenter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILogCenter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILogCenter * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILogCenter * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILogCenter * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILogCenter * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_LogCount )( 
            ILogCenter * This,
            /* [retval][out] */ long *pCount);
        
        END_INTERFACE
    } ILogCenterVtbl;

    interface ILogCenter
    {
        CONST_VTBL struct ILogCenterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILogCenter_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ILogCenter_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ILogCenter_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ILogCenter_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ILogCenter_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ILogCenter_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ILogCenter_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ILogCenter_get_LogCount(This,pCount)	\
    ( (This)->lpVtbl -> get_LogCount(This,pCount) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ILogCenter_INTERFACE_DEFINED__ */



#ifndef __BdLogviewLib_LIBRARY_DEFINED__
#define __BdLogviewLib_LIBRARY_DEFINED__

/* library BdLogviewLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_BdLogviewLib;

EXTERN_C const CLSID CLSID_BdLogView;

#ifdef __cplusplus

class DECLSPEC_UUID("F5C68AE4-272E-4271-BF37-C7D122484089")
BdLogView;
#endif

EXTERN_C const CLSID CLSID_LogCenter;

#ifdef __cplusplus

class DECLSPEC_UUID("85A42930-3187-48C9-A152-16C3515104BD")
LogCenter;
#endif
#endif /* __BdLogviewLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


