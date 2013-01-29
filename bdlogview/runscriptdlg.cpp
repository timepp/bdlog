#include "stdafx.h"
#include "runscriptdlg.h"
#include "filedialog.h"
#include "config.h"
#include "helper.h"
#include <algorithm>

#define IDT_UPDATE_COLOR 1001

enum UserCodeType
{
	ctApp = jsparser::ctMax + 1,
	ctFunction,
};

CRunScriptDlg::CRunScriptDlg()
{
	Create(NULL);
	m_parser.add_user_keyword(L"application", ctApp);
	std::fill(m_colormap, m_colormap + _countof(m_colormap), 0);
	m_colormap[jsparser::ctBlockComment] = RGB(144, 192, 144);
	m_colormap[jsparser::ctLineComment] = RGB(144, 144, 144);
	
	m_colormap[jsparser::ctKeywordDecl] = RGB(0, 0, 255);
	m_colormap[jsparser::ctKeywordLoop] = RGB(0, 0, 255);
	m_colormap[jsparser::ctKeywordFlow] = RGB(0, 0, 255);
	m_colormap[jsparser::ctKeywordConst] = RGB(0, 0, 255);
	m_colormap[jsparser::ctKeywordBuiltin] = RGB(0, 0, 255);

	m_colormap[jsparser::ctStringLiteral] = RGB(0, 128, 0);
	m_colormap[jsparser::ctNumberLiteral] = RGB(224, 0, 0);

	m_colormap[ctApp] = RGB(0, 64, 255);
}

CRunScriptDlg::~CRunScriptDlg()
{
	if (IsWindow())
	{
		DestroyWindow();
	}
}

void CRunScriptDlg::Show()
{
	ShowWindow(SW_SHOW);
	SetActiveWindow();
}

LRESULT CRunScriptDlg::OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
{
	DlgResize_Init(true);
//	m_editFont.CreatePointFont(160, L"Consolas", NULL, false, false);
	m_edit = GetDlgItem(IDC_EDIT_SCRIPT);
	m_edit.SetEventMask(ENM_CHANGE);
//	GetDlgItem(IDC_EDIT_SCRIPT).SetFont(m_editFont);
	CHARFORMAT cf = {};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE|CFM_SIZE;
	wcsncpy_s(cf.szFaceName, L"Consolas", _TRUNCATE);
	cf.yHeight = 300;
	m_edit.SetDefaultCharFormat(cf);
	CenterWindow(GetParent());

	return 0;
}

void CRunScriptDlg::OnClose()
{
	ShowWindow(SW_HIDE);
}

LRESULT CRunScriptDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	CStringW code;
	GetDlgItemText(IDC_EDIT_SCRIPT, code);
	SERVICE(CScriptHost)->RunScript(code);
	return 0;
}

LRESULT CRunScriptDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	OnClose();
	return 0;
}

LRESULT CRunScriptDlg::OnSaveScript(WORD , WORD , HWND , BOOL& )
{
	std::wstring& path = CConfig::Instance()->GetConfig().ui.savedpath.script;
	CSaveFileDialog dlg(path.c_str(), L"js", L"script.js", L"日志查看器脚本(*.js)\0*.js\0所有文件(*.*)\0*.*\0\0");
	if (dlg.DoModal() == IDOK)
	{
		CStringW content;
		GetDlgItemText(IDC_EDIT_SCRIPT, content);
		helper::SaveTextContentToFile(dlg.m_szFileName, content);
		path = helper::GetDir(dlg.m_szFileName);
	}
	return 0;
}

LRESULT CRunScriptDlg::OnLoadScript(WORD , WORD , HWND , BOOL& )
{
	std::wstring& path = CConfig::Instance()->GetConfig().ui.savedpath.script;
	COpenFileDialog dlg(path.c_str(), L"日志查看器脚本(*.js)\0*.js\0所有文件(*.*)\0*.*\0\0");
	if (dlg.DoModal() == IDOK)
	{
		CStringW content = helper::GetTextFileContent(dlg.m_szFileName);
		m_edit.SetWindowTextW(content);
		path = helper::GetDir(dlg.m_szFileName);
//		ReColorize();
//		m_edit.RedrawWindow();
	}
	return 0;
}

void CRunScriptDlg::ReColorize()
{
	CStringW strCode;
	m_edit.GetWindowTextW(strCode);
	// FIXME 为什么richedit返出来的text包含有\r???
	strCode.Remove(L'\r');

	if (strCode == m_code)
	{
		// 文本没有变化
		return;
	}

	jsparser::segments_t segs;
	jsparser::segments_t diff;
	m_parser.parse(strCode, segs);

	// 根据文本变化调整m_segments
	CHARRANGE cr;
	m_edit.GetSel(cr);
	int delta = m_code.GetLength() - strCode.GetLength();
	for (jsparser::segments_t::iterator it = m_segments.begin(); it != m_segments.end();)
	{
		if (it->range.begin >= cr.cpMin)
		{
			it->range.begin -= delta;
			it->range.end -= delta;
			// 如果偏移后， 落到插入点左边， 则删除区段
			if (it->range.begin < cr.cpMin)
			{
				it = m_segments.erase(it);
				continue;
			}
		}
		++it;
	}

	std::set_difference(segs.begin(), segs.end(), m_segments.begin(), m_segments.end(), std::back_inserter(diff));
	AssignCodeColor(diff);
	m_segments = segs;
	m_code = strCode;
}

void CRunScriptDlg::AssignCodeColor(const jsparser::segments_t& segs)
{
	CHARFORMAT cf = {};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR|CFM_FACE;
	wcsncpy_s(cf.szFaceName, L"Consolas", _TRUNCATE);
	CHARRANGE cr;
	POINT pt;
	m_edit.GetScrollPos(&pt);
	m_edit.GetSel(cr);

	m_edit.LockWindowUpdate();

	for (jsparser::segments_t::const_iterator it = segs.begin(); it != segs.end(); ++it)
	{
		cf.crTextColor = m_colormap[it->type];
		m_edit.SetSel(it->range.begin, it->range.end);
		m_edit.SetSelectionCharFormat(cf);
	}

	m_edit.SetSel(cr);
	m_edit.SetScrollPos(&pt);

	m_edit.LockWindowUpdate(FALSE);
}


LRESULT CRunScriptDlg::OnEnChangeEditScript(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return TRUE;
}


LRESULT CRunScriptDlg::OnEnUpdateEditScript(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the __super::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	SetTimer(IDT_UPDATE_COLOR, 0);

	return 0;
}

void CRunScriptDlg::OnTimer(UINT_PTR id)
{
	if (id == IDT_UPDATE_COLOR)
	{
		KillTimer(id);
		ReColorize();
	}
}
