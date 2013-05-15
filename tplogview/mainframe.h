#include "resource.h"

#include <atlsync.h>
#include "resource.h"
#include "tagselector.h"
#include "whitebkdlg.h"
#include "common.h"
#include "loginfodlg.h"
#include "searchdlg.h"
#include "droptarget.h"
#include "filtersetdlg.h"
#include <tplog_synctime.h>
#include <tplog_reader.h>
#include "quickfilterdlg.h"
#include "logcenter.h"
#include "logicvis.h"

#define WM_MQINVOKE WM_USER + 20
#define WM_USER_FIRST_TIME_RUN WM_USER + 22
#define ID_QUICKFILTER_INCLUDE_BEGIN         (WM_USER+0x1000)
#define ID_QUICKFILTER_INCLUDE_END           (WM_USER+0x10FF)
#define ID_QUICKFILTER_EXCLUDE_BEGIN         (WM_USER+0x1100)
#define ID_QUICKFILTER_EXCLUDE_END           (WM_USER+0x11FF)
#define ID_MRU_BEGIN                         (WM_USER+0x1200)
#define ID_MRU_END                           (WM_USER+0X127F)

struct StatusInfo
{
	size_t log_count;
	size_t filtered_log_count;
	size_t thread_count;
	bool has_filter;

	bool operator == (const StatusInfo& rhs)
	{
		return log_count == rhs.log_count 
			&& filtered_log_count == rhs.filtered_log_count 
			&& thread_count == rhs.thread_count
			&& has_filter == rhs.has_filter;
	}
	bool operator != (const StatusInfo& rhs)
	{
		return !(*this == rhs);
	}
};

class CMainFrame 
	: public CFrameWindowImpl<CMainFrame>
	, public CUpdateUI<CMainFrame>
	, public CMessageFilter
	, public CIdleHandler
	, public CLogCenterListener
{
public:

	CMainFrame();

	virtual void OnNewLog(const LogRange& range);

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_MSG_MAP(CMainFrame)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_MENUSELECT(OnMenuSelect)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		
		MESSAGE_HANDLER(WM_MQINVOKE, OnMqInvoke)
		MESSAGE_HANDLER(WM_FILTER_CHANGE, OnFilterChange)
		MESSAGE_HANDLER(WM_USER_FIRST_TIME_RUN, OnUserFirstTimeRun)

// 		if(uMsg == WM_NOTIFY && IDC_LIST == ((LPNMHDR)lParam)->idFrom)
// 		{
// 			UINT cd = ((LPNMHDR)lParam)->code;
// 			CStringW strInfo;
// 			strInfo.Format(L"list view notify: %15u, %15u\n", cd, UINT_MAX-cd);
// 			::OutputDebugStringW(strInfo);
// 		}

		NOTIFY_HANDLER(IDC_LIST, NM_DBLCLK, OnNMDblclkList)
		NOTIFY_HANDLER(IDC_LIST, NM_CUSTOMDRAW, OnNMCustomdrawList)
		//NOTIFY_HANDLER(IDC_LIST, NM_RCLICK, OnNMRclickList)
		NOTIFY_HANDLER(IDC_LIST, LVN_GETDISPINFO, OnListGetDispInfo)
//		NOTIFY_HANDLER(IDC_LIST, NM_CLICK, OnNMClick)
//		NOTIFY_HANDLER(IDC_LIST, LVN_ODSTATECHANGED, OnListStateChanged)
		NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListStateChanged)
