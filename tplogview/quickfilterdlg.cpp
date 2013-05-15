#include "stdafx.h"
#include "quickfilterdlg.h"
#include "concretefilter.h"
#include "servicehelper.h"
#include "logpropdb.h"
#include "config.h"
#include <tplib/include/msg_crack.h>

CQuickFilterDlg* g_dlg = NULL;

#define IDT_REFRESH 1001

CQuickFilterDlg::CQuickFilterDlg() : m_lvlVersion(0), m_initing(true), m_internalUpdatingUI(false), m_tagVersion(0)
{

}

BOOL CQuickFilterDlg::HasFilter()
{
	return FALSE;
}

void CQuickFilterDlg::SyncTags()
{
	int tagVersion;
	const CLogPropertyDB::TagSet& ts = ServiceHelper::GetLogPropertyDB()->GetTags(&tagVersion);
	if (tagVersion != m_tagVersion)
	{
		m_tagVersion = tagVersion;

		m_tagSelector.Clear();
		for (CLogPropertyDB::TagSet::const_iterator it = ts.begin(); it != ts.end(); ++it)
		{
			m_tagSelector.AddTag((*it).c_str());
		}
	}
}

void CQuickFilterDlg::SyncLevels()
{
	int lvlVersion;
	const CLogPropertyDB::LevelSet& ls = ServiceHelper::GetLogPropertyDB()->GetRuntimeLevels(&lvlVersion);
	if (lvlVersion != m_lvlVersion)
	{
		m_lvlVersion = lvlVersion;

		wchar_t text[1024];
		m_comboLevel.GetWindowText(text, _countof(text));

		CLogPropertyDB::LevelSet s = ServiceHelper::GetLogPropertyDB()->GetDefaultLevels();
		std::copy(ls.begin(), ls.end(), std::inserter(s, s.begin()));
		s.insert(0);

		m_comboLevel.ResetContent();
		for (CLogPropertyDB::LevelSet::const_iterator it = s.begin(); it != s.end(); ++it)		
		{
			CStringW desc = helper::GetLogLevelDescription(*it);
			int i = m_comboLevel.AddString(desc);
			m_comboLevel.SetItemData(i, (DWORD_PTR)*it);
		}

		m_comboLevel.SelectString(0, text);
	}
}

LRESULT CQuickFilterDlg::OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
{
	DlgResize_Init(false);

	m_comboLevel = GetDlgItem(IDC_QF_LEVEL);
	m_editTags = GetDlgItem(IDC_QF_TAGS);
	m_frame = GetTopLevelParent();
	m_tagSelector.Create(m_hWnd);
	m_tagSelector.AssignWindow(m_editTags);

	m_brsHasFilter.CreateSolidBrush(RGB(128,255,128));
	m_brsNormal.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));

	m_internalUpdatingUI = true;
	SyncLevels();
	SyncTags();
	UpdateUI();
	m_internalUpdatingUI = false;
	::PostMessage(GetTopLevelParent(), WM_FILTER_CHANGE, 0, 0);

	m_hook = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)&CQuickFilterDlg::CallWndProc, NULL, GetCurrentThreadId());

	SetTimer(IDT_REFRESH, 1000, NULL);

	g_dlg = this;
	m_initing = false;
	return FALSE;
}

void CQuickFilterDlg::OnTimer(UINT_PTR id)
{
	if (id == IDT_REFRESH)
	{
		SyncLevels();
		SyncTags();
		m_tagSelector.SyncFromTarget();
	}
}

LRESULT CQuickFilterDlg::OnCtlColorStatic(HDC hDC, HWND hWnd)
{
	CDCHandle dc(hDC);
	dc.SetBkMode(TRANSPARENT);
	if (::GetDlgCtrlID(hWnd) == IDC_STATIC_TITLE)
	{
		dc.SetTextColor(RGB(0, 0, 255));
	}

	return (LRESULT)(HBRUSH)(HasFilter()? m_brsHasFilter : m_brsNormal);
}


