#include "stdafx.h"

#include "filtereditdlg.h"
#include "filtercreator.h"
#include "filedialog.h"
#include "helper.h"

CFilterEditDlg::CFilterEditDlg(std::wstring& defaultFilterDir)
: m_defaultFilterDir(defaultFilterDir)
{
	m_defaultPage = 0;
}

void CFilterEditDlg::SetDefaultPage(int page)
{
	m_defaultPage = page;
}

int CFilterEditDlg::GetActivePage() const
{
	return m_activePage;
}

LRESULT CFilterEditDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CDialogResize<CFilterEditDlg>::DlgResize_Init(false, false);

	m_tbImg.Create(16, 16, ILC_COLOR32, 5, 5);
	m_tbImg.AddIcon(AtlLoadIcon(IDI_SIMPLE));
	m_tbImg.AddIcon(AtlLoadIcon(IDI_TREE));
	m_tbImg.AddIcon(AtlLoadIcon(IDI_ADVANCED));
	m_tbImg.AddIcon(AtlLoadIcon(IDI_OPEN));
	m_tbImg.AddIcon(AtlLoadIcon(IDI_SAVE));
	m_tbImg.AddIcon(AtlLoadIcon(IDI_SIMPLIFY));

	CRect rc(0, 0, 400, 16);
	m_toolbar.Create(m_hWnd, rc, NULL, WS_CHILD|WS_VISIBLE|TBSTYLE_FLAT|TBSTYLE_TOOLTIPS|CCS_NODIVIDER);
	m_toolbar.SendMessage(CCM_SETVERSION, (WPARAM) 6, 0);
	m_toolbar.SetButtonStructSize();
	m_toolbar.SetButtonSize(CSize(16, 16));
	m_toolbar.SetImageList(m_tbImg);

	m_toolbar.AddButton(ID_PAGE_BEGIN + 0, BTNS_CHECKGROUP, TBSTATE_ENABLED, 0, L"基本视图", NULL);
	m_toolbar.AddButton(ID_PAGE_BEGIN + 1, BTNS_CHECKGROUP, TBSTATE_ENABLED, 1, L"树形视图", NULL);
	m_toolbar.AddButton(ID_PAGE_BEGIN + 2, BTNS_CHECKGROUP, TBSTATE_ENABLED, 2, L"高级视图", NULL);
	m_toolbar.AddButton(0, BTNS_SEP, 0, 0, NULL, NULL);
	m_toolbar.AddButton(ID_SIMPLIFY, BTNS_BUTTON, TBSTATE_ENABLED, 5, L"化简", NULL);
	m_toolbar.AddButton(0, BTNS_SEP, 0, 0, NULL, NULL);
	m_toolbar.AddButton(ID_IMPORT_FILTER, BTNS_BUTTON, TBSTATE_ENABLED, 3, L"导入", NULL);
	m_toolbar.AddButton(ID_EXPORT_FILTER, BTNS_BUTTON, TBSTATE_ENABLED, 4, L"导出", NULL);

	m_toolbar.SetMaxTextRows(0);

	m_toolbar.AutoSize();
	m_toolbar.ShowWindow(SW_SHOW);


	m_dlgSimple.Create(m_hWnd);
	m_dlgGraphic.Create(m_hWnd);
	m_dlgAdvanced.Create(m_hWnd);

	m_editors.push_back(&m_dlgSimple);
	m_editors.push_back(&m_dlgGraphic);
	m_editors.push_back(&m_dlgAdvanced);

	m_activePage = m_defaultPage;
	if (m_activePage < 0 || m_activePage >= static_cast<int>(m_editors.size()))
	{
		m_activePage = 0;
	}
	SelectPage(m_activePage);

	ResizeChildDialogs();
	return 0;
}

CFilterEditor* CFilterEditDlg::GetActiveEditor() const
{
	return GetEditor(m_activePage);
}

CFilterEditor* CFilterEditDlg::GetEditor(int index) const
{
	return m_editors[static_cast<size_t>(index)];
}

