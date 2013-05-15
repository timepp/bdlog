#include "stdafx.h"
#include "logsourceinfodlg.h"

LRESULT CLogSourceInfoDlg::OnOK(WORD , WORD , HWND , BOOL&)
{
	EndDialog(IDOK);
	return 0;
}

LRESULT CLogSourceInfoDlg::OnCancel(WORD , WORD , HWND , BOOL&)
{
	EndDialog(IDCANCEL);
	return 0;
}

void CLogSourceInfoDlg::SetLogSourceInfo(const lsds_t& lsds)
{
	m_lsds = lsds;
}

LRESULT CLogSourceInfoDlg::OnInitDialog(HWND , LPARAM)
{
	CDialogResize<CLogSourceInfoDlg>::DlgResize_Init();
	m_list.SubclassWindow(GetDlgItem(IDC_LIST));
	m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	struct
	{
		LPCWSTR name;
		int width;
	}columnInfo[] =
	{
		L"进程", 160,
		L"连接时间", 120, 
		L"进程路径", 400,
	};
	for (int i = 0; i < _countof(columnInfo); i++)
	{
		m_list.AddColumn(columnInfo[i].name, i);
		m_list.SetColumnWidth(i, columnInfo[i].width);
	}

	m_image.Create(16, 16, ILC_COLOR32|ILC_MASK, 2, 2);
	m_list.SetImageList(m_image, LVSIL_SMALL);

	int index = 0;
	for (lsds_t::const_iterator it = m_lsds.begin(); it != m_lsds.end(); ++it, ++index)
	{
		int iconIndex = 0;
		HICON hIcon = ::ExtractIconW(ModuleHelper::GetModuleInstance(), it->ppath, 0);
		if (hIcon)
		{
			iconIndex = m_image.AddIcon(hIcon);
			::DestroyIcon(hIcon);
		}

		m_list.AddItem(0, 0, L"", iconIndex);

		struct tm tt;
		localtime_s(&tt, &it->joinTime);
		wchar_t timebuf[256];
		wcsftime(timebuf, _countof(timebuf), L"%Y-%m-%d %H:%M:%S", &tt);

		wchar_t pidbuf[1024];
		swprintf_s(pidbuf, L"%s(%u)", (LPCWSTR)it->pname, it->pid);

		m_list.SetItemText(0, 0, pidbuf);
		m_list.SetItemText(0, 1, timebuf);
		m_list.SetItemText(0, 2, it->ppath);
	}

	CenterWindow(GetParent());

	return 0;
}
