#pragma once

class CSMessageProvider
{
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& )
	{
		if (uMsg == WM_ACTIVATE)
		{
			if (wParam == 0)
			{
				GlobalData<CModlessDlgKeyProcessor>::m_hActiveDlg = NULL;
			}
			else
			{
				GlobalData<CModlessDlgKeyProcessor>::m_hActiveDlg = hWnd;
			}
		}

		return FALSE;
	}
};