LRESULT CQuickFilterDlg::OnEnSetfocusQfTags(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PlaceTagSelector();
	m_tagSelector.ShowWindow(SW_SHOWNOACTIVATE);
	
	return 0;
}

void CQuickFilterDlg::PlaceTagSelector()
{
	CRect rc;
	m_editTags.GetWindowRect(&rc);

	CSize size = m_tagSelector.GetBestSize();
	m_tagSelector.SetWindowPos(NULL, rc.left, rc.bottom + 3, rc.Width(), size.cy, SWP_NOACTIVATE);
}

void CQuickFilterDlg::OnMove(CPoint /*pt*/)
{
	if (m_tagSelector.IsWindowVisible())
	{
		PlaceTagSelector();
	}
}

void CQuickFilterDlg::OnDestroy()
{
	UnhookWindowsHookEx(m_hook);
}

LRESULT CQuickFilterDlg::OnEnKillfocusQfTags(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_tagSelector.IsWindow())
		m_tagSelector.ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CALLBACK CQuickFilterDlg::CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && wParam == 0)
	{
		CWPSTRUCT* msg = (CWPSTRUCT*)lParam;
		if (msg->message == WM_MOVING && g_dlg && msg->hwnd == g_dlg->m_frame)
		{
			g_dlg->OnMove(CPoint());
		}
	}
	return ::CallNextHookEx(0, nCode, wParam, lParam);
}

LRESULT CQuickFilterDlg::OnCbnSelchangeQfLevel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!m_internalUpdatingUI)
	{
		UpdateQuickFilter();
		SendMessage(GetTopLevelParent(), WM_FILTER_CHANGE, 0, 0);
	}
	return 0;
}

LRESULT CQuickFilterDlg::OnEnChangeQfTags(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tagSelector.SyncFromTarget();
	if (!m_internalUpdatingUI)
	{
		UpdateQuickFilter();
		SendMessage(GetTopLevelParent(), WM_FILTER_CHANGE, 0, 0);
	}
	return 0;
}

LRESULT CQuickFilterDlg::OnEnChangeQfText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!m_internalUpdatingUI)
	{
		UpdateQuickFilter();
		SendMessage(GetTopLevelParent(), WM_FILTER_CHANGE, 0, 0);
	}
	return 0;
}

void CQuickFilterDlg::UpdateQuickFilter()
{
	if (m_initing) return;

	quickfilter& qf = CConfig::Instance()->GetConfig().log_quickfilter;

	// level
	int index = m_comboLevel.GetCurSel();
	qf.level = m_comboLevel.GetItemData(index);

	// tags
	wchar_t buffer[1024];
	m_editTags.GetWindowText(buffer, 1024);
	qf.tags = buffer;
	
	// text
	GetDlgItem(IDC_QF_TEXT).GetWindowText(buffer, 1024);
	qf.text = buffer;

}

void CQuickFilterDlg::UpdateUI()
{
	const quickfilter& qf = CConfig::Instance()->GetConfig().log_quickfilter;

	m_internalUpdatingUI = true;

	GetDlgItem(IDC_QF_TEXT).SetWindowText(qf.text.c_str());

	m_editTags.SetWindowText(qf.tags.c_str());

	for (int i = 0; i < m_comboLevel.GetCount(); i++)
	{
		if (m_comboLevel.GetItemData(i) == qf.level)
		{
			m_comboLevel.SetCurSel(i);
		}
	}

	m_internalUpdatingUI = false;
}

LRESULT CQuickFilterDlg::OnBnClickedQfResetfilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	quickfilter& qf = CConfig::Instance()->GetConfig().log_quickfilter;
	qf.tags.clear();
	qf.text.clear();
	qf.level = 0;
	UpdateUI();
	::PostMessage(GetTopLevelParent(), WM_FILTER_CHANGE, 0, 0);

	return 0;
}
