#include "stdafx.h"

#include "filtersetdlg.h"
#include "config.h"

extern HWND g_activeModlessDlg;

CFilterSetDlg::CFilterSetDlg()
: m_dlg(CConfig::Instance()->GetConfig().ui.savedpath.filter)
{

}

void CFilterSetDlg::Show()
{
	m_dlg.SetFilter(CConfig::Instance()->GetConfig().log_filter);
	ShowWindow(SW_SHOW);
}

LRESULT CFilterSetDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	CConfig::Instance()->GetConfig().ui.default_filter_cfg_page = m_dlg.GetActivePage();

	BOOL tmp = FALSE;
	OnApply(0, 0, 0, tmp);
	ShowWindow(SW_HIDE);

	return 0;
}

LRESULT CFilterSetDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	CConfig::Instance()->GetConfig().ui.default_filter_cfg_page = m_dlg.GetActivePage();

	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CFilterSetDlg::OnApply(WORD , WORD , HWND , BOOL& )
{
	filter*& f = CConfig::Instance()->GetConfig().log_filter;
	delete f;
	f = NULL;

	component* c = dynamic_cast<component*>(m_dlg.GetFilter());
	if (c)
	{
		f = dynamic_cast<filter*>(c->clone());
	}

	SendMessage(GetParent(), WM_FILTER_CHANGE, 0, 0);
	return 0;
}

LRESULT CFilterSetDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CDialogResize<CFilterSetDlg>::DlgResize_Init();

	int page = CConfig::Instance()->GetConfig().ui.default_filter_cfg_page;
	m_dlg.SetDefaultPage(page);
	m_dlg.Create(m_hWnd);
	m_dlg.ShowWindow(SW_SHOW);
	ResizeChildDialogs();
	CenterWindow(GetParent());
	
	return 0;
}

void CFilterSetDlg::ResizeChildDialogs()
{
	CWindow wnd = GetDlgItem(IDC_STATIC_SUBDLG);
	CRect rc;
	wnd.GetWindowRect(&rc);

	ScreenToClient(&rc);

	m_dlg.MoveWindow(&rc);
}

LRESULT CFilterSetDlg::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT ret = CDialogResize<CFilterSetDlg>::OnSize(uMsg, wParam, lParam, bHandled);
	ResizeChildDialogs();
	return ret;
}
