#include "stdafx.h"

#include "loginfodlg.h"
#include "helper.h"
#include <atltime.h>
#include "logpropdb.h"

CLogInfoDlg::CLogInfoDlg()
{

}

void CLogInfoDlg::OnClose()
{
	ShowWindow(SW_HIDE);
}

LRESULT CLogInfoDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	OnClose();
	return 0;
}


LRESULT CLogInfoDlg::OnInitDialog(HWND hwndFocus, LPARAM lp)
{
	CenterWindow(GetParent());
	DlgResize_Init(false);

	GetDlgItem(IDC_CHECK_AUTOWRAP).SetFocus();

	return FALSE;
}

LRESULT CLogInfoDlg::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
	lpMMI->ptMinTrackSize =  CPoint(420, 140);
	return 0;
}


void CLogInfoDlg::ShowLogInfo(const LogInfo& li)
{
	CString strInfo;
	bdlog::logitem* item = li.item;

	strInfo.Format(L"%I64u", item->log_index);
	SetDlgItemTextW(IDC_EDIT_ID, strInfo);

	CStringW strClass;
	const wchar_t* desc = CLogPropertyDB::GetLevelDesc(item->log_class);
	if (!desc) desc = L"-";
	strClass.Format(L"%s(%u)", desc, item->log_class);
	SetDlgItemTextW(IDC_EDIT_CLASS, strClass);

	strInfo.Format(L"%s(%d)", (LPCWSTR)item->log_process_name.c_str(), item->log_pid);
	SetDlgItemTextW(IDC_EDIT_PID, strInfo);

	strInfo.Format(L"%x", item->log_tid);
	SetDlgItemTextW(IDC_EDIT_TID, strInfo);

	CTime t(item->log_time_sec);
	strInfo.Format(L"%s  (.%d)", (LPCWSTR)t.Format(L"%Y-%m-%d %H:%M:%S"), item->log_time_msec);
	SetDlgItemTextW(IDC_EDIT_TIME, strInfo);

	SetDlgItemTextW(IDC_EDIT_TAG, item->log_tags.c_str());

	strInfo.Format(L"%u", item->log_depth);
	SetDlgItemTextW(IDC_EDIT_CALLDEPTH, strInfo);

	CString strContent = item->log_content.c_str();
	strContent.Replace(L"\r\n", L"\n");
	strContent.Replace(L"\n", L"\r\n");
	SetDlgItemTextW(IDC_EDIT_CONTENT, strContent);
	SetDlgItemTextW(IDC_EDIT_CONTENT_WRAP, strContent);
}

LRESULT CLogInfoDlg::OnCtlColorStatic(HDC hDC, HWND hWnd)
{
	CDCHandle dc(hDC);
	if (::GetDlgCtrlID(hWnd) == IDC_STATIC)
	{
		dc.SetTextColor(RGB(0, 0, 255));
	}

	return (LRESULT)::GetStockObject(WHITE_BRUSH);
}

LRESULT CLogInfoDlg::OnBnClickedAutoWrap(WORD , WORD , HWND , BOOL& )
{
	CButton btn = GetDlgItem(IDC_CHECK_AUTOWRAP).m_hWnd;
	
	int check = btn.GetCheck();
	GetDlgItem(IDC_EDIT_CONTENT).ShowWindow(check? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_EDIT_CONTENT_WRAP).ShowWindow(check? SW_SHOW : SW_HIDE);

	return TRUE;
}