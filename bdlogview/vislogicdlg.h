#pragma once

#include "resource.h"
#include "modelesskeyprocessor.h"
#include "logicvis.h"
#include <accutime.h>

class CVisLogicDlg 
	: public CDialogImpl<CVisLogicDlg>
	, public CDialogResize<CVisLogicDlg>
	, public CModlessDlgKeyProcessor
{
public:
	enum {IDD = IDD_VISLOGIC};

	BEGIN_MSG_MAP(CVisLogicDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		COMMAND_HANDLER(IDC_COMBO_PID, CBN_SELCHANGE, OnCbnSelchangeComboPid)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		CHAIN_MSG_MAP(CModlessDlgKeyProcessor)
		CHAIN_MSG_MAP(CDialogResize<CVisLogicDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CVisLogicDlg)
		DLGRESIZE_CONTROL(IDC_STATIC_SPLITTER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SCROLLBAR, DLSZ_SIZE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_CANVAS, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	static void Show();

	CVisLogicDlg();
	~CVisLogicDlg();

private:
	void Refresh();
	void UpdateCombo();

	void DrawThreads(time_range range, int usec_per_pixel, CDC* pDC, CRect rc);
	void SetThreadStyle(const thread_info* ti, CDC* pDC);

	CLogAccurateTime m_lat;
	const process_info* m_process;

	UINT32 m_channel[1000];

	WTL::CPen m_pen[16];

public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeComboPid(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};
