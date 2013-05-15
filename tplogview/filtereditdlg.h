#pragma once

#include "resource.h"
#include <tplib/include/compositetreectrl.h>
#include "filter.h"
#include <wtl/atlctrlx.h>

class CFilterEditor
{
public:
	virtual bool SetFilter(const filter* f) = 0;
	virtual filter* GetFilter() const = 0;
	virtual void Show() = 0;
	virtual void Hide() = 0;
	virtual ~CFilterEditor() {}
};

class CSimpleFilterEditDlg 
	: public CDialogImpl<CSimpleFilterEditDlg>
	, public CDialogResize<CSimpleFilterEditDlg>
	, public CFilterEditor
{
public:
	enum {IDD = IDD_FILTER_BASIC};

	BEGIN_MSG_MAP(CSimpleFilterEditDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CDialogResize<CSimpleFilterEditDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CSimpleFilterEditDlg)
		DLGRESIZE_CONTROL(IDC_EDIT_LOGCONTENT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_LOGTAG, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_PROCESSNAME, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_PID, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_TID, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_STATIC_BADNODE, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_BTN_SELECT_TAG, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_BTN_SELECT_PROCESS_NAME, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_BTN_SELECT_PROCESS_ID, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

public:
	CSimpleFilterEditDlg();
	virtual bool SetFilter(const filter* f);
	virtual filter* GetFilter() const;
	virtual void Show() { ShowWindow(SW_SHOW); }
	virtual void Hide() { ShowWindow(SW_HIDE); }

private:
	bool SetConcreteFilter(const component* c);
	void ShowAllChild(int showcmd);
	void ResetControls();
	filter* m_filter;

private:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class CGraphicFilterEditDlg
	: public CDialogImpl<CGraphicFilterEditDlg>
	, public CFilterEditor
{
public:
	enum {IDD = IDD_FILTER_TREE};

	BEGIN_MSG_MAP(CSimpleFilterEditDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	virtual bool SetFilter(const filter* f);
	virtual filter* GetFilter() const;
	virtual void Show() { ShowWindow(SW_SHOW); }
	virtual void Hide() { ShowWindow(SW_HIDE); }

private:
	CCompositeTreeCtrl m_tree;
	CImageList m_imgList;

private:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class CAdvancedFilterEditDlg
	: public CDialogImpl<CAdvancedFilterEditDlg>
	, public CFilterEditor
{
public:
	CAdvancedFilterEditDlg();

	enum {IDD = IDD_FILTER_ADVANCED };

	BEGIN_MSG_MAP(CAdvancedFilterEditDlg)
	END_MSG_MAP()

	virtual bool SetFilter(const filter* f);
	virtual filter* GetFilter() const;
	virtual void Show() { ShowWindow(SW_SHOW); }
	virtual void Hide() { ShowWindow(SW_HIDE); }

private:
	filter* m_filter;
};

#define ID_PAGE_BEGIN 1000
#define ID_PAGE_END 1100
#define ID_IMPORT_FILTER 1101
#define ID_EXPORT_FILTER 1102
#define ID_SIMPLIFY 1103

class CFilterEditDlg 
	: public CDialogImpl<CFilterEditDlg>
	, public CDialogResize<CFilterEditDlg>
{
public:
	enum {IDD = IDD_FILTER_EDIT};

	BEGIN_MSG_MAP(CFilterEditDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)

		COMMAND_RANGE_HANDLER(ID_PAGE_BEGIN, ID_PAGE_END, OnPage)
		
		COMMAND_ID_HANDLER(ID_IMPORT_FILTER, OnImport)
		COMMAND_ID_HANDLER(ID_EXPORT_FILTER, OnExport)
		COMMAND_ID_HANDLER(ID_SIMPLIFY, OnSimplify)

		CHAIN_MSG_MAP(CDialogResize<CFilterEditDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CFilterEditDlg)
		DLGRESIZE_CONTROL(IDC_STATIC_SUBDLG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	virtual bool SetFilter(const filter* f);
	virtual filter* GetFilter() const;

	void SetDefaultPage(int page);
	int GetActivePage() const;

	CFilterEditDlg(std::wstring& defaultFilterDir);

private:
	void ResizeChildDialogs();
	void SelectPage(int index);
	void UpdateFilter(int index);
	CFilterEditor* GetActiveEditor() const;
	CFilterEditor* GetEditor(int index) const;

private:
	CSimpleFilterEditDlg m_dlgSimple;
	CGraphicFilterEditDlg m_dlgGraphic;
	CAdvancedFilterEditDlg m_dlgAdvanced;
	CToolBarCtrl m_toolbar;
	CImageList m_tbImg;

	component* m_filter;
	std::vector<CFilterEditor*> m_editors;
	int m_activePage;
	int m_defaultPage;

	std::wstring& m_defaultFilterDir;

public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSimplify(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
