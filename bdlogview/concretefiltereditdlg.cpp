#include "StdAfx.h"
#include "concretefiltereditdlg.h"
#include "helper.h"
#include "servicehelper.h"

bool CLogClassFilterEditDlg::Do(component* c)
{
	m_filter = dynamic_cast<logclass_filter*>(c);
	return DoModal() == IDOK;
}

LRESULT CLogClassFilterEditDlg::OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
{
	CComboBox combo1 = (HWND)GetDlgItem(IDC_COMBO_LOGCLASS_LOW);
	CComboBox combo2 = (HWND)GetDlgItem(IDC_COMBO_LOGCLASS_HIGH);
	combo1.SetRedraw(FALSE);
	combo2.SetRedraw(FALSE);

	CLogPropertyDB::LevelSet ls = ServiceHelper::GetLogPropertyDB()->GetDefaultLevels();
	ls.insert(0);
	for (CLogPropertyDB::LevelSet::const_iterator it = ls.begin(); it != ls.end(); ++it)
	{
		CStringW name = helper::GetLogLevelDescription(*it);
		int index = combo1.AddString(name);
		combo1.SetItemData(index, (DWORD_PTR)*it);
		int index2 = combo2.AddString(name);
		combo2.SetItemData(index2, (DWORD_PTR)*it);
	}

	combo2.SetRedraw(TRUE);
	combo1.SetRedraw(TRUE);

	SetLogClass(IDC_COMBO_LOGCLASS_HIGH, m_filter->m_class_high);
	SetLogClass(IDC_COMBO_LOGCLASS_LOW, m_filter->m_class_low);
	CenterWindow();

	return 0;
}

LRESULT CLogClassFilterEditDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	m_filter->m_class_high = GetLogClass(IDC_COMBO_LOGCLASS_HIGH);
	m_filter->m_class_low = GetLogClass(IDC_COMBO_LOGCLASS_LOW);
	EndDialog(IDOK);
	return 0;
}

LRESULT CLogClassFilterEditDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return 0;
}

UINT CLogClassFilterEditDlg::GetLogClass(int comboID)
{
	CComboBox combo = (HWND)GetDlgItem(comboID);
	int index = combo.GetCurSel();
	if (index >= 0)
	{
		return combo.GetItemData(index);
	}
	
	CStringW text;
	combo.GetWindowText(text);
	return wcstoul(text, NULL, 10);
}

void CLogClassFilterEditDlg::SetLogClass(int comboID, UINT c)
{
	CComboBox combo = (HWND)GetDlgItem(comboID);
	CStringW desc = helper::GetLogLevelDescription(c);
	if (combo.SelectString(0, desc) == CB_ERR)
	{
		combo.SetWindowText(desc);
	}
}

bool CLogContentFilterEditDlg::Do(component* c)
{
	m_filter = dynamic_cast<logcontent_filter*>(c);
	return DoModal() == IDOK;
}

LRESULT CLogContentFilterEditDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	CStringW text;
	GetDlgItemText(IDC_EDIT_LOGCONTENT, text);
	CButton btn((HWND)GetDlgItem(IDC_CHECK_IGNORECASE));
	bool ics = (btn.GetCheck() == 1);
	CButton btn2((HWND)GetDlgItem(IDC_CHECK_REGEX));
	bool use_regex = (btn2.GetCheck() == 1);
	
	if (!m_filter->setfilter((LPCWSTR)text, ics, use_regex))
	{
		::MessageBoxW(m_hWnd, L"正则表达式解析错误，请检查。", L"错误", MB_ICONWARNING|MB_OK);
		return 0;
	}

	EndDialog(IDOK);
	return 0;
}

LRESULT CLogContentFilterEditDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CLogContentFilterEditDlg::OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
{
	SetDlgItemText(IDC_EDIT_LOGCONTENT, m_filter->m_matcher.c_str());
	if (m_filter->m_ignore_case)
	{
		GetDlgItem(IDC_CHECK_IGNORECASE).SendMessageW(BM_SETCHECK, 1);
	}
	if (m_filter->m_use_regex)
	{
		GetDlgItem(IDC_CHECK_REGEX).SendMessageW(BM_SETCHECK, 1);
	}
	CenterWindow();
	
	return 0;
}



bool CLogTagFilterEditDlg::Do(component* c)
{
	m_filter = dynamic_cast<logtag_filter*>(c);
	return DoModal() == IDOK;
}

LRESULT CLogTagFilterEditDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	CStringW text;
	GetDlgItemText(IDC_EDIT_LOGTAG, text);
	m_filter->setfilter((LPCWSTR)text);
	EndDialog(IDOK);
	return 0;
}

LRESULT CLogTagFilterEditDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CLogTagFilterEditDlg::OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
{
	SetDlgItemText(IDC_EDIT_LOGTAG, m_filter->m_tag.c_str());
	SetHWND(m_hWnd);
	SetStaticTextColor(IDC_STATIC_INFO1, RGB(0, 0, 255));
	SetStaticTextColor(IDC_STATIC_INFO2, RGB(0, 0, 255));
	CenterWindow();
	return 0;
}

bool CLogProcessNameFilterEditDlg::Do(component* c)
{
	m_filter = dynamic_cast<logprocessname_filter*>(c);
	return DoModal() == IDOK;
}

LRESULT CLogProcessNameFilterEditDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	CStringW text;
	GetDlgItemText(IDC_EDIT_LOG_PROCESSNAME, text);
	m_filter->setfilter((LPCWSTR)text);
	EndDialog(IDOK);
	return 0;
}

LRESULT CLogProcessNameFilterEditDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CLogProcessNameFilterEditDlg::OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
{
	SetDlgItemText(IDC_EDIT_LOG_PROCESSNAME, m_filter->m_process_name.c_str());
	CenterWindow();
	return 0;
}


bool CLogPIDFilterEditDlg::Do(component* c)
{
	m_filter = dynamic_cast<logpid_filter*>(c);
	return DoModal() == IDOK;
}

LRESULT CLogPIDFilterEditDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	CStringW text;
	GetDlgItemText(IDC_EDIT_LOGPID, text);
	unsigned int pid;
	swscanf_s((LPCWSTR)text, L"%u", &pid);
	m_filter->setfilter(pid);
	EndDialog(IDOK);
	return 0;
}

LRESULT CLogPIDFilterEditDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CLogPIDFilterEditDlg::OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
{
	SetDlgItemText(IDC_EDIT_LOGPID, tp::cz(L"%u", m_filter->m_pid));
	CenterWindow();
	return 0;
}


bool CLogTIDFilterEditDlg::Do(component* c)
{
	m_filter = dynamic_cast<logtid_filter*>(c);
	return DoModal() == IDOK;
}

LRESULT CLogTIDFilterEditDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	CStringW text;
	GetDlgItemText(IDC_EDIT_LOGTID, text);
	unsigned int tid;
	swscanf_s((LPCWSTR)text, L"%u", &tid);
	m_filter->setfilter(tid);
	EndDialog(IDOK);
	return 0;
}

LRESULT CLogTIDFilterEditDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CLogTIDFilterEditDlg::OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
{
	SetDlgItemText(IDC_EDIT_LOGTID, tp::cz(L"%u", m_filter->m_tid));
	CenterWindow();
	return 0;
}


LRESULT CLogPIDFilterEditDlg::OnEnChangeEditLogpid(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the __super::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here

	return 0;
}
