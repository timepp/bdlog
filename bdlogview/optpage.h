#pragma once

#include "resource.h"
#include "colorselector.h"

class COptPage
{
public:
	virtual void Save() = 0;
};

class COptPage_PerfMark 
	: public CDialogImpl<COptPage_PerfMark>
	, public COptPage
{
public:
	enum {IDD = IDD_OPT_PERFMARK};

	virtual void Save();

private:
	BEGIN_MSG_MAP(COptPage_PerfMark)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);

private:
	CColorSelector m_cs;
};

class COptPage_System 
	: public CDialogImpl<COptPage_System>
	, public COptPage
{
public:
	enum {IDD = IDD_OPT_SYSTEM};

	virtual void Save();

private:
	BEGIN_MSG_MAP(COptPage_System)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/);

private:

};
