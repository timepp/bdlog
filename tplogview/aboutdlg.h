#pragma  once
#include "resource.h"
#include "whitebkdlg.h"
#include "config.h"
#include "helper.h"

#define DLL_DOWNLOAD_URL L"ftp://tmp:tmp@win.baidu.com/incoming/timepp/tplogview.exe"

const LPCWSTR g_tips[] =
{
	L"可以用ctrl+f在过滤结果中进行搜索哦",
	L"日志信息对话框可以选择自动换行",
	L"可以配置高亮规则，把符合条件的日志突出显示",
};

class CAboutDlg
	: public CDialogImpl<CAboutDlg>
	, public CColoredDlgImpl
{
public:
	enum {IDD = IDD_ABOUT};

	BEGIN_MSG_MAP(CAboutDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BTN_DOWNLOAD, OnDownload)

		COMMAND_HANDLER(IDC_BUTTON_SHOW_VERSIONINFO, BN_CLICKED, OnBnClickedButtonShowVersioninfo)
		CHAIN_MSG_MAP(CColoredDlgImpl)
	END_MSG_MAP()

private:

	CHyperLink m_link;

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
	{
		CenterWindow(GetParent());

		CStringW strConfPath = CConfig::Instance()->GetConfigFilePath();
		m_link.SubclassWindow(GetDlgItem(IDC_STATIC_CONF_PATH));
		m_link.SetLabel(strConfPath);
		m_link.SetHyperLink(strConfPath);

		GetDlgItem(IDC_STATIC_PRODUCT_NAME).SetWindowText(CConfig::Instance()->GetConfig().product_name.c_str());
		GetDlgItem(IDC_STATIC_VERSION).SetWindowText(helper::GetVersion());

		SetStaticTextColor(IDC_STATIC_INFO, RGB(0, 0, 255));
		CConfig::Instance()->Save();

		return 0;
	}
	void OnClose()
	{
		EndDialog(IDOK);
	}
	LRESULT OnOK(WORD , WORD , HWND , BOOL& )
	{
		OnClose();
		return 0;
	}
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& )
	{
		OnClose();
		return 0;
	}
	LRESULT OnDownload(WORD , WORD , HWND , BOOL& )
	{
		return 0;
	}
public:
	LRESULT OnBnClickedButtonShowVersioninfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
