#pragma once

class CModlessDlgKeyProcessor : public CMessageMap, public CMessageFilter
{
private:
	template <typename T> struct GlobalData
	{
		static HWND m_hActiveDlg;
	};

public:
	virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, LRESULT& /*lResult*/, DWORD /*dwMsgMapID*/ = 0)
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

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return GlobalData<CModlessDlgKeyProcessor>::m_hActiveDlg
			&& ::IsDialogMessageW(GlobalData<CModlessDlgKeyProcessor>::m_hActiveDlg, pMsg);
	}

};

HWND CModlessDlgKeyProcessor::GlobalData<CModlessDlgKeyProcessor>::m_hActiveDlg;