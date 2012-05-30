#include "stdafx.h"
#include "mainframe.h"
#include <atltime.h>
#include "helper.h"
#include "config.h"
#include "aboutdlg.h"
#include <wtl/atldlgs.h>
#include <math.h>
#include <share.h>
#include <shlobj.h>
#include "filtersetdlg.h"
#include "specialpathdlg.h"
#include "concretefilter.h"
#include "logsourceinfodlg.h"
#include "hilightermanagedlg.h"
#include "logpropdb.h"
#include "filedialog.h"
#include <Strsafe.h>
#include "vislogicdlg.h"
#include "servicehelper.h"

extern CAppModule _Module;
extern HANDLE g_pipe;

#define IDT_FILL_TIMER 1001
#define IDT_UPDATE_TIMER 1003

#define MQCMD_LOADCONFIG 1
#define MQCMD_LOADLOGHISTORY 2
#define MQCMD_OPENXLOG 3


#undef MULTI_THREAD_GUARD
#define MULTI_THREAD_GUARD(cs) \
	cs.Enter(); \
	ON_LEAVE_1(cs.Leave(), CCriticalSection&, cs)

CMainFrame::CMainFrame()
: m_bShowToolbar(true)
, m_bShowStatusbar(true)
, m_cfg(CConfig::Instance()->GetConfig())
, m_functionBegin(-1)
, m_functionEnd(-1)
{
	m_isHex = false;
}

void CMainFrame::CreateReBar()
{
	struct ToolBarButtonInfo
	{
		int cmdID;
		BYTE state;
		BYTE style;
		UINT iconID;
	} tbbi[] =
	{
		{ID_START_MONITOR,   TBSTATE_ENABLED, BTNS_BUTTON, IDI_START},
		{ID_STOP_MONITOR,    TBSTATE_ENABLED, BTNS_BUTTON, IDI_STOP},
		{0,                  0, BTNS_SEP,    0},
		{ID_TOGGLE_BOOKMARK, TBSTATE_ENABLED, BTNS_BUTTON, IDI_STAR},
		{ID_PREV_BOOKMARK,   TBSTATE_ENABLED, BTNS_BUTTON, IDI_STAR_PREV},
		{ID_NEXT_BOOKMARK,   TBSTATE_ENABLED, BTNS_BUTTON, IDI_STAR_NEXT},
		{0,                  0, BTNS_SEP,    0},
		{ID_FORCEAUTOSCROLL, TBSTATE_ENABLED, BTNS_BUTTON, IDI_DOWN},
		{ID_CLEARALL, TBSTATE_ENABLED, BTNS_BUTTON, IDI_CLEAR},
	};

	m_toolbar.Create(m_hWnd, 0, 0, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	m_toolbar.GetToolTips().SetDlgCtrlID(IDC_TOOLBAR);
	m_toolbarImageList.Create(16, 16, ILC_COLOR32|ILC_MASK, 10, 10);
	m_toolbar.SetImageList(m_toolbarImageList);
	int index = 0;
	for (size_t i = 0; i < _countof(tbbi); ++i, ++index)
	{
		if (tbbi[i].iconID != 0)
		{
			m_toolbarImageList.AddIcon(::LoadIconW(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCEW(tbbi[i].iconID)));
			m_toolbar.InsertButton(index, tbbi[i].cmdID, tbbi[i].style, tbbi[i].state, m_toolbarImageList.GetImageCount()-1, 0, NULL);
		}
		else
		{
			m_toolbar.InsertButton(index, 0, BTNS_SEP, 0, 7, 0, NULL);
		}
	}

	m_cmdbar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_cmdbar.AttachMenu(GetMenu());
//	m_cmdbar.LoadImages(IDR_MAINFRAME);
	SetMenu(NULL);

//	m_edit.Create(m_hWnd, CRect(0, 0, 150, 20), 0, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
//	m_editFont.CreateFontW(-16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"宋体");
//	m_edit.SetFont(m_editFont);
//	m_edit.SetDlgCtrlID(IDC_SEARCH_EDIT);

	m_qfdlg.Create(m_hWnd);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(m_cmdbar);
	AddSimpleReBarBand(m_toolbar, NULL, TRUE);
	AddSimpleReBarBand(m_qfdlg, NULL, 0, 0, TRUE);
	m_rebar = m_hWndToolBar;

	UIAddToolBar(m_toolbar);
	UISetCheck(ID_VIEW_TOOLBAR, true);
}

void CMainFrame::CreateStatusBar()
{
	int panes[] = {ID_DEFAULT_PANE, ID_PANE_LOGCOUNT, ID_PANE_FILTERED_LOGCOUNT, ID_PANE_WORKTHREAD_COUNT};
	m_wndStatusbar.Create(m_hWnd);
	m_wndStatusbar.SetPanes(panes, _countof(panes), true);
	m_wndStatusbar.SetPaneText(ID_PANE_LOGCOUNT, L"总数:0");
	m_wndStatusbar.SetPaneText(ID_PANE_FILTERED_LOGCOUNT, L"显示:0");
	m_wndStatusbar.SetPaneText(ID_PANE_WORKTHREAD_COUNT, L"日志源:0");

	m_hWndStatusBar = m_wndStatusbar;
	UIAddStatusBar(m_hWndStatusBar);
	UISetCheck(ID_VIEW_STATUS_BAR, true);
}

void CMainFrame::CreateList()
{
	DWORD dwStyle = LVS_REPORT|LVS_SINGLESEL|LVS_OWNERDATA|LVS_SHOWSELALWAYS;
	DWORD dwExStyle = LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_INFOTIP|LVS_EX_BORDERSELECT;
	if (CConfig::Instance()->GetConfig().ui.list.show_gridline) dwExStyle |= LVS_EX_GRIDLINES;

	m_list.Create(m_hWnd, rcDefault, NULL, WS_CHILD|WS_BORDER|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|dwStyle, WS_EX_ACCEPTFILES);
	m_list.SetExtendedListViewStyle(dwExStyle);
	m_list.SetDlgCtrlID(IDC_LIST);
	m_listFont.CreateFontW(-12, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"宋体");
	m_list.SetFont(m_listFont);
	m_listImageList.Create(16, 16, ILC_COLOR32|ILC_MASK, 2, 2);
	m_listImageList.AddIcon(::LoadIconW(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCEW(IDI_STAR)));
	m_list.SetImageList(m_listImageList, LVSIL_SMALL);

	struct
	{
		LPCWSTR name;
		int width;
	}columnInfo[] =
	{
		L"", 20,
		L"序号", 60, 
		L"时间", 90,
		L"级别", 40,
		L"进程", 130,
		L"线程", 50,
		L"内容", 600,
	};
	for (int i = 0; i < _countof(columnInfo); i++)
	{
		m_list.AddColumn(columnInfo[i].name, i);
		m_list.SetColumnWidth(i, columnInfo[i].width);
	}

	m_hWndClient = m_list;
}

void CMainFrame::OnSysCommand(UINT id, CPoint pt)
{
	if (id == ID_VIEW_MENUBAR)
	{
		SendMessage(WM_COMMAND, ID_VIEW_MENUBAR, 0);
	}
	SetMsgHandled(FALSE);
}

LRESULT CMainFrame::OnCreate( LPCREATESTRUCTW /*cs*/ )
{
	CreateReBar();
	CreateStatusBar();
	CreateList();
	m_infoDlg.Create(m_hWnd);
	m_tagSelector.Create(m_hWnd);
	m_filterDlg.Create(m_hWnd);
	UpdateUI();

	CMenu mu = GetSystemMenu(FALSE);
	mu.InsertMenu(0, MF_BYPOSITION|MF_SEPARATOR, 0U, L"");
	mu.InsertMenu(0, MF_BYPOSITION|MF_STRING, ID_VIEW_MENUBAR, L"显示/隐藏菜单\tF9");

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	m_lat.Init();

	SetTimer(IDT_UPDATE_TIMER, 500);

	ServiceHelper::GetLogCenter()->AddListener(this);

	if (CConfig::Instance()->GetConfig().first_time_run)
	{
		PostMessage(WM_USER_FIRST_TIME_RUN);
	}

	if (!GD.strXLogFile.IsEmpty())
	{
		PostMessage(WM_MQINVOKE, MQCMD_OPENXLOG);
	}
	else
	{
		PostMessage(WM_COMMAND, ID_START_MONITOR);
	}

	return 0;
}

LRESULT CMainFrame::OnViewMenubar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	helper::bool_reverse(CConfig::Instance()->GetConfig().ui.placement.show_menubar);
	UpdateUI();

	return 0;
}

