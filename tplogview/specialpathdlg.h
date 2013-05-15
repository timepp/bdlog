#pragma once

#include "resource.h"

#define SPBTNID_BEGIN 1243
#define SPBTNID_END 1343

class CSpecialPathDlg
	: public CDialogImpl<CSpecialPathDlg>
{
public:
	enum {IDD = IDD_SPECIAL_POS};

	BEGIN_MSG_MAP(CSpecialPathDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_RANGE_HANDLER_EX(SPBTNID_BEGIN, SPBTNID_END, OnCommandRangeHandlerEX)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	void OnClose();
	void OnCommandRangeHandlerEX(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );

private:
	CRect GetDlgItemRect(int nID);
};