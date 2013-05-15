#include "StdAfx.h"
#include "aboutdlg.h"
#include "updater.h"

LRESULT CAboutDlg::OnBnClickedButtonShowVersioninfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShellExecute(NULL, L"open", Updater::GetVersionInfoFilePath(), NULL, NULL, SW_SHOW);
	return 0;
}
