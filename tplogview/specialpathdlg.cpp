#include "stdafx.h"
#include "specialpathdlg.h"
#include "helper.h"

struct spinfo
{
	const wchar_t* name;
	const wchar_t* path;
} sp[] = 
{
	{ L"数据目录",     L"<CSIDL:APPDATA>\\Baidu\\<VAR:PRODUCT>\\"        }, 
	{ L"公用数据目录", L"<CSIDL:COMMON_APPDATA>\\Baidu\\<VAR:PRODUCT>\\" }, 
	{ L"",             L""                                             }, 
	{ L"临时目录",     L"%temp%\\Baidu\\<VAR:PRODUCT>\\" }, 
	{ L"系统临时目录", L"%systemroot%\\temp" }, 
	{ L"",             L""                                             }, 
	{ L"安装目录",     L"<REG:HKLM\\Software\\Baidu\\<VAR:PRODUCT>\\exePath>" }, 
};

void CSpecialPathDlg::OnClose()
{
	EndDialog(IDOK);
}

LRESULT CSpecialPathDlg::OnInitDialog(HWND, LPARAM)
{
	CRect rcBtn1 = GetDlgItemRect(IDC_BUTTON_REF1);
	CRect rcBtn2 = GetDlgItemRect(IDC_BUTTON_REF2);
	CRect rcSta1 = GetDlgItemRect(IDC_STATIC_REF1);
	CRect rcSta2 = GetDlgItemRect(IDC_STATIC_REF2);
	CRect rcSta3 = GetDlgItemRect(IDC_STATIC_REF3);
	
	CRect rcBtn = rcBtn1;
	CRect rcSta = rcSta1;
	int gap = rcBtn2.top - rcBtn1.top;
	int d = rcBtn2.top - rcBtn1.bottom;
	for (size_t i = 0; i < _countof(sp); i++)
	{
		if (sp[i].name[0])
		{
			CButton btn;
			btn.Create(m_hWnd, rcBtn, sp[i].name, WS_CHILD|WS_VISIBLE);
			btn.SetFont(GetFont());
			btn.SetDlgCtrlID(SPBTNID_BEGIN + static_cast<int>(i));
			CStatic st;
			st.Create(m_hWnd, rcSta, helper::ExpandPath(sp[i].path), WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE);
			st.SetFont(GetFont());
			rcBtn.OffsetRect(0, gap);
			rcSta.OffsetRect(0, gap);
		}
		else
		{
			CStatic st;
			CRect rcStaSplitter = rcBtn;
			rcStaSplitter.right = rcSta1.right;
			st.Create(m_hWnd, rcStaSplitter, L"", WS_CHILD|WS_VISIBLE|SS_ETCHEDHORZ);
			rcBtn.OffsetRect(0, d + 2);
			rcSta.OffsetRect(0, d + 2);
		}
	}

	CenterWindow(GetParent());

	return 0;
}

CRect CSpecialPathDlg::GetDlgItemRect(int nID)
{
	CRect rc;
	CWindow wnd = GetDlgItem(nID);
	wnd.GetWindowRect(&rc);
	ScreenToClient(&rc);

	return rc;
}

void CSpecialPathDlg::OnCommandRangeHandlerEX(UINT uNotifyCode, int nID, ATL::CWindow wndCtl)
{
	int index = nID - SPBTNID_BEGIN;
	CStringW strPath = helper::ExpandPath(sp[index].path);
	if (strPath.Right(1) == L"\\")
	{
		::ShellExecuteW(NULL, L"open", strPath, NULL, NULL, SW_SHOW);
	}
	else
	{
		::ShellExecuteW(NULL, L"open", strPath, NULL, NULL, SW_SHOW);
	}
}


LRESULT CSpecialPathDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return 0;
}