LRESULT CMainFrame::OnViewToolbar(WORD , WORD , HWND , BOOL& )
{
	helper::bool_reverse(CConfig::Instance()->GetConfig().ui.placement.show_toolbar);
	UpdateUI();

	return 0;
}

LRESULT CMainFrame::OnViewSearchbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	helper::bool_reverse(CConfig::Instance()->GetConfig().ui.placement.show_filterbar);
	UpdateUI();

	return 0;
}

LRESULT CMainFrame::OnViewStatusbar(WORD , WORD , HWND , BOOL& )
{
	helper::bool_reverse(CConfig::Instance()->GetConfig().ui.placement.show_statusbar);
	UpdateUI();

	return 0;
}

void CMainFrame::UpdateUI()
{
	CReBarCtrl rebar = m_hWndToolBar;
	const config::ui_cfg& ui = CConfig::Instance()->GetConfig().ui;

	rebar.ShowBand(rebar.IdToIndex(ATL_IDW_BAND_FIRST + 0), ui.placement.show_menubar);
	UISetCheck(ID_VIEW_MENUBAR, ui.placement.show_menubar);

	rebar.ShowBand(rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1), ui.placement.show_toolbar);
	UISetCheck(ID_VIEW_TOOLBAR, ui.placement.show_toolbar);

	rebar.ShowBand(rebar.IdToIndex(ATL_IDW_BAND_FIRST + 2), ui.placement.show_filterbar);
	UISetCheck(ID_VIEW_SEARCHBAR, ui.placement.show_filterbar);

	::ShowWindow(m_hWndStatusBar, ui.placement.show_statusbar ? SW_SHOWNOACTIVATE : SW_HIDE);

	UpdateLayout();
}

LRESULT CMainFrame::OnStopMonitor(WORD, WORD, HWND, BOOL&)
{
	ServiceHelper::GetLogCenter()->Disconnect();

	m_toolbar.EnableButton(ID_STOP_MONITOR, FALSE);
	m_toolbar.EnableButton(ID_START_MONITOR, TRUE);
	UpdateWindowTitle();

	return 0;
}

LRESULT CMainFrame::OnStartMonitor(WORD, WORD, HWND, BOOL&)
{
	ClearAllLog();

	GD.strXLogFile = L"";
	ServiceHelper::GetLogCenter()->ConnectPipe();

	m_toolbar.EnableButton(ID_START_MONITOR, FALSE);
	m_toolbar.EnableButton(ID_STOP_MONITOR, TRUE);
	UpdateWindowTitle();

	return 0;
}

void CMainFrame::Export(BOOL bFilter)
{
	struct inner
	{
		static void WriteOneLog(FILE* fp, const LogInfo* pLog)
		{
			WCHAR rtpos[256];
			CTime tm(pLog->item->log_time_sec);
			_snwprintf_s(rtpos, _TRUNCATE, L"[%s:%x:%x:%d]", 
				pLog->item->log_process_name.c_str(), 
				pLog->item->log_pid, 
				pLog->item->log_tid, 
				pLog->item->log_depth);
			fwprintf_s(fp, L"%s.%03d %02d %32s {%-16s} %s\n", 
				(LPCWSTR)tm.Format(L"%Y-%m-%d %H:%M:%S"), 
				pLog->item->log_time_msec, 
				pLog->item->log_class, 
				rtpos,
				pLog->item->log_tags.c_str(),
				pLog->item->log_content.c_str());
		}
	};
	CSaveFileDialog dlg(
		m_cfg.ui.savedpath.logfile.c_str(),
		L"xlog", 
		CTime::GetCurrentTime().Format(L"%Y%m%d%H%M%S"),
		L"日志文件(*.xlog)\0*.xlog\0文本文件(*.txt)\0*.txt\0所有文件(*.*)\0*.*\0\0",
		m_hWnd);
	if (dlg.DoModal() == IDOK)
	{
		CConfig::Instance()->GetConfig().ui.savedpath.logfile = (LPCWSTR)helper::GetDir(dlg.m_szFileName);

		FILE* fp = NULL;
		_wfopen_s(&fp, dlg.m_szFileName, L"wt,ccs=UNICODE");
		if (!fp)
		{
			// TODO 提示
			return;
		}

		CLogCenter& lc = *ServiceHelper::GetLogCenter();
		lc.LockLog();

		if (bFilter)
		{
			for (FilterMap::const_iterator it = m_lvs.begin(); it != m_lvs.end(); ++it)
			{
				LogInfo* pLog = lc.GetLog(*it);
				inner::WriteOneLog(fp, pLog);
			}
		}
		else
		{
			LogRange range = lc.GetLogRange();
			for (UINT64 i = range.idmin; i < range.idmax; i++)
			{
				LogInfo* pLog = lc.GetLog(i);
				inner::WriteOneLog(fp, pLog);
			}
		}

		lc.UnlockLog();

		fclose(fp);
	}
}

