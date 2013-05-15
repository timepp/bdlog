#pragma once

#include "resource.h"
#include "common.h"
#include "modelesskeyprocessor.h"


class CLogInfoDlg : public CDialogImpl<CLogInfoDlg>, public CDialogResize<CLogInfoDlg>, public CModlessDlgKeyProcessor
{
public:
	enum {IDD = IDD_LOGINFO};

	void ShowLogInfo(const LogInfo& li);

	CLogInfoDlg();

	BEGIN_MSG_MAP(CLogInfoDlg)
		CHAIN_MSG_MAP(CModlessDlgKeyProcessor)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
		MSG_WM_CTLCOLORDLG(OnCtlColorStatic)
		COMMAND_HANDLER(IDC_CHECK_AUTOWRAP, BN_CLICKED, OnBnClickedAutoWrap)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)

		CHAIN_MSG_MAP(CDialogResize<CLogInfoDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CLogInfoDlg)
		DLGRESIZE_CONTROL(IDC_EDIT_CONTENT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_EDIT_CONTENT_WRAP, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CHECK_AUTOWRAP, DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

private:
	void OnClose();
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );
	LRESULT OnInitDialog(HWND hwndFocus, LPARAM lp);
	LRESULT OnCtlColorStatic(HDC hDC, HWND hWnd);
	LRESULT OnBnClickedAutoWrap(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
};