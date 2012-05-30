#pragma once

#include "taglist.h"
#include "resource.h"

class CTagSelectWnd 
	: public CDialogImpl<CTagSelectWnd>
	, public CDialogResize<CTagSelectWnd>
{
public:
	enum {IDD = IDD_TIP};

	void Create(HWND hwndParent);
	void AddTag(LPCWSTR pszTags);
	void Clear();

	void AssignWindow(HWND hTarget);
	void SyncFromTarget();
	CSize GetBestSize();

private:
	BEGIN_MSG_MAP(CTagSelectWnd)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_NCACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_ACTIVATEAPP, OnActivate)
		NOTIFY_HANDLER(IDC_LIST, NM_CUSTOMDRAW, OnNMCustomdrawList)
		MESSAGE_HANDLER(WM_CHECKTAG_CHANGED, OnCheckTagChange)

		CHAIN_MSG_MAP(CDialogResize<CTagSelectWnd>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CTagSelectWnd)
		DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	CWindow m_wndTarget;
	CTagList m_list;

	LRESULT OnMouseActivate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnActivate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnInitDialog(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled);
	LRESULT OnNMCustomdrawList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnCheckTagChange(UINT, WPARAM, LPARAM, BOOL&);
};