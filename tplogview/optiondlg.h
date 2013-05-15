#pragma once

#include "resource.h"
#include "whitebkdlg.h"
#include "config.h"
#include "helper.h"
#include "optpage.h"
#include <wtl/atlctrlx.h>
#include <vector>

class COptionDlg
	: public CDialogImpl<COptionDlg>
{
public:
	enum {IDD = IDD_OPTION};

	BEGIN_MSG_MAP(COptionDlg)

		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BUTTON_APPLY, OnApply)

		NOTIFY_HANDLER(IDC_TREE, TVN_SELCHANGED, OnTvnSelchangedTree)
	END_MSG_MAP()

private:
	CTreeViewCtrlEx m_tree;

	COptPage_PerfMark m_optPerfMark;
	COptPage_System m_optSystem;

	std::vector<COptPage*> m_pages;

private:
	void CreateOptionPages();
	void SaveAllPages();
	void SwitchToPage(COptPage* page);
	CTreeItem AddOptionPage(CTreeItem root, LPCWSTR name, COptPage* page);

	void ExpandAllChildItem(CTreeItem item);

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );
	LRESULT OnApply(WORD , WORD , HWND , BOOL& );
public:
	LRESULT OnTvnSelchangedTree(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};