void CFilterEditDlg::SelectPage(int index)
{
	for (int i = 0; i < static_cast<int>(m_editors.size()); i++)
	{
		if (i == index)
		{
			GetEditor(i)->Show();
			m_toolbar.SetState(ID_PAGE_BEGIN + i, TBSTATE_ENABLED|TBSTATE_CHECKED);
		}
		else
		{
			GetEditor(i)->Hide();
			m_toolbar.SetState(ID_PAGE_BEGIN + i, TBSTATE_ENABLED);
		}
	}
	m_activePage = index;
}

void CFilterEditDlg::UpdateFilter(int index)
{
	filter* f = GetEditor(index)->GetFilter();
	for (size_t i = 0; i < m_editors.size(); i++)
	{
		if (static_cast<int>(i) != index)
		{
			m_editors[i]->SetFilter(f);
		}
	}
}

void CFilterEditDlg::ResizeChildDialogs()
{
	CWindow wnd = GetDlgItem(IDC_STATIC_SUBDLG);
	CRect rc;
	wnd.GetWindowRect(&rc);

	ScreenToClient(&rc);

	m_dlgSimple.MoveWindow(&rc);
	m_dlgGraphic.MoveWindow(&rc);
	m_dlgAdvanced.MoveWindow(&rc);
}

LRESULT CFilterEditDlg::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT ret = CDialogResize<CFilterEditDlg>::OnSize(uMsg, wParam, lParam, bHandled);
	ResizeChildDialogs();
	return ret;
}

LRESULT CFilterEditDlg::OnImport(WORD , WORD , HWND , BOOL& )
{
	COpenFileDialog dlg(m_defaultFilterDir.c_str(), L"日志过滤配置文件(*.xml)\0*.xml\0所有文件(*.*)\0*.*\0\0");
	if (dlg.DoModal() == IDOK)
	{
		m_defaultFilterDir = helper::GetDir(dlg.m_szFileName);
		filter_creator* fc = filter_creator::instance();
		filter* f = fc->load(dlg.m_szFileName);
		for (size_t i = 0; i < m_editors.size(); i++)
		{
			m_editors[i]->SetFilter(f);
		}
	}
	return 0;
}

LRESULT CFilterEditDlg::OnExport(WORD , WORD , HWND , BOOL& )
{
	CSaveFileDialog dlg(m_defaultFilterDir.c_str(), L"xml", L"filter.xml", L"日志过滤配置文件(*.xml)\0*.xml\0所有文件(*.*)\0*.*\0\0");
	if (dlg.DoModal() == IDOK)
	{
		m_defaultFilterDir = helper::GetDir(dlg.m_szFileName);
		filter_creator* fc = filter_creator::instance();
		filter* f = GetActiveEditor()->GetFilter();
		fc->save(f, dlg.m_szFileName);
	}
	return 0;
}

LRESULT CFilterEditDlg::OnPage(WORD , WORD id, HWND , BOOL& )
{
	int page = id - ID_PAGE_BEGIN;
	GetEditor(page)->SetFilter(GetActiveEditor()->GetFilter());
	SelectPage(page);
	return 0;
}

LRESULT CFilterEditDlg::OnSimplify(WORD , WORD , HWND , BOOL& )
{
	CFilterEditor* e = GetActiveEditor();
	component* c = dynamic_cast<component*>(e->GetFilter());
	if (c)
	{
		component* simp_c = c->simplify();
		e->SetFilter(dynamic_cast<filter*>(simp_c));
		delete simp_c;
	}

	return 0;
}

bool CFilterEditDlg::SetFilter(const filter* f)
{
	if (!GetActiveEditor()->SetFilter(f) && m_defaultPage == -1)
	{
		for (size_t i = 0; i < m_editors.size(); i++)
		{
			if (m_editors[i]->SetFilter(f))
			{
				SelectPage(static_cast<int>(i));
				break;
			}
		}
	}
	return true;
}

filter* CFilterEditDlg::GetFilter() const
{
	return GetActiveEditor()->GetFilter();
}
