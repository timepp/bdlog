#pragma once

#include <map>
#include "uipublic.h"

// 让对话框和文本有白色背景（在XP下对话框稍好看一些）

class CColoredDlgImpl
{
public:
	BEGIN_MSG_MAP(CColoredDlgImpl)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORSCROLLBAR, OnCtlColorScrollbar)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColorEdit)
	END_MSG_MAP()

	CColoredDlgImpl()
	{
		m_hTargetWnd = NULL;
		m_brs = NULL;
		m_own_brs = FALSE;
	}

	void SetHWND(HWND hWnd)
	{
		m_hTargetWnd = hWnd;
	}
	void SetBkColor(COLORREF cr)
	{
		DeleteBrush();
		m_brs = ::CreateSolidBrush(cr);
		m_own_brs = TRUE;
	}
	void SetBkSysColor(int index)
	{
		DeleteBrush();
		m_brs = ::GetSysColorBrush(index);
		m_own_brs = FALSE;
	}
	void SetStockBrush(int index)
	{
		DeleteBrush();
		m_brs = (HBRUSH)::GetStockObject(index);
		m_own_brs = FALSE;
	}

	void SetStaticTextColor(UINT id, COLORREF cr)
	{
		m_scm[id] = cr;
	}

protected:
	void DeleteBrush()
	{
		if (m_own_brs && m_brs)
		{
			::DeleteObject(m_brs);
		}
		m_brs = NULL;
	}

	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!m_brs) bHandled = FALSE;
		return (LRESULT)m_brs;
	}
	LRESULT OnCtlColorScrollbar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnCtlColorDlg(uMsg, wParam, lParam, bHandled);
	}
	LRESULT OnCtlColorEdit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnCtlColorDlg(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		UINT id = (UINT)::GetDlgCtrlID((HWND)lParam);
		static_color_map_t::const_iterator it = m_scm.find(id);

		if (it == m_scm.end() && !m_brs)
		{
			bHandled = FALSE;
			return 0;
		}

		LRESULT ret;
		if (!m_brs)
		{
			ret = ::DefWindowProc(m_hTargetWnd, uMsg, wParam, lParam);
		}
		else
		{
			ret = (LRESULT)m_brs;
		}

		if (it != m_scm.end())
		{
			CDCHandle dc((HDC)wParam);
			dc.SetTextColor(it->second);
		}

		return ret;
	}
	
	HWND m_hTargetWnd;

	HBRUSH m_brs;
	BOOL m_own_brs;

	typedef std::map<UINT, COLORREF> static_color_map_t;
	static_color_map_t m_scm;
};
