#pragma once

#include "resource.h"

struct LogSourceDispInfo
{
	DWORD pid;
	CStringW pname;
	CStringW ppath;
	time_t joinTime;
	size_t logCount;
};

typedef std::vector<LogSourceDispInfo> lsds_t;

class CLogSourceInfoDlg
	: public CDialogImpl<CLogSourceInfoDlg>
	, public CDialogResize<CLogSourceInfoDlg>
{
public:
	enum {IDD = IDD_LOG_SOURCE};
	BEGIN_MSG_MAP(CLogSourceInfoDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CDialogResize<CLogSourceInfoDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CLogSourceInfoDlg)
		DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	void SetLogSourceInfo(const lsds_t& lsds);

private:
	CSortListViewCtrl m_list;
	lsds_t m_lsds;
	CImageList m_image;

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	LRESULT OnOK(WORD , WORD , HWND , BOOL&);
	LRESULT OnCancel(WORD , WORD , HWND , BOOL&);
};