//		NOTIFY_HANDLER(IDC_LIST, LVN_ODFINDITEM , OnListFindItem)
//		COMMAND_HANDLER(IDC_SEARCH_EDIT, EN_CHANGE, OnSearchEditChange)

		COMMAND_ID_HANDLER(ID_OPEN_XLOG, OnOpenXLog)
		COMMAND_ID_HANDLER(ID_OPEN_SM_LOG, OnOpenShareMemoryLog);
		COMMAND_ID_HANDLER(ID_EDIT_XLOG_INI, OnEditXlogIni)
		COMMAND_ID_HANDLER(ID_EDIT_DEBUGSET, OnEditDebugset)
		COMMAND_ID_HANDLER(ID_VIEW_DETAIL, OnViewDetail)
		COMMAND_ID_HANDLER(ID_STOP_MONITOR, OnStopMonitor)
		COMMAND_ID_HANDLER(ID_START_MONITOR, OnStartMonitor)
		COMMAND_ID_HANDLER(ID_RUN_SCRIPT, OnRunScript)
		COMMAND_ID_HANDLER(ID_CLEARALL, OnClearall)
		COMMAND_ID_HANDLER(ID_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(ID_HELP, OnHelp)
		COMMAND_ID_HANDLER(ID_EDIT_FILTER, OnEditFilter)
		COMMAND_ID_HANDLER(ID_EDIT_HLIGHT, OnEditHilight)
		COMMAND_ID_HANDLER(ID_FILE_CLOSE, OnFileClose)
		COMMAND_ID_HANDLER(ID_LOG_SOURCE_INFO, OnViewLogSourceInfo)
		COMMAND_ID_HANDLER(ID_VIEW_SPECIAL_PATH, OnViewSpecialPath)
		COMMAND_ID_HANDLER(ID_TOGGLE_BOOKMARK, OnToggleBookmark)
		COMMAND_ID_HANDLER(ID_FUNCTION_JUMP, OnFunctionJump)
		COMMAND_ID_HANDLER(ID_NEXT_BOOKMARK, OnNextBookmark)
		COMMAND_ID_HANDLER(ID_PREV_BOOKMARK, OnPrevBookmark)
		COMMAND_ID_HANDLER(ID_CLEAR_BEFORE_THIS, OnClearBeforeThisLog)
		COMMAND_ID_HANDLER(ID_SEARCH_NEXT, OnSearchNext)
		COMMAND_ID_HANDLER(ID_SEARCH_PREV, OnSearchPrev)
		COMMAND_ID_HANDLER(ID_SEARCH, OnSearch)
		COMMAND_ID_HANDLER(ID_OPTION, OnOption)

		COMMAND_ID_HANDLER(ID_TOGGLE_TOOLBAR, OnViewToolbar)
		COMMAND_ID_HANDLER(ID_TOGGLE_STATUSBAR, OnViewStatusbar)
		COMMAND_ID_HANDLER(ID_VIEW_MENUBAR, OnViewMenubar)
		COMMAND_ID_HANDLER(ID_VIEW_SEARCHBAR, OnViewSearchbar)
		COMMAND_ID_HANDLER(ID_SHOW_GRIDLINE, OnShowGridline)
		COMMAND_ID_HANDLER(ID_FORCEAUTOSCROLL, OnForceScrollDown)
		COMMAND_ID_HANDLER(ID_HEX, OnHex)
		COMMAND_ID_HANDLER(ID_SHOW_ABS_TIME, OnShowAbsTime)
		COMMAND_ID_HANDLER(ID_SET_TIME_BASE, OnSetTimeBase)
		COMMAND_ID_HANDLER(ID_SHOW_VISUAL_LOGIC, OnShowVisualLogic)
		COMMAND_ID_HANDLER(ID_CHECK_UPDATE, OnCheckUpdate)
		COMMAND_RANGE_HANDLER(ID_QUICKFILTER_INCLUDE_BEGIN, ID_QUICKFILTER_INCLUDE_END, OnQuickFilterInclude)
		COMMAND_RANGE_HANDLER(ID_QUICKFILTER_EXCLUDE_BEGIN, ID_QUICKFILTER_EXCLUDE_END, OnQuickFilterExclude)
		COMMAND_RANGE_HANDLER(ID_MRU_BEGIN, ID_MRU_END, OnOpenMRU)
		MSG_WM_COMMAND(OnUserCommand)

		
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
		
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_TOGGLE_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TOGGLE_STATUSBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_MENUBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_SEARCHBAR, UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_START_MONITOR, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_STOP_MONITOR, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_SHOW_GRIDLINE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_HEX, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

private:
	void FilteredAppendNewLogToList();
	void UpdateWindowTitle(LPCWSTR pszFileName = NULL);
	void ApplyFilter();
	LogInfo* GetLogInfo(int index);
	LogInfo* GetSelectedLog();
	void OpenXLog(LPCWSTR pszFileName);
	void OpenShareMemory(LPCWSTR name);
	void Export(BOOL bFilter); 
	void ClearAllLog();
	void UpdateStatusInfo();
	bool FilterMatch(const LogInfo& li);
	bool EnablePipeDevice(bool bEnable);

	void CreateReBar();
	void CreateStatusBar();
	void CreateList();

	void UpdateUI();
	void UpdateFunctionPos(int index, int lookuplimit);

	// 判断是否是一条函数日志
	bool IsFunctionLog(const tplog::logitem& item);
	// 取某一条日志在当前视图的索引
	int GetLogIndex(UINT64 logid);

	void UpdateMRU();

private:
	LRESULT OnCreate(LPCREATESTRUCTW cs);
	void OnDestroy();
	void OnTimer(UINT nID);
	void OnMenuSelect(UINT nID, UINT flags, HMENU hMenu);
	void OnContextMenu(HWND hWnd, CPoint pt);
	void OnSysCommand(UINT id, CPoint pt);
	LRESULT OnMqInvoke(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled);
	LRESULT OnFilterChange(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled);
	LRESULT OnUserFirstTimeRun(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled);
	LRESULT OnNMDblclkList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawList(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnNMRclickList(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnListGetDispInfo(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnListStateChanged(int, LPNMHDR pNMHDR, BOOL&);
	LRESULT OnNMClick(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnListFindItem(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnSearchEditChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOpenXLog(WORD, WORD, HWND, BOOL&);
	LRESULT OnOpenShareMemoryLog(WORD, WORD, HWND, BOOL&);
	LRESULT OnFileClose(WORD, WORD, HWND, BOOL&);
	LRESULT OnViewLogSourceInfo(WORD, WORD, HWND, BOOL&);
	LRESULT OnViewSpecialPath(WORD, WORD, HWND, BOOL&);
	LRESULT OnEditXlogIni(WORD, WORD, HWND, BOOL&);
	LRESULT OnEditDebugset(WORD, WORD, HWND, BOOL&);
	LRESULT OnToggleBookmark(WORD, WORD, HWND, BOOL&);
	LRESULT OnFunctionJump(WORD, WORD, HWND, BOOL&);
	LRESULT OnNextBookmark(WORD, WORD, HWND, BOOL&);
	LRESULT OnPrevBookmark(WORD, WORD, HWND, BOOL&);
	LRESULT OnSearchNext(WORD, WORD, HWND, BOOL&);
	LRESULT OnSearchPrev(WORD, WORD, HWND, BOOL&);
	LRESULT OnSearch(WORD, WORD, HWND, BOOL&);
	LRESULT OnRunScript(WORD, WORD, HWND, BOOL&);
	LRESULT OnClearBeforeThisLog(WORD, WORD, HWND, BOOL&);
	LRESULT OnViewDetail(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnQuickFilterInclude(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnQuickFilterExclude(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOpenMRU(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStopMonitor(WORD, WORD, HWND, BOOL&);
	LRESULT OnStartMonitor(WORD, WORD, HWND, BOOL&);
	void OnUserCommand(UINT uCode, int nID, HWND hWnd);

private:
	// childs
	CToolBarCtrl m_toolbar;
	CCommandBarCtrl m_cmdbar;
	CReBarCtrl m_rebar;
	CImageList m_toolbarImageList;
	CQuickFilterDlg m_qfdlg;
	CEdit m_edit;
	bool m_bShowToolbar;
	CFont m_editFont;

	CMultiPaneStatusBarCtrl m_wndStatusbar;
	bool m_bShowStatusbar;
	CIcon m_filterIcon;

	CListViewCtrl m_list;
	CImageList m_listImageList;
	CFont m_listFont;

	CIcon m_filterIndicator; // 右下角显示的漏斗ICON，提示现在是否有过滤情况

	CTagSelectWnd m_tagSelector;

	// 过滤
	typedef std::vector<UINT64> FilterMap;
	FilterMap m_lvs;
	CFilterSetDlg m_filterDlg;

	// 查找
	CStringW m_searchText;
	size_t m_searchPos;
	CSearchDlg m_searchDlg;

	int m_functionBegin;
	int m_functionEnd;

	// 书签
	std::set<UINT64> m_bookmarks;

	// MRU
	CMenuHandle m_mru;

	// misc
	CLogInfoDlg m_infoDlg;
	StatusInfo m_status;
	CLogAccurateTime m_lat;
	bool m_isHex;
	bool m_showAbsTime;

	accutime m_relativeTimeBase;

	const config& m_cfg;
public:
	LRESULT OnClearall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditHilight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowGridline(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewMenubar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewSearchbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnForceScrollDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowAbsTime(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSetTimeBase(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowVisualLogic(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCheckUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
