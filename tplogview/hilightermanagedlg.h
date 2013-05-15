#pragma once

#include "resource.h"
#include "filter.h"
#include "config.h"

class CHilighterManageDlg
	: public CDialogImpl<CHilighterManageDlg>
	, public CDialogResize<CHilighterManageDlg>
{
public:
	enum {IDD = IDD_HILIGHTER_MANAGE};

	BEGIN_DLGRESIZE_MAP(CHilighterManageDlg)
		DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_BUTTON_ADD_HL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_BUTTON_MODIFY_HL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_BUTTON_DELETE_HL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_BUTTON_MOVE_UP, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_BUTTON_MOVE_DOWN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CHilighterManageDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_BUTTON_MOVE_UP, BN_CLICKED, OnBnClickedButtonMoveUp)
		COMMAND_HANDLER(IDC_BUTTON_MOVE_DOWN, BN_CLICKED, OnBnClickedButtonMoveDown)
		COMMAND_HANDLER(IDC_BUTTON_ADD_HL, BN_CLICKED, OnBnClickedButtonAddHl)
		COMMAND_HANDLER(IDC_BUTTON_MODIFY_HL, BN_CLICKED, OnBnClickedButtonModifyHl)
		COMMAND_HANDLER(IDC_BUTTON_DELETE_HL, BN_CLICKED, OnBnClickedButtonDeleteHl)
		NOTIFY_HANDLER(IDC_LIST, NM_CUSTOMDRAW, OnNMCustomdrawList)
		NOTIFY_HANDLER(IDC_LIST, NM_DBLCLK, OnNMDblclkList)
		NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnLvnItemchangedList)
		NOTIFY_HANDLER(IDC_LIST, NM_CLICK, OnNMClickList)
		NOTIFY_HANDLER(IDC_LIST, NM_RCLICK, OnNMRClickList)
		CHAIN_MSG_MAP(CDialogResize<CHilighterManageDlg>)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );
	LRESULT OnNMCustomdrawList(int /**/, LPNMHDR pNMHDR, BOOL& /**/);

private:
	CListViewCtrl m_list;
	void SyncControls();

	hilighter& GetHilighter(int index);

public:
	LRESULT OnBnClickedButtonMoveUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonMoveDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonAddHl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonModifyHl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonDeleteHl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNMDblclkList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnLvnItemchangedList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMClickList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMRClickList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};
