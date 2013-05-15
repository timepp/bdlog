#include "stdafx.h"
#include "tagselector.h"
#include "helper.h"
#include "common.h"

LRESULT CTagSelectWnd::OnInitDialog(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled)
{
	DlgResize_Init(false, false, 0);

	m_list.SubclassWindow(GetDlgItem(IDC_LIST));
	m_list.SetExtendedListViewStyle(LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT);

	CRect rc;
	m_list.GetClientRect(&rc);
	m_list.AddColumn(L"", 0);
	m_list.SetColumnWidth(0, rc.Width()-20);
	m_list.InsertItem(0, L"");
	m_list.SetItemText(0, 0, L"全部");

	return TRUE;
}

void CTagSelectWnd::Create(HWND hwndParent)
{
	__super::Create(hwndParent);
}

void CTagSelectWnd::AssignWindow(HWND hTarget)
{
	m_wndTarget = hTarget;
}

void CTagSelectWnd::AddTag(LPCWSTR pszTags)
{
	std::set<CStringW> ss;
	CStringW strText;
	helper::SplitString(pszTags, L";", ss);
	for (std::set<CStringW>::const_iterator it = ss.begin(); it != ss.end(); ++it)
	{
		CStringW strTag = *it;
		// 查找是否有,没有就加上
		int i = 0;
		for (i = 0; i < m_list.GetItemCount(); i++)
		{
			m_list.GetItemText(i, 0, strText);
			if (strText == strTag) break;
		}
		if (i == m_list.GetItemCount())
		{
			m_list.InsertItem(i, L"");
			m_list.SetItemText(i, 0, strTag);
		}
	}
}

void CTagSelectWnd::Clear()
{
	while (m_list.GetItemCount() > 1)
	{
		m_list.DeleteItem(1);
	}
}

LRESULT CTagSelectWnd::OnMouseActivate(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	return MA_NOACTIVATE;
}

LRESULT CTagSelectWnd::OnActivate(UINT, WPARAM, LPARAM, BOOL&)
{
	return TRUE;
}


LRESULT CTagSelectWnd::OnNMCustomdrawList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		return CDRF_NOTIFYITEMDRAW;
	}
	if (pNMCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT &&
		pNMCD->nmcd.dwItemSpec == (DWORD_PTR)m_list.GetSelectedIndex())
	{
//		pNMCD->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT);
//		pNMCD->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
	}
	return 0;
}

void CTagSelectWnd::SyncFromTarget()
{
	CStringW strText;
	m_wndTarget.GetWindowText(strText);

	std::set<CStringW> ss;
	helper::SplitString(strText, L";", ss);

	BOOL bAllChecked = TRUE;
	for (int i = 1; i < m_list.GetItemCount(); i++)
	{
		CStringW strTag;
		m_list.GetItemText(i, 0, strTag);
		if (ss.find(strTag) != ss.end())
		{
			m_list.SetCheckState(i, TRUE);
		}
		else
		{
			m_list.SetCheckState(i, FALSE);
			bAllChecked = FALSE;
		}
	}
	m_list.SetCheckState(0, bAllChecked);
}

LRESULT CTagSelectWnd::OnCheckTagChange(UINT, WPARAM, LPARAM, BOOL &)
{
	CStringW strText;
	m_wndTarget.GetWindowText(strText);

	std::set<CStringW> ss;
	helper::SplitString(strText, L";", ss);

	for (int i = 1; i < m_list.GetItemCount(); i++)
	{
		CStringW strTag;
		m_list.GetItemText(i, 0, strTag);
		BOOL bChecked = m_list.GetCheckState(i);
		std::set<CStringW>::iterator it = ss.find(strTag);
		if (it != ss.end() && !bChecked)
		{
			ss.erase(it);
		}
		else if (it == ss.end() && bChecked)
		{
			ss.insert(strTag);
		}
	}

	strText = L"";
	for (std::set<CStringW>::const_iterator it = ss.begin(); it != ss.end(); ++it)
	{
		strText += *it;
		strText += L";";
	}
	strText.TrimRight(L";");
	m_wndTarget.SetWindowText(strText);

	return 0;
}

CSize CTagSelectWnd::GetBestSize()
{
	DWORD dim = m_list.ApproximateViewRect();
	CSize sz(LOWORD(dim)+5, HIWORD(dim)+5);
	if (sz.cy > 300) sz.cy = 300;
	return sz;
}