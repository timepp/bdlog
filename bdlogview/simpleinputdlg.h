#pragma  once
#include "resource.h"

class CSimpleInputDlg
	: public CDialogImpl<CSimpleInputDlg>
{
public:
	enum {IDD = IDD_SIMPLE_INPUT};

	BEGIN_MSG_MAP(CSimpleInputDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	CSimpleInputDlg(LPCWSTR title, LPCWSTR tip) : m_strTitle(title), m_strTip(tip)
	{

	}

	CStringW GetInput() const
	{
		return m_strInput;
	}

private:
	CStringW m_strTitle;
	CStringW m_strTip;
	CStringW m_strInput;

private:

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
	{
		CenterWindow(GetParent());

		SetDlgItemText(IDC_TIP, m_strTip);
		SetWindowText(m_strTitle);

		return 0;
	}

	LRESULT OnOK(WORD , WORD , HWND , BOOL& )
	{
		GetDlgItemText(IDC_EDIT, m_strInput);
		EndDialog(IDOK);
		return 0;
	}
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& )
	{
		EndDialog(IDCANCEL);
		return 0;
	}

};
