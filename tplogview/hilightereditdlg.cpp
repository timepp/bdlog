#include "stdafx.h"
#include "hilightereditdlg.h"

CHilighterEditDlg::CHilighterEditDlg()
: m_dlg(CConfig::Instance()->GetConfig().ui.savedpath.filter)
{
	m_hl.d.bkcolor = RGB(255, 255, 255);
	m_hl.d.color = RGB(0, 0, 0);
	m_hl.f = NULL;
	m_hl.name = L"";
}

void CHilighterEditDlg::SetHilighter(const hilighter& hl)
{
	m_hl = hl;
}

hilighter CHilighterEditDlg::GetHilighter() const
{
	return m_hl;
}

LRESULT CHilighterEditDlg::OnInitDialog(HWND, LPARAM)
{
	CDialogResize<CHilighterEditDlg>::DlgResize_Init();

	m_dlg.SetDefaultPage(-1);
	m_dlg.Create(m_hWnd);
	m_dlg.ShowWindow(SW_SHOW);
	ResizeChildDialogs();

	m_brs.CreateSolidBrush(m_hl.d.bkcolor);

	DisplayHilighter(m_hl);
	CenterWindow(GetParent());

	if (m_hl.name.length() == 0)
	{
		GetDlgItem(IDC_EDIT_HILIGHTER_NAME).SetFocus();
		return FALSE;
	}

	return TRUE;
}

LRESULT CHilighterEditDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	filter* f = m_dlg.GetFilter();
	if (!f)
	{
		MessageBox(L"过滤规则不能为空", L"错误", MB_OK|MB_ICONWARNING);
		return 0;
	}

	CStringW strName;
	GetDlgItemText(IDC_EDIT_HILIGHTER_NAME, strName);
	if (strName.IsEmpty())
	{
		MessageBox(L"必须指定过滤规则名字", L"错误", MB_OK|MB_ICONWARNING);
		return 0;
	}

	m_hl.f = dynamic_cast<filter*>(dynamic_cast<component*>(f)->clone());
	m_hl.name = (LPCWSTR)strName;

	EndDialog(IDOK);
	return TRUE;
}

LRESULT CHilighterEditDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return TRUE;
}

void CHilighterEditDlg::ResizeChildDialogs()
{
	CWindow wnd = GetDlgItem(IDC_STATIC_SUBDLG);
	CRect rc;
	wnd.GetWindowRect(&rc);

	ScreenToClient(&rc);

	m_dlg.MoveWindow(&rc);
}

LRESULT CHilighterEditDlg::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT ret = CDialogResize<CHilighterEditDlg>::OnSize(uMsg, wParam, lParam, bHandled);
	ResizeChildDialogs();
	return ret;
}

void CHilighterEditDlg::DisplayHilighter(const hilighter& h)
{
	m_dlg.SetFilter(h.f);
	SetDlgItemText(IDC_EDIT_HILIGHTER_NAME, h.name.c_str());
}

LRESULT CHilighterEditDlg::OnCtlColorStatic(HDC hDC, HWND hWnd)
{
	CDCHandle dc(hDC);
	if (::GetDlgCtrlID(hWnd) == IDC_STATIC_SAMPLE_LOG)
	{
		dc.SetTextColor(m_hl.d.color);
		dc.SetBkMode(TRANSPARENT);
		return (LRESULT)(HBRUSH)m_brs;
	}

	return 0;
}

LRESULT CHilighterEditDlg::OnBnClickedButtonFg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(m_hl.d.color);
	if (dlg.DoModal() == IDOK)
	{
		m_hl.d.color = dlg.GetColor();
		GetDlgItem(IDC_STATIC_SAMPLE_LOG).RedrawWindow();
	}
	return 0;
}

LRESULT CHilighterEditDlg::OnBnClickedButtonBg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(m_hl.d.bkcolor);
	if (dlg.DoModal() == IDOK)
	{
		m_hl.d.bkcolor = dlg.GetColor();
		m_brs.DeleteObject();
		m_brs.CreateSolidBrush(m_hl.d.bkcolor);
		GetDlgItem(IDC_STATIC_SAMPLE_LOG).RedrawWindow();
	}
	return 0;
}
