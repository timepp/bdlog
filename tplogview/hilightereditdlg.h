#pragma once

#include "resource.h"
#include "filter.h"
#include "filtereditdlg.h"
#include "config.h"
#include "colorselector.h"

class CHilighterEditDlg
	: public CDialogImpl<CHilighterEditDlg>
	, public CDialogResize<CHilighterEditDlg>
{
public:
	CHilighterEditDlg();

	void SetHilighter(const hilighter& hl);
	hilighter GetHilighter() const;

public:
	enum {IDD = IDD_HILIGHTER_EDIT};

	BEGIN_DLGRESIZE_MAP(CHilighterEditDlg)
		DLGRESIZE_CONTROL(IDC_EDIT_HILIGHTER_NAME, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_STATIC_SAMPLE_LOG, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_BUTTON_FG, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_BUTTON_BG, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STATIC_SUBDLG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_SPLITTER2, DLSZ_SIZE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CHilighterEditDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

		COMMAND_HANDLER(IDC_BUTTON_FG, BN_CLICKED, OnBnClickedButtonFg)
		COMMAND_HANDLER(IDC_BUTTON_BG, BN_CLICKED, OnBnClickedButtonBg)
		CHAIN_MSG_MAP(CDialogResize<CHilighterEditDlg>)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(HDC hDC, HWND hWnd);

private:
	CFilterEditDlg m_dlg;
	hilighter m_hl;
	CBrush m_brs;

	void ResizeChildDialogs();
	void DisplayHilighter(const hilighter& h);
	void DisplayCurrentHilighter();
public:
	LRESULT OnBnClickedButtonFg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonBg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
