#include "stdafx.h"
#include "optpage.h"
#include "config.h"
#include "helper.h"

LRESULT COptPage_PerfMark::OnInitDialog(HWND, LPARAM)
{
	m_cs.SubclassWindow(GetDlgItem(IDC_STATIC_COLOR));

	CDlgItem(m_hWnd, IDC_CHECK_ENABLE_PERFMARK).SetCheck(CFG.ui.perfmark.enable);
	m_cs.SetColor(CFG.ui.perfmark.mark_color);
	SetDlgItemInt(IDC_EDIT_MAX_OCCUPYTIME, CFG.ui.perfmark.maxinterval);
	
	return TRUE;
}

void COptPage_PerfMark::Save()
{
	CFG.ui.perfmark.mark_color = m_cs.GetColor();
	CFG.ui.perfmark.maxinterval = GetDlgItemInt(IDC_EDIT_MAX_OCCUPYTIME);
	CFG.ui.perfmark.enable = CDlgItem(m_hWnd, IDC_CHECK_ENABLE_PERFMARK).GetCheck();
}

LRESULT COptPage_System::OnInitDialog(HWND, LPARAM)
{
	CDlgItem(m_hWnd, IDC_EDIT).SetText(CFG.product_name.c_str());
	return TRUE;
}

void COptPage_System::Save()
{
	CFG.product_name = CDlgItem(m_hWnd, IDC_EDIT).GetText();
}

