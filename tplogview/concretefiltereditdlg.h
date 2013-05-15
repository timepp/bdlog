#pragma once

#include "resource.h"
#include <tplib/include/compositetreectrl.h>
#include "concretefilter.h"
#include "whitebkdlg.h"

class CComponentConfigCommand
{
public:
	virtual bool Do(component* c) = 0;
	virtual ~CComponentConfigCommand(){}
};


class CLogClassFilterEditDlg 
	: public CDialogImpl<CLogClassFilterEditDlg>
	, public CComponentConfigCommand
{
public:
	enum {IDD = IDD_FILTER_EDIT_LOGCLASS};

	virtual bool Do(component* c);

	BEGIN_MSG_MAP(CLogClassFilterEditDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );

private:
	UINT GetLogClass(int comboID);
	void SetLogClass(int comboID, UINT c);

	logclass_filter* m_filter;
};

class CLogContentFilterEditDlg
	: public CDialogImpl<CLogContentFilterEditDlg>
	, public CComponentConfigCommand
{
public:
	enum {IDD = IDD_FILTER_EDIT_LOGCONTENT};

	virtual bool Do(component* c);

	BEGIN_MSG_MAP(CLogContentFilterEditDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );

private:
	logcontent_filter* m_filter;
};

class CLogTagFilterEditDlg
	: public CDialogImpl<CLogTagFilterEditDlg>
	, public CColoredDlgImpl
	, public CComponentConfigCommand
{
public:
	enum {IDD = IDD_FILTER_EDIT_LOGTAG};

	virtual bool Do(component* c);

	BEGIN_MSG_MAP(CLogTagFilterEditDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CColoredDlgImpl)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );

private:
	logtag_filter* m_filter;
};



class CLogProcessNameFilterEditDlg
	: public CDialogImpl<CLogProcessNameFilterEditDlg>
	, public CComponentConfigCommand
{
public:
	enum {IDD = IDD_FILTER_EDIT_PROCESSNAME};

	virtual bool Do(component* c);

	BEGIN_MSG_MAP(CLogProcessNameFilterEditDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );

private:
	logprocessname_filter* m_filter;
};



class CLogPIDFilterEditDlg
	: public CDialogImpl<CLogPIDFilterEditDlg>
	, public CComponentConfigCommand
{
public:
	enum {IDD = IDD_FILTER_EDIT_PID};

	virtual bool Do(component* c);

	BEGIN_MSG_MAP(CLogPIDFilterEditDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_EDIT_LOGPID, EN_CHANGE, OnEnChangeEditLogpid)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );

private:
	logpid_filter* m_filter;
public:
	LRESULT OnEnChangeEditLogpid(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};



class CLogTIDFilterEditDlg
	: public CDialogImpl<CLogTIDFilterEditDlg>
	, public CComponentConfigCommand
{
public:
	enum {IDD = IDD_FILTER_EDIT_TID};

	virtual bool Do(component* c);

	BEGIN_MSG_MAP(CLogTIDFilterEditDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );

private:
	logtid_filter* m_filter;
};