LRESULT CMainFrame::OnOpenXLog(WORD, WORD, HWND, BOOL&)
{
	COpenFileDialog dlg(
		m_cfg.ui.savedpath.logfile.c_str(), 
		L"日志(*.log,*.txt,*.xlog,*.bdlog)\0*.log;*.txt;*.xlog;*.bdlog\0所有文件(*.*)\0*.*\0\0");
	if (dlg.DoModal() == IDOK)
	{
		GD.strXLogFile = dlg.m_szFileName;
		CConfig::Instance()->GetConfig().ui.savedpath.logfile = (LPCWSTR)helper::GetDir(dlg.m_szFileName);
		PostMessage(WM_MQINVOKE, MQCMD_OPENXLOG);
	}
	return 0;
}

LRESULT CMainFrame::OnUserFirstTimeRun(UINT /*uMsg*/, WPARAM /*wp*/, LPARAM /*lp*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMainFrame::OnEditXlogIni(WORD, WORD, HWND, BOOL&)
{
	CStringW strXlogPath = helper::ExpandPath(BSP_BDXLOG_INI);
	helper::WriteFileIfNotExist(strXlogPath, MAKEINTRESOURCEW(IDR_BDXLOG_INI), L"PDATA");
	::ShellExecuteW(m_hWnd, L"open", strXlogPath, NULL, NULL, SW_SHOW);
	return 0;
}

LRESULT CMainFrame::OnEditDebugset(WORD, WORD, HWND, BOOL&)
{
	CStringW strPath = helper::ExpandPath(BSP_DEBUGSET_INI);
	helper::WriteFileIfNotExist(strPath, MAKEINTRESOURCEW(IDR_DEBUGSET_INI), L"PDATA");
	::ShellExecuteW(m_hWnd, L"open", strPath, NULL, NULL, SW_SHOW);
	return 0;
}

LRESULT CMainFrame::OnViewDetail(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const LogInfo* pLi = GetSelectedLog();
	if (pLi)
	{
		m_infoDlg.ShowLogInfo(*pLi);
		m_infoDlg.ShowWindow(SW_SHOW);
	}

	return 0;
}

void CMainFrame::OnUserCommand(UINT /*uCode*/, int nID, HWND /*hWnd*/)
{
	switch(nID)
	{
	case ID_HELP:
		break;
	case ID_ABOUT:
		PostMessage(WM_SYSCOMMAND, ID_ABOUT, 0);
		break;
	case ID_EXPORT_ALL:
		Export(FALSE);
		break;
	case ID_EXPORT_FILTERED:
		Export(TRUE);
		break;
	}
}

void CMainFrame::UpdateWindowTitle()
{
	CStringW strTitle = L"日志查看器";

	if (ServiceHelper::GetLogCenter()->MonitoringPipe())
	{
		strTitle += L" - 正在监控";
	}
	else if (!GD.strXLogFile.IsEmpty())
	{
		strTitle += L" - ";
		strTitle += GD.strXLogFile;
	}

	SetWindowTextW(strTitle);
}

void CMainFrame::OnNewLog(const LogRange& range)
{
	bool bHasNewVisibleItem = false;
	for (UINT64 i = range.idmin; i < range.idmax; i++)
	{
		const LogInfo* li = ServiceHelper::GetLogCenter()->GetLog(i);
		if (FilterMatch(*li))
		{
			m_lvs.push_back(li->logid);
			bHasNewVisibleItem = true;
		}
	}

	if (bHasNewVisibleItem)
	{
		BOOL bAutoScroll = FALSE;
		if (m_list.GetSelectedIndex() == -1 || m_list.GetSelectedIndex() == m_list.GetItemCount() - 1)
		{
			bAutoScroll = TRUE;
		}

		ListView_SetItemCountEx(m_list.m_hWnd, m_lvs.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);

		if (bAutoScroll)
		{
			m_list.SelectItem(m_list.GetItemCount() - 1);
		}		
	}
}

LRESULT CMainFrame::OnMqInvoke(UINT /*uMsg*/, WPARAM wp, LPARAM lp, BOOL &/*bHandled*/)
{
	if (wp == MQCMD_LOADCONFIG)
	{
		CStringW* pStr = (CStringW*)lp;
		CConfig::Instance()->Load(*pStr);
		const config& cfg = CConfig::Instance()->GetConfig();

		DWORD dwStyle = m_list.GetExtendedListViewStyle();
		if (cfg.ui.list.show_gridline)
		{
			dwStyle |= LVS_EX_GRIDLINES;
		}
		else
		{
			dwStyle &= ~LVS_EX_GRIDLINES;
		}
		m_list.SetExtendedListViewStyle(dwStyle);

		delete pStr;
	}
	else if (wp == MQCMD_OPENXLOG)
	{
		OpenXLog(GD.strXLogFile);
	}

	return 0;
}

void CMainFrame::OpenXLog(LPCWSTR pszFileName)
{
	SendMessage(m_hWnd, WM_COMMAND, ID_STOP_MONITOR, 0);
	ClearAllLog();
	UpdateWindowTitle();

	// TODO decrypt遇到大文件会崩溃，目前没有decrypt的需求 所以先屏蔽
	// strFileName = helper::DecryptXLogFile(strFileName);
	ServiceHelper::GetLogCenter()->ConnectFile(pszFileName);
}

void CMainFrame::OnTimer(UINT nID)
{
	if (nID == IDT_UPDATE_TIMER)
	{
		UpdateStatusInfo();
	}
}

LogInfo* CMainFrame::GetLogInfo(int index)
{
	return ServiceHelper::GetLogCenter()->GetLog(m_lvs[static_cast<size_t>(index)]);
}

LogInfo* CMainFrame::GetSelectedLog()
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0)
	{
		return GetLogInfo(index);
	}
	return NULL;
}

