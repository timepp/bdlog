#pragma  once
#include "resource.h"
#include <wtl/atlframe.h>
#include <wtl/atlcrack.h>
#include "helper.h"
#include "config.h"
#include "historycombo.h"

class CSearchDlg
	: public CDialogImpl<CSearchDlg>
	, public CDialogResize<CSearchDlg>
	, public CModlessDlgKeyProcessor
{
public:
	enum {IDD = IDD_SEARCH_DLG};

	BEGIN_DLGRESIZE_MAP(CSearchDlg)
		DLGRESIZE_CONTROL(IDC_BTN_PREVSEARCH, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_BTN_NEXTSEARCH, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_COMBO_TEXT, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CSearchDlg)
		COMMAND_HANDLER(IDC_BTN_PREVSEARCH, BN_CLICKED, OnPervSearch)
		COMMAND_HANDLER(IDC_BTN_NEXTSEARCH, BN_CLICKED, OnNextSearch)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_INITDIALOG(OnInitDialog)

		CHAIN_MSG_MAP(CModlessDlgKeyProcessor)
		CHAIN_MSG_MAP(CDialogResize<CSearchDlg>)
	END_MSG_MAP()

	CSearchDlg() : m_combo(CConfig::Instance()->GetConfig().search.history)
	{

	}

	CStringW GetSearchText() const
	{
		if (!IsWindow()) return L"";
		CStringW text;
		GetDlgItem(IDC_COMBO_TEXT).GetWindowText(text);
		return text;
	}

private:

	CHistoryComboBox m_combo;

	LRESULT OnInitDialog(HWND hWnd, LPARAM lp)
	{
		DlgResize_Init(false);
		m_combo.SubclassWindow(GetDlgItem(IDC_COMBO_TEXT));
		m_combo.LoadHistory();

		GetDlgItem(IDC_COMBO_TEXT).SetFocus();
		return FALSE;
	}

	void NotifySearch(BOOL searchNext)
	{
		m_combo.SaveHistory();
		GetParent().SendMessage(WM_COMMAND, WPARAM(searchNext? ID_SEARCH_NEXT : ID_SEARCH_PREV), 0);
	}

	LRESULT OnOK(WORD, WORD, HWND, BOOL&)
	{
		NotifySearch(TRUE);
		OnClose();
		return 0;
	}
	LRESULT OnCancel(WORD, WORD, HWND, BOOL&)
	{
		OnClose();
		return 0;
	}
	void OnClose()
	{
		ShowWindow(SW_HIDE);
//		GetParent().SendMessage(WM_COMMAND, ID_CLEAR_SEARCH, 0);	
	}

	LRESULT OnPervSearch(WORD, WORD, HWND , BOOL&)
	{
		NotifySearch(FALSE);
		return 0;
	}
	LRESULT OnNextSearch(WORD, WORD, HWND , BOOL&)
	{
		NotifySearch(TRUE);
		return 0;
	}
};
