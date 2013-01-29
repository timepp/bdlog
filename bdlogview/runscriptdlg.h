#pragma  once
#include "resource.h"
#include "scripthost.h"
#include "serviceid.h"
#include "modelesskeyprocessor.h"
#include "jsparser.h"

class CRunScriptDlg
	: public tp::service_impl<SID_ScriptDlg>
	, public CDialogImpl<CRunScriptDlg>
	, public CDialogResize<CRunScriptDlg>
	, public CModlessDlgKeyProcessor
{
public:
	enum {IDD = IDD_RUNSCRIPT};

	CRunScriptDlg();
	~CRunScriptDlg();
	void Show();

	BEGIN_MSG_MAP(CRunScriptDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_TIMER(OnTimer)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_BTN_LOADSCRIPT, OnLoadScript)
		COMMAND_ID_HANDLER(IDC_BTN_SAVESCRIPT, OnSaveScript)
		CHAIN_MSG_MAP(CModlessDlgKeyProcessor)
		CHAIN_MSG_MAP(CDialogResize<CRunScriptDlg>)
		COMMAND_HANDLER(IDC_EDIT_SCRIPT, EN_CHANGE, OnEnChangeEditScript)
		COMMAND_HANDLER(IDC_EDIT_SCRIPT, EN_UPDATE, OnEnUpdateEditScript)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CRunScriptDlg)
		DLGRESIZE_CONTROL(IDC_EDIT_SCRIPT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_BTN_SAVESCRIPT, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_BTN_LOADSCRIPT, DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

private:
	CFont m_editFont;
	CRichEditCtrl m_edit;
	jsparser::segments_t m_segments;
	jsparser m_parser;
	COLORREF m_colormap[256];
	CStringW m_code;

	void ReColorize();
	void AssignCodeColor(const jsparser::segments_t& segs);

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);
	void OnClose();
	LRESULT OnOK(WORD , WORD , HWND , BOOL& );
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& );
	LRESULT OnLoadScript(WORD , WORD , HWND , BOOL& );
	LRESULT OnSaveScript(WORD , WORD , HWND , BOOL& );
	void OnTimer(UINT_PTR id);
public:
	LRESULT OnEnChangeEditScript(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnUpdateEditScript(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

DEFINE_SERVICE(CRunScriptDlg, L"‘À––Script");