void CMainFrame::ApplyFilter()
{
	const LogInfo *pLI = GetSelectedLog();

	m_lvs.clear();
	CLogCenter& lc = *ServiceHelper::GetLogCenter();
	lc.LockLog();

	LogRange range = lc.GetLogRange();

	int index = 0;
	for (UINT64 i = range.idmin; i < range.idmax; i++)
	{
		LogInfo* li = lc.GetLog(i);
		if (FilterMatch(*li))
		{
			if (li == pLI) index = static_cast<int>(m_lvs.size());
			m_lvs.push_back(li->logid);
		}
	}
	m_list.SetItemCount(static_cast<int>(m_lvs.size()));
	m_list.SelectItem(index);
}

LRESULT CMainFrame::OnNMDblclkList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pNMHDR;
	if (lpnmitem->iSubItem == 0)
	{
		PostMessage(WM_COMMAND, ID_TOGGLE_BOOKMARK);
		return 0;
	}

	const LogInfo *pLi = GetSelectedLog();
	if (pLi)
	{
		m_infoDlg.ShowLogInfo(*pLi);
		m_infoDlg.ShowWindow(SW_SHOW);
	}

	return 0;
}

LRESULT CMainFrame::OnNMCustomdrawList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		return CDRF_NOTIFYITEMDRAW;
	}
	if (pNMCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		int index = static_cast<int>(pNMCD->nmcd.dwItemSpec);
		if (index < m_list.GetItemCount())
		{
			const LogInfo* pInfo = GetLogInfo(index);
			hilighters_t& v = CConfig::Instance()->GetConfig().hls;
			for (hilighters_t::const_iterator it = v.begin(); it != v.end(); ++it)
			{
				if (it->f->meet(*pInfo))
				{
					pNMCD->clrText = it->d.color;
					pNMCD->clrTextBk = it->d.bkcolor;
					break;
				}
			}
			if (index == m_functionBegin || index == m_functionEnd)
			{
				pNMCD->clrTextBk = RGB(255, 224, 224);
			}
			if (m_searchText.GetLength() > 0)
			{
				if (helper::wcsistr(pInfo->item->log_content.c_str(), m_searchText))
				{
					//pNMCD->clrTextBk = RGB(255, 255, 0);
					return CDRF_NOTIFYSUBITEMDRAW;
				}
			}
		}
	}
	if (pNMCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT|CDDS_ITEM|CDDS_SUBITEM))
	{
		if (pNMCD->iSubItem == 6 && m_searchText.GetLength() > 0)
		{
			int index = static_cast<int>(pNMCD->nmcd.dwItemSpec);
			if (index < m_list.GetItemCount())
			{
				const LogInfo* pInfo = GetLogInfo(index);
				const wchar_t* str = pInfo->item->log_content.c_str();
				const wchar_t* pos = helper::wcsistr(str, m_searchText);
				if (pos)
				{
					std::wstring content;
					size_t prefixlen = m_cfg.ui.list.calldepth_sign.length() * pInfo->item->log_depth;
					content.reserve(pInfo->item->log_content.length() + prefixlen);
					for (size_t i = 0; i < pInfo->item->log_depth; i++)
					{
						content += m_cfg.ui.list.calldepth_sign;
					}
					content += pInfo->item->log_content;
					const wchar_t* dispstr = content.c_str();
					pos = dispstr + (pos - str) + prefixlen;

					CDCHandle dc(pNMCD->nmcd.hdc);
					CRect rect, rc;

					CRect rcLabel, rcIcon, rcBounds;
					m_list.GetItemRect(index, &rcLabel, LVIR_LABEL);
					m_list.GetItemRect(index, &rcIcon, LVIR_ICON);
					m_list.GetItemRect(index, &rcBounds, LVIR_BOUNDS);

					m_list.GetSubItemRect(index, pNMCD->iSubItem, LVIR_BOUNDS, &rect);

					int margin = rcLabel.left - rcBounds.left - rcIcon.Width();
					rect.InflateRect(-2 * GetSystemMetrics(SM_CXBORDER) - margin, 0);

					if (index == m_list.GetSelectedIndex())
					{
						dc.FillSolidRect(&rect, RGB(224, 224, 244));
					}

					rc = rect;
					if (pos > dispstr)
					{
						if (rc.Width() > 0)
						{
							dc.SetBkMode(TRANSPARENT);
							dc.DrawText(dispstr, pos-dispstr, &rc, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
							dc.DrawText(dispstr, pos-dispstr, &rc, DT_CALCRECT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
						}
						rect.left = rc.right;
					}
					
					rc = rect;
					if (rc.Width() > 0)
					{
						dc.SetBkMode(OPAQUE);
						dc.SetBkColor(RGB(255,255,0));
						dc.DrawText(pos, m_searchText.GetLength(), &rc, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
						dc.DrawText(pos, m_searchText.GetLength(), &rc, DT_CALCRECT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
					}
					rect.left = rc.right;

					rc = rect;
					if (rc.Width() > 0)
					{
						dc.SetBkMode(TRANSPARENT);
						dc.DrawText(pos + m_searchText.GetLength(), -1, &rc, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
						dc.DrawText(pos + m_searchText.GetLength(), -1, &rc, DT_CALCRECT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
					}
					rect.left = rc.right;

					return CDRF_SKIPDEFAULT;
				}
			}
		}
	}
	return CDRF_DODEFAULT;
}

LRESULT CMainFrame::OnNMRclickList(int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& /*bHandled*/)
{
	int index = m_list.GetSelectedIndex();
	if (index < 0)
	{
		return 0;
	}

	POINT pt;
	::GetCursorPos(&pt);
	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, ID_CLEAR_BEFORE_THIS, L"清除此条之前的所有日志(&D)");
	menu.AppendMenu(MF_STRING, ID_TOGGLE_BOOKMARK, L"设置/取消书签(&M)");
	

	menu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);

	return 0;
}

BOOL CMainFrame::PreTranslateMessage( MSG* pMsg )
{
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
	{
		return TRUE;
	}

	if (pMsg->message == WM_DROPFILES)
	{
		WCHAR filename[MAX_PATH];
		HDROP hDrop = (HDROP)pMsg->wParam;
		::DragQueryFile(hDrop, 0, filename, _countof(filename));
		GD.strXLogFile = filename;
		PostMessage(WM_MQINVOKE, MQCMD_OPENXLOG);
		::DragFinish(hDrop);
		return TRUE;
	}

	return FALSE;
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();

	bool bMonitoring = ServiceHelper::GetLogCenter()->MonitoringPipe();
	UIEnable(ID_START_MONITOR, !bMonitoring);
	UIEnable(ID_STOP_MONITOR, bMonitoring);
	UISetCheck(ID_VIEW_TOOLBAR, m_toolbar.IsWindowVisible());
	UISetCheck(ID_VIEW_STATUS_BAR, m_wndStatusbar.IsWindowVisible());
	UISetCheck(ID_SHOW_GRIDLINE, CConfig::Instance()->GetConfig().ui.list.show_gridline);

	return TRUE;
}

void CMainFrame::UpdateStatusInfo()
{
	LogRange r = ServiceHelper::GetLogCenter()->GetLogRange();

	StatusInfo info;
	info.log_count = static_cast<size_t>(r.idmax - r.idmin);
	info.filtered_log_count = m_lvs.size();
	info.thread_count = ServiceHelper::GetLogCenter()->source_count();

	const filter* f = CConfig::Instance()->GetConfig().log_filter;
	const quickfilter& qf = CConfig::Instance()->GetConfig().log_quickfilter;
	info.has_filter = f ||
		              qf.level > 0 ||
		              qf.tags.length() > 0 ||
					  qf.text.length() > 0;

	if (info != m_status)
	{
		m_status = info;
		m_wndStatusbar.SetPaneText(ID_PANE_LOGCOUNT, tp::cz(L"总数:%u", info.log_count));
		m_wndStatusbar.SetPaneText(ID_PANE_FILTERED_LOGCOUNT, tp::cz(L"显示:%u", info.filtered_log_count));
		m_wndStatusbar.SetPaneText(ID_PANE_WORKTHREAD_COUNT, tp::cz(L"日志源:%u", info.thread_count));
		HICON icon = info.has_filter? AtlLoadIconImage(IDI_FILTER, LR_DEFAULTCOLOR, 16, 16) : NULL;
		m_wndStatusbar.SetPaneIcon(ID_PANE_FILTERED_LOGCOUNT, icon);
	}
}

bool CMainFrame::FilterMatch(const LogInfo& li)
{
	const filter* f = CConfig::Instance()->GetConfig().log_filter;
	if (f && !f->meet(li)) return false;

	const quickfilter& qf = CConfig::Instance()->GetConfig().log_quickfilter;
	if (li.item->log_class < qf.level) return false;
	if (helper::wcsistr(li.item->log_content.c_str(), qf.text.c_str()) == NULL) return false;
	if (!helper::TagMatch(li.item->log_tags.c_str(), qf.tags.c_str())) return false;

	return true;
}

void CMainFrame::ClearAllLog()
{
	m_list.SetItemCount(0);
	m_lvs.clear();
	ServiceHelper::GetLogCenter()->ClearAllLog();
}

LRESULT CMainFrame::OnClearall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ClearAllLog();
	return 0;
}


LRESULT CMainFrame::OnAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal(m_hWnd);

	return 0;
}

LRESULT CMainFrame::OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShellExecute(NULL, L"open", L"http://db-win-dump00.db01.baidu.com:8180/bdlog/help", NULL, NULL, SW_SHOW);
	return 0;
}


