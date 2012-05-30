#pragma once

#include "modelesskeyprocessor.h"
#include "filter.h"
#include "filtereditdlg.h"

class CFilterSetDlg 
	: public CDialogImpl<CFilterSetDlg>
	, public CDialogResize<CFilterSetDlg>
	, public CModlessDlgKeyProcessor
{
public:
	enum {IDD = IDD_FILTER_SET};

	BEGIN_MSG_MAP(CFilterSetDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_APPLY, OnApply)

		CHAIN_MSG_MAP(CModlessDlgKeyProcessor)
		CHAIN_MSG_MAP(CDialogResize<CFilterSetDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CFilterSetDlg)
		DLGRESIZE_CONTROL(IDC_STATIC_SPLITTER, DLSZ_SIZE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_APPLY, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_SUBDLG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	void Show();

	CFilterSetDlg();

private:
	CFilterEditDlg m_dlg;

	void ResizeChildDialogs();
public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnApply(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
