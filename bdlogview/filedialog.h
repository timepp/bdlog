#pragma once

class COpenFileDialog : public WTL::CFileDialog
{
public:
	COpenFileDialog(LPCWSTR initdir = NULL, LPCWSTR filter = NULL, HWND parent = NULL)
		:CFileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST|OFN_EXPLORER|OFN_ENABLESIZING, filter, parent)
	{
		m_ofn.lpstrInitialDir = initdir;
	}
};

class CSaveFileDialog : public WTL::CFileDialog
{
public:
	CSaveFileDialog(LPCWSTR initdir = NULL, LPCWSTR ext = NULL, LPCWSTR filename = NULL, LPCWSTR filter = NULL, HWND parent = NULL)
		:CFileDialog(FALSE, ext, filename, OFN_OVERWRITEPROMPT|OFN_EXPLORER|OFN_ENABLESIZING, filter, parent)
	{
		m_ofn.lpstrInitialDir = initdir;
	}
};
