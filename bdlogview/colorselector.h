#pragma once

class CColorSelector : public CWindowImpl<CColorSelector, CStatic>
{
public:
	void SetColor(COLORREF cr)
	{
		m_color = cr;
		InvalidateRect(NULL);
	}
	COLORREF GetColor() const
	{
		return m_color;
	}

	CColorSelector() : m_color(0)
	{
	}

public:
	BEGIN_MSG_MAP(CColorSelector)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	END_MSG_MAP()

private:
	COLORREF m_color;

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}
	LRESULT OnPaint(UINT , WPARAM , LPARAM , BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);

		CRect rc;
		GetClientRect(&rc);
		
		int val = dc.SaveDC();

		dc.SelectPen((HPEN)::GetStockObject(BLACK_PEN));
		dc.SelectBrush((HBRUSH)::GetStockObject(WHITE_BRUSH));
		dc.Rectangle(&rc);

		rc.DeflateRect(2, 2, 2, 2);
		dc.FillSolidRect(&rc, m_color);

		dc.RestoreDC(val);
		return 1;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CColorDialog dlg(m_color);
		if (dlg.DoModal(GetParent()) == IDOK)
		{
			SetColor(dlg.GetColor());
		}
		
		return 0;
	}
};