LRESULT CMainFrame::OnEditFilter(WORD , WORD , HWND , BOOL& )
{
	m_filterDlg.CenterWindow(m_hWnd);
	m_filterDlg.Show();
	m_filterDlg.SetActiveWindow();
	
	return 0;
}

LRESULT CMainFrame::OnFileClose(WORD, WORD, HWND, BOOL&)
{
	SendMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFilterChange(UINT /*uMsg*/, WPARAM /*wp*/, LPARAM /*lp*/, BOOL& /*bHandled*/)
{
	ApplyFilter();
	return 0;
}

LRESULT CMainFrame::OnViewLogSourceInfo(WORD, WORD, HWND, BOOL&)
{
	lsds_t lsds;
	bdlog::lsi_vec_t src = ServiceHelper::GetLogCenter()->get_sources();
	CStringW strInfo;
	for (size_t i = 0; i < src.size(); i++)
	{
		LogSourceDispInfo lsdi;
		lsdi.joinTime = src[i].join_time;
		lsdi.pid = src[i].pid;
		lsdi.pname = src[i].process_name.c_str();
		lsdi.ppath = src[i].process_path.c_str();
		lsdi.logCount = 0; // TODO
		lsds.push_back(lsdi);
	}

	CLogSourceInfoDlg dlg;
	dlg.SetLogSourceInfo(lsds);
	dlg.DoModal();

	return 0;
}

LRESULT CMainFrame::OnViewSpecialPath(WORD, WORD, HWND, BOOL&)
{
	CSpecialPathDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnEditHilight(WORD, WORD, HWND, BOOL&)
{
	CHilighterManageDlg dlg;
	dlg.DoModal();
	m_list.RedrawWindow();
	return 0;
}

LRESULT CMainFrame::OnNMClick(int , LPNMHDR pNMHDR, BOOL& )
{
	return 0;
}

LRESULT CMainFrame::OnListStateChanged(int, LPNMHDR pNMHDR, BOOL&)
{
	LPNMLISTVIEW pInfo = (LPNMLISTVIEW)pNMHDR;
	if (pInfo->uNewState & LVIS_SELECTED)
	{
		int index = pInfo->iItem;
		int fb = m_functionBegin;
		int fe = m_functionEnd;
		UpdateFunctionPos(index, 1000);
		if (fb != m_functionBegin)
		{
			m_list.RedrawItems(fb, fb);
			m_list.RedrawItems(m_functionBegin, m_functionBegin);
		}
		if (fe != m_functionEnd)
		{
			m_list.RedrawItems(fe, fe);
			m_list.RedrawItems(m_functionEnd, m_functionEnd);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnListGetDispInfo(int /**/, LPNMHDR pNMHDR, BOOL& /**/)
{
	static const LPCWSTR cname[] = {L"", L"调试", L"信息", L"警告", L"错误", L"严重"};

	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem = &(pDispInfo)->item;

	int index = pItem->iItem;
	int subindex = pItem->iSubItem;
	const LogInfo* li = GetLogInfo(index);
	size_t textlen = static_cast<size_t>(pItem->cchTextMax);

	wchar_t cvtbuf[1024];
	if (pItem->mask & LVIF_TEXT)
	{
		switch(subindex)
		{
		case 1:
			swprintf_s(pItem->pszText, textlen, L"%I64u", li->logid);
			break;
		case 2:
			struct tm tt;
			localtime_s(&tt, &li->item->log_time_sec);
			wcsftime(cvtbuf, _countof(cvtbuf), L"%H:%M:%S", &tt);
			swprintf_s(pItem->pszText, textlen, L"%s.%06d", cvtbuf, li->item->log_time_msec);
			break;
		case 3:
			{
				const wchar_t* desc = ServiceHelper::GetLogPropertyDB()->GetLevelDesc(li->item->log_class);
				if (!desc)
				{
					_itow_s(static_cast<int>(li->item->log_class), cvtbuf, 10);
				}
				wcscpy_s(pItem->pszText, textlen, desc?desc:cvtbuf);
				break;
			}
		case 4:
			{
				if (!m_isHex)
				{
					StringCchPrintf(pItem->pszText, textlen, L"%s(%d)", li->item->log_process_name.c_str(), li->item->log_pid);
				}
				else
				{
					StringCchPrintf(pItem->pszText, textlen, L"%s(x%X)", li->item->log_process_name.c_str(), li->item->log_pid);
				}				
			}
			
			break;
		case 5:
			{
				if (!m_isHex)
				{
					StringCchPrintf(pItem->pszText, textlen, L"%d", li->item->log_tid);
				}
				else
				{
					StringCchPrintf(pItem->pszText, textlen, L"x%X", li->item->log_tid);
				}
			}			
			break;
		case 6:
			{
				size_t pos = 0;
				const std::wstring& sign = m_cfg.ui.list.calldepth_sign;
				for (size_t i = li->item->log_depth; i > 0; i--)
				{
					StringCchCopy(pItem->pszText + pos, textlen - pos, sign.c_str());
					pos += sign.length();
				}
				StringCchCopy(pItem->pszText + pos, textlen - pos, li->item->log_content.c_str());
			}
			break;
		}
	}
	if (pItem->mask & LVIF_IMAGE)
	{
//		if (li->marked)
		{
//			pItem->iImage = 0;
		}
	}

	return 0;
}

void CMainFrame::OnDestroy()
{
	SendMessage(WM_COMMAND, ID_STOP_MONITOR);
	ServiceHelper::GetLogCenter()->RemoveListener(this);
	::PostQuitMessage(0);
}

LRESULT CMainFrame::OnToggleBookmark(WORD, WORD, HWND, BOOL&)
{
// 	int index = m_list.GetSelectedIndex();
// 	LogInfo* li = GetSelectedLog();
// 	if (li)
// 	{
// 		li->marked = !li->marked;
// 		m_list.RedrawItems(index, index);
// 	}
	return 0;
}

LRESULT CMainFrame::OnFunctionJump(WORD, WORD, HWND, BOOL&)
{
	int index = m_list.GetSelectedIndex();
	if (index == m_functionBegin)
	{
		m_list.SelectItem(m_functionEnd);
	}
	else if (index == m_functionEnd)
	{
		m_list.SelectItem(m_functionBegin);
	}
	else
	{
		LogInfo* li = GetSelectedLog();
		if (!li || !IsFunctionLog(*li->item))
		{
			MessageBox(L"无法定位到函数头/尾，因为当前日志不是函数日志(标签中不包含function)", L"提示", MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(L"未找到函数头/尾，可能是当前的过滤规则隐藏的对应的日志记录，或者对应的日志记录已被删除", L"提示", MB_OK|MB_ICONINFORMATION);
		}
	}

	return 0;
}

LRESULT CMainFrame::OnNextBookmark(WORD, WORD, HWND, BOOL&)
{
// 	size_t index = static_cast<size_t>(m_list.GetSelectedIndex());
// 	for (size_t i = 1; i < m_lvs.size(); i++)
// 	{
// 		size_t pos = (index + i) % m_lvs.size();
// 		if (m_lvs[pos]->marked)
// 		{
// 			m_list.SelectItem(static_cast<int>(pos));
// 			break;
// 		}
// 	}
	return 0;
}

LRESULT CMainFrame::OnPrevBookmark(WORD, WORD, HWND, BOOL&)
{
// 	size_t index = static_cast<size_t>(m_list.GetSelectedIndex());
// 	for (size_t i = 1; i < m_lvs.size(); i++)
// 	{
// 		size_t pos = (index + m_lvs.size() - i) % m_lvs.size();
// 		if (m_lvs[pos]->marked)
// 		{
// 			m_list.SelectItem(static_cast<int>(pos));
// 			break;
// 		}
// 	}
	return 0;
}

LRESULT CMainFrame::OnClearBeforeThisLog(WORD, WORD, HWND, BOOL&)
{
	int index = m_list.GetSelectedIndex();
	if (index < 0) return 0;
	
	LogInfo* li = GetLogInfo(index);
	UINT64 logIndex = li->logid;
	m_lvs.erase(m_lvs.begin(), m_lvs.begin() + index);

	ServiceHelper::GetLogCenter()->ClearOldLog(logIndex);
	
	m_list.SetItemCount(static_cast<int>(m_lvs.size()));
	m_list.SelectItem(0);

	return 0;
}

void CMainFrame::OnMenuSelect(UINT nID, UINT flags, HMENU hMenu)
{
	if (flags == 0xFFFF && hMenu == NULL)
	{
		m_wndStatusbar.SetPaneText(0, L"");
	}
	else
	{
		WCHAR buffer[1024] = L"";
		AtlLoadString(nID, buffer, _countof(buffer));
		LPWSTR p = wcschr(buffer, L'\n');
		if (p != NULL) *p = L'\0';
		m_wndStatusbar.SetPaneText(0, buffer);
	}
}

LRESULT CMainFrame::OnShowGridline(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DWORD style = m_list.GetExtendedListViewStyle();
	if (style & LVS_EX_GRIDLINES)
	{
		style &= ~LVS_EX_GRIDLINES;
		CConfig::Instance()->GetConfig().ui.list.show_gridline = false;
	}
	else
	{
		style |= LVS_EX_GRIDLINES;
		CConfig::Instance()->GetConfig().ui.list.show_gridline = true;
	}
	m_list.SetExtendedListViewStyle(style);
	return 0;
}

void CMainFrame::OnContextMenu(HWND hWnd, CPoint pt)
{
	if (hWnd == m_hWnd)
	{
		SetMsgHandled(FALSE);
	}
	else if (hWnd == m_list.m_hWnd)
	{
		int index = m_list.GetSelectedIndex();
		if (index < 0)
		{
			return;
		}

		CPoint ptClient;
		if (pt.x == -1 && pt.y == -1)
		{
			CRect rcBounds;
			m_list.GetItemRect(index, &rcBounds, LVIR_BOUNDS);
			ptClient.x = (rcBounds.left + rcBounds.right) / 2;
			ptClient.y = rcBounds.bottom;
			pt = ptClient;
			m_list.ClientToScreen(&pt);
		}
		else
		{
			ptClient = pt;
			m_list.ScreenToClient(&ptClient);
		}

		CRect rcHdr;
		CHeaderCtrl hdrCtrl = m_list.GetHeader();
		hdrCtrl.GetWindowRect(&rcHdr);
		if (ptClient.y > rcHdr.Height())
		{
			LogInfo* li = GetLogInfo(index);

			CMenu m1;
			m1.CreatePopupMenu();
			UINT i = ID_QUICKFILTER_INCLUDE_BEGIN;
			if (m_isHex)
			{
				m1.AppendMenu(MF_STRING, i++, formatstr(L"进程ID：%08X", li->item->log_pid));
				m1.AppendMenu(MF_STRING, i++, formatstr(L"线程ID：%08X", li->item->log_tid));
			}
			else
			{
				m1.AppendMenu(MF_STRING, i++, formatstr(L"进程ID：%u", li->item->log_pid));
				m1.AppendMenu(MF_STRING, i++, formatstr(L"线程ID：%u", li->item->log_tid));
			}			
			m1.AppendMenu(MF_STRING, i++, formatstr(L"进程名：%s", li->item->log_process_name.c_str()));
			m1.AppendMenu(MF_STRING, i++, formatstr(L"日志级别：%u", li->item->log_class));
			m1.AppendMenu(MF_STRING, i++, formatstr(L"日志标签：%s", li->item->log_tags.c_str()));
			m1.AppendMenu(MF_STRING, i++, formatstr(L"日志内容：%.*s", 30, li->item->log_content.c_str()));

			CMenu m2;
			m2.CreatePopupMenu();
			i = ID_QUICKFILTER_EXCLUDE_BEGIN;
			if (m_isHex)
			{
				m2.AppendMenu(MF_STRING, i++, formatstr(L"进程ID：x%08X", li->item->log_pid));
				m2.AppendMenu(MF_STRING, i++, formatstr(L"线程ID：x%08X", li->item->log_tid));
			}
			else
			{
				m2.AppendMenu(MF_STRING, i++, formatstr(L"进程ID：%u", li->item->log_pid));
				m2.AppendMenu(MF_STRING, i++, formatstr(L"线程ID：%u", li->item->log_tid));
			}			
			m2.AppendMenu(MF_STRING, i++, formatstr(L"进程名：%s", li->item->log_process_name.c_str()));
			m2.AppendMenu(MF_STRING, i++, formatstr(L"日志级别：%u", li->item->log_class));
			m2.AppendMenu(MF_STRING, i++, formatstr(L"日志标签：%s", li->item->log_tags.c_str()));
			m2.AppendMenu(MF_STRING, i++, formatstr(L"日志内容：%.*s", 30, li->item->log_content.c_str()));

			CMenu menu;
			menu.CreatePopupMenu();

			menu.AppendMenu(MF_STRING, ID_TOGGLE_BOOKMARK, L"设置/取消书签(&M)\tCtrl+F2");
			menu.AppendMenu(MF_STRING, ID_FUNCTION_JUMP, L"跳到函数头/尾(&F)");
			menu.AppendMenu(MF_SEPARATOR, 0U, L"");
			menu.AppendMenu(MF_POPUP, (UINT_PTR)m1.m_hMenu, L"上下文过滤：包含(&I)");
			// TODO: menu.AppendMenu(MF_POPUP, (UINT_PTR)m1.m_hMenu, L"上下文过滤：解除包含(&U)");
			menu.AppendMenu(MF_POPUP, (UINT_PTR)m2.m_hMenu, L"上下文过滤：排除(&E)");
			menu.AppendMenu(MF_SEPARATOR, 0U, L"");
			menu.AppendMenu(MF_STRING, ID_CLEAR_BEFORE_THIS, L"清除此条之前的所有日志(&C)");
			menu.AppendMenu(MF_SEPARATOR, 0U, L"");
			menu.AppendMenu(MF_STRING, ID_VIEW_DETAIL, L"详细信息(&D)");

			menu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
		}
	}
}

LRESULT CMainFrame::OnQuickFilterInclude(WORD , WORD nID, HWND , BOOL&)
{
	LogInfo* li = GetSelectedLog();
	if (li == NULL)
	{
		return 0;
	}

	component* child = NULL;
	switch (nID - ID_QUICKFILTER_INCLUDE_BEGIN)
	{
	case 0: // pid
		child = new logpid_filter(li->item->log_pid);
		break;
	case 1: // tid
		child = new logtid_filter(li->item->log_tid);
		break;
	case 2: // pname
		child = new logprocessname_filter(li->item->log_process_name.c_str());
		break;
	case 3: // class
		child = new logclass_filter(li->item->log_class, li->item->log_class);
		break;
	case 4: // tags
		child = new logtag_filter(li->item->log_tags.c_str());
		break;
	case 5: // content
		child = new logcontent_filter(li->item->log_content.c_str(), false);
		break;
	default:
		return 0;
	}

	filter*& f = CConfig::Instance()->GetConfig().log_filter;
	if (f == NULL)
	{
		f = dynamic_cast<filter*>(child);
	}
	else
	{
		logical_and_filter* andfilter = dynamic_cast<logical_and_filter*>(f);
		if (andfilter)
		{
			andfilter->add_child(child);
		}
		else
		{
			component* c = dynamic_cast<component*>(f);
			andfilter = new logical_and_filter;
			andfilter->add_child(c);
			andfilter->add_child(child);
			f = andfilter;
		}
	}

	ApplyFilter();
	return 0;
}

LRESULT CMainFrame::OnQuickFilterExclude(WORD , WORD nID, HWND , BOOL&)
{
	LogInfo* li = GetSelectedLog();
	if (li == NULL)
	{
		return 0;
	}

	component* child = NULL;
	switch (nID - ID_QUICKFILTER_EXCLUDE_BEGIN)
	{
	case 0: // pid
		child = new logpid_filter(li->item->log_pid);
		break;
	case 1: // tid
		child = new logtid_filter(li->item->log_tid);
		break;
	case 2: // pname
		child = new logprocessname_filter(li->item->log_process_name.c_str());
		break;
	case 3: // class
		child = new logclass_filter(li->item->log_class, li->item->log_class);
		break;
	case 4: // tags
		child = new logtag_filter(li->item->log_tags.c_str());
		break;
	case 5: // content
		child = new logcontent_filter(li->item->log_content.c_str(), false);
		break;
	default:
		return 0;
	}

	logical_not_filter* notfilter = new logical_not_filter;
	notfilter->add_child(child);
	child = notfilter;

	filter*& f = CConfig::Instance()->GetConfig().log_filter;
	if (f == NULL)
	{
		f = dynamic_cast<filter*>(child);
	}
	else
	{
		logical_and_filter* andfilter = dynamic_cast<logical_and_filter*>(f);
		if (andfilter)
		{
			andfilter->add_child(child);
		}
		else
		{
			component* c = dynamic_cast<component*>(f);
			andfilter = new logical_and_filter;
			andfilter->add_child(c);
			andfilter->add_child(child);
			f = andfilter;
		}
	}

	ApplyFilter();
	return 0;
}

LRESULT CMainFrame::OnSearchEditChange(WORD , WORD , HWND , BOOL& )
{
	m_edit.GetWindowText(m_searchText);
	m_list.RedrawWindow();
	return 0;
}

LRESULT CMainFrame::OnSearchNext(WORD, WORD, HWND, BOOL&)
{
	CStringW newText = m_searchDlg.GetSearchText();
	if (newText != m_searchText)
	{
		m_searchText = newText;
		m_list.RedrawWindow();
	}

	if (m_searchText.IsEmpty()) return 0;

	size_t index = static_cast<size_t>(m_list.GetSelectedIndex());
	for (size_t i = 1; i < m_lvs.size(); i++)
	{
		size_t pos = (index + i) % m_lvs.size();
		if (helper::wcsistr(GetLogInfo(static_cast<int>(pos))->item->log_content.c_str(), m_searchText))
		{
			m_list.SelectItem(static_cast<int>(pos));
			break;
		}
	}
	return 0;
}

LRESULT CMainFrame::OnSearchPrev(WORD, WORD, HWND, BOOL&)
{
	CStringW newText = m_searchDlg.GetSearchText();
	if (newText != m_searchText)
	{
		m_searchText = newText;
		m_list.RedrawWindow();
	}

	if (m_searchText.IsEmpty()) return 0;

	size_t index = static_cast<size_t>(m_list.GetSelectedIndex());
	for (size_t i = 1; i < m_lvs.size(); i++)
	{
		size_t pos = (index + m_lvs.size() - i) % m_lvs.size();
		if (helper::wcsistr(GetLogInfo(static_cast<int>(pos))->item->log_content.c_str(), m_searchText))
		{
			m_list.SelectItem(static_cast<int>(pos));
			break;
		}
	}
	return 0;
}

LRESULT CMainFrame::OnForceScrollDown( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	int nCount = m_list.GetItemCount();
	if (nCount > 0)
	{
		m_list.SelectItem(nCount - 1);
	}		
	m_list.EnsureVisible(m_list.GetItemCount() - 1, FALSE);
	return ERROR_SUCCESS;
}


LRESULT CMainFrame::OnHex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{	
	m_isHex = !m_isHex;
	UISetCheck(ID_HEX, m_isHex);
	m_list.InvalidateRect(NULL, TRUE);
	return 0;
}

LRESULT CMainFrame::OnSearch(WORD, WORD, HWND, BOOL&)
{
	if (!m_searchDlg.IsWindow())
	{
		m_searchDlg.Create(m_hWnd);
		m_searchDlg.CenterWindow(m_hWnd);
	}

	m_searchDlg.ShowWindow(SW_SHOW);
	m_searchDlg.SetActiveWindow();

	return 0;
}

bool CMainFrame::IsFunctionLog(const bdlog::logitem& item)
{
	return helper::TagMatch(item.log_tags.c_str(), L"function");
}

void CMainFrame::UpdateFunctionPos(int index, int lookuplimit)
{
// 	m_functionBegin = m_functionEnd = -1;
// 
// 	if (index < 0)
// 	{
// 		return;
// 	}
// 
// 	const bdlog::logitem* item = GetLogInfo(index)->item;
// 	if (!IsFunctionLog(*item))
// 	{
// 		m_functionBegin = m_functionEnd = -1;
// 		return;
// 	}
// 
// 	size_t len = m_lis.size();
// 	if (len == 0 || m_lis[0]->logid + len <= GetLogInfo(index)->logid)
// 	{
// 		return;
// 	}
// 
// 	int lis_index = static_cast<int>(GetLogInfo(index)->logid - m_lis[0]->logid);
// 
// 	if (item->log_content.length() > 0 && item->log_content[0] == L'}')
// 	{
// 		for (int i = lis_index - 1; i >= 0 && lookuplimit != 0; i--, lookuplimit--)
// 		{
// 			const bdlog::logitem* item2 = m_lis[i]->item;
// 			if (item2->log_pid == item->log_pid &&
// 				item2->log_tid == item->log_tid &&
// 				item2->log_depth == item->log_depth &&
// 				item2->log_content[0] != L'}' &&
// 				IsFunctionLog(*item2))
// 			{
// 				for (int j = index; j >= 0 && m_lvs[j]->logid >= m_lis[i]->logid; j--)
// 				{
// 					if (m_lvs[j]->logid == m_lis[i]->logid)
// 					{
// 						m_functionBegin = j;
// 						m_functionEnd = index;
// 						return;
// 					}
// 				}
// 			}
// 		}
// 	}
// 	else
// 	{
// 		for (int i = lis_index + 1; i < m_lis.size() && lookuplimit != 0; i++, lookuplimit--)
// 		{
// 			const bdlog::logitem* item2 = m_lis[i]->item;
// 			if (item2->log_pid == item->log_pid &&
// 				item2->log_tid == item->log_tid &&
// 				item2->log_depth == item->log_depth &&
// 				item2->log_content[0] == L'}' &&
// 				IsFunctionLog(*item2))
// 			{
// 				for (int j = index; j < m_lvs.size() && m_lvs[j]->logid <= m_lis[i]->logid; j++)
// 				{
// 					if (m_lvs[j]->logid == m_lis[i]->logid)
// 					{
// 						m_functionBegin = index;
// 						m_functionEnd = j;
// 						return;
// 					}
// 				}
// 			}
// 		}
// 	}
}

LRESULT CMainFrame::OnShowVisualLogic(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CVisLogicDlg::Show();
	return 0;
}
