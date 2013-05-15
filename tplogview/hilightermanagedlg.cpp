#include "stdafx.h"
#include "hilightermanagedlg.h"
#include "hilightereditdlg.h"

#define LOG_SAMPLE_TEXT L"日志样例 Sample Log"

LRESULT CHilighterManageDlg::OnInitDialog(HWND, LPARAM)
{
	CDialogResize<CHilighterManageDlg>::DlgResize_Init();
	m_list = GetDlgItem(IDC_LIST);
	m_list.SetExtendedListViewStyle(LVS_EX_GRIDLINES);

	struct ColumnInfo
	{
		LPCWSTR name;
		int width;
	} ci[] =
	{
		{L"高亮名", 100},
		{L"预览", 300}
	};
	for (int i = 0; i < _countof(ci); i++)
	{
		m_list.AddColumn(ci[i].name, i);
		m_list.SetColumnWidth(i, ci[i].width);
	}

	const hilighters_t& hls = CConfig::Instance()->GetConfig().hls;
	
	int index = 0;
	for (hilighters_t::const_iterator it = hls.begin(); it != hls.end(); ++it, ++index)
	{
		m_list.InsertItem(index, it->name.c_str());
		m_list.SetItemText(index, 1, LOG_SAMPLE_TEXT);
	}
	if (m_list.GetItemCount() > 0)
	{
		m_list.SelectItem(0);
	}

	SyncControls();
	CenterWindow();

	return FALSE;
}

LRESULT CHilighterManageDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDOK);
	return TRUE;
}

LRESULT CHilighterManageDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return TRUE;
}


LRESULT CHilighterManageDlg::OnBnClickedButtonMoveUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i = m_list.GetSelectedIndex();
	if (i > 0)
	{
		hilighter& hls1 = GetHilighter(i);
		hilighter& hls2 = GetHilighter(i-1);
		std::swap(hls1, hls2);
		m_list.SetItemText(i-1, 0, hls2.name.c_str());
		m_list.SetItemText(i, 0, hls1.name.c_str());
		m_list.SelectItem(i-1);
		m_list.RedrawItems(i-1, i);
	}

	return 0;
}

LRESULT CHilighterManageDlg::OnBnClickedButtonMoveDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i = m_list.GetSelectedIndex();
	if (i >= 0 && i < m_list.GetItemCount() - 1)
	{
		hilighter& hls1 = GetHilighter(i);
		hilighter& hls2 = GetHilighter(i+1);
		std::swap(hls1, hls2);
		m_list.SetItemText(i, 0, hls1.name.c_str());
		m_list.SetItemText(i+1, 0, hls2.name.c_str());
		m_list.SelectItem(i+1);
		m_list.RedrawItems(i, i+1);
	}

	return 0;
}

LRESULT CHilighterManageDlg::OnBnClickedButtonAddHl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CHilighterEditDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		hilighters_t& hls = CConfig::Instance()->GetConfig().hls;
		hilighter hl = dlg.GetHilighter();
		int index = m_list.GetSelectedIndex() + 1;
		hls.insert(hls.begin() + index, hl);
		m_list.InsertItem(index, hl.name.c_str());
		m_list.SetItemText(index, 1, LOG_SAMPLE_TEXT);
		SyncControls();
	}
	
	return 0;
}

LRESULT CHilighterManageDlg::OnBnClickedButtonModifyHl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int index = m_list.GetSelectedIndex();
	if (index < 0) return 0;

	CHilighterEditDlg dlg;
	hilighter& h = GetHilighter(index);
	dlg.SetHilighter(h);
	if (dlg.DoModal() == IDOK)
	{
		hilighter hl = dlg.GetHilighter();
		delete h.f;
		h = hl;
		m_list.SetItemText(index, 0, hl.name.c_str());
		m_list.RedrawItems(index, index);
	}

	return 0;
}

LRESULT CHilighterManageDlg::OnBnClickedButtonDeleteHl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int index = m_list.GetSelectedIndex();
	if (index < 0) return 0;

	hilighters_t& hls = CConfig::Instance()->GetConfig().hls;
	delete hls[(size_t)index].f;
	hls.erase(hls.begin() + index);
	m_list.DeleteItem(index);
	SyncControls();

	return 0;
}

LRESULT CHilighterManageDlg::OnNMDblclkList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
//	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pNMHDR;

	POINT pt;
	::GetCursorPos(&pt);
	m_list.ScreenToClient(&pt);
	LVHITTESTINFO info;
	info.flags = LVHT_ONITEM;
	info.pt = pt;
	m_list.SubItemHitTest(&info);
	m_list.SelectItem(info.iItem);
	SendMessage(WM_COMMAND, IDC_BUTTON_MODIFY_HL, 0);
	return 0;
}


LRESULT CHilighterManageDlg::OnNMCustomdrawList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	AtlTrace(L"drawstage: %u\n", pNMCD->nmcd.dwDrawStage);
	if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		return CDRF_NOTIFYITEMDRAW;
	}
	if (pNMCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		return CDRF_NOTIFYSUBITEMDRAW;
	}
	if (pNMCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
	{
		if (pNMCD->iSubItem == 1)
		{
			int index = static_cast<int>(pNMCD->nmcd.dwItemSpec);
			if (index < m_list.GetItemCount())
			{
				hilighter& hl = GetHilighter(index);
				pNMCD->clrText = hl.d.color;
				pNMCD->clrTextBk = hl.d.bkcolor;
			}
		}
	}
	return CDRF_DODEFAULT;
}

void CHilighterManageDlg::SyncControls()
{
	int index = m_list.GetSelectedIndex();
	bool hasActiveItem = (index >= 0);

	GetDlgItem(IDC_BUTTON_MODIFY_HL).EnableWindow(hasActiveItem);
	GetDlgItem(IDC_BUTTON_DELETE_HL).EnableWindow(hasActiveItem);
}

LRESULT CHilighterManageDlg::OnLvnItemchangedList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
//	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	SyncControls();
	return 0;
}

LRESULT CHilighterManageDlg::OnNMClickList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
//	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pNMHDR;

	POINT pt;
	::GetCursorPos(&pt);
	m_list.ScreenToClient(&pt);
	LVHITTESTINFO info;
	info.flags = LVHT_ONITEM;
	info.pt = pt;
	m_list.SubItemHitTest(&info);
	m_list.SelectItem(info.iItem);

	return 0;
}
LRESULT CHilighterManageDlg::OnNMRClickList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
//	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pNMHDR;

	POINT pt;
	::GetCursorPos(&pt);
	m_list.ScreenToClient(&pt);
	LVHITTESTINFO info;
	info.flags = LVHT_ONITEM;
	info.pt = pt;
	m_list.SubItemHitTest(&info);
	m_list.SelectItem(info.iItem);

	return 0;
}
hilighter& CHilighterManageDlg::GetHilighter(int index)
{
	return CConfig::Instance()->GetConfig().hls[static_cast<size_t>(index)];
}
