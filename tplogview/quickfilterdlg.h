#pragma once

#pragma once

#include "resource.h"
#include "common.h"
#include "modelesskeyprocessor.h"
#include "tagselector.h"
#include <wtl/atlframe.h>
#include <wtl/atlcrack.h>
#include <wtl/atlctrls.h>

class CQuickFilterDlg 
	: public CDialogImpl<CQuickFilterDlg>
	, public CDialogResize<CQuickFilterDlg>
	, public CModlessDlgKeyProcessor
{
public:
	enum {IDD = IDD_QUICKFILTER};

	CQuickFilterDlg();

	BEGIN_MSG_MAP(CQuickFilterDlg)
		COMMAND_HANDLER(IDC_QF_TAGS, EN_SETFOCUS, OnEnSetfocusQfTags)
		COMMAND_HANDLER(IDC_QF_TAGS, EN_KILLFOCUS, OnEnKillfocusQfTags)
		COMMAND_HANDLER(IDC_QF_LEVEL, CBN_SELCHANGE, OnCbnSelchangeQfLevel)
		COMMAND_HANDLER(IDC_QF_TAGS, EN_CHANGE, OnEnChangeQfTags)
		COMMAND_HANDLER(IDC_QF_TEXT, EN_CHANGE, OnEnChangeQfText)
		COMMAND_HANDLER(IDC_QF_RESETFILTER, BN_CLICKED, OnBnClickedQfResetfilter)
		CHAIN_MSG_MAP(CModlessDlgKeyProcessor)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
		MSG_WM_CTLCOLORDLG(OnCtlColorStatic)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_MOVE(OnMove)
		MSG_WM_DESTROY(OnDestroy)
		CHAIN_MSG_MAP(CDialogResize<CQuickFilterDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CQuickFilterDlg)
		DLGRESIZE_CONTROL(IDC_QF_TEXT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_QF_RESETFILTER, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

private:
	LRESULT OnInitDialog(HWND hwndFocus, LPARAM lp);
	LRESULT OnCtlColorStatic(HDC hDC, HWND hWnd);
	void OnTimer(UINT_PTR id);
	void SyncLevels();
	void SyncTags();
	void PlaceTagSelector();
	void OnMove(CPoint pt);
	void OnDestroy();
	void UpdateQuickFilter();
	void UpdateUI();

	static LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);

	BOOL HasFilter();

	HHOOK m_hook;

	CBrush m_brsNormal;
	CBrush m_brsHasFilter;

	CComboBox m_comboLevel;
	CTagSelectWnd m_tagSelector;
	CEdit m_editTags;
	CWindow m_frame;

	int m_lvlVersion;
	int m_tagVersion;

	bool m_initing;
	bool m_internalUpdatingUI;
public:
	LRESULT OnEnSetfocusQfTags(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnKillfocusQfTags(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeQfLevel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeQfTags(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeQfText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedQfResetfilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
