#pragma  once
#include "resource.h"
#include "scripthost.h"

class CRunScriptDlg
	: public CDialogImpl<CRunScriptDlg>
{
public:
	enum {IDD = IDD_RUNSCRIPT};

	BEGIN_MSG_MAP(CRunScriptDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

private:

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
	{
		CenterWindow(GetParent());

		return 0;
	}
	void OnClose()
	{
		EndDialog(IDOK);
	}
	LRESULT OnOK(WORD , WORD , HWND , BOOL& )
	{
		CStringW code;
		GetDlgItemText(IDC_EDIT_SCRIPT, code);
		SERVICE(CScriptHost)->RunScript(code);
		return 0;
	}
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& )
	{
		OnClose();
		return 0;
	}
};