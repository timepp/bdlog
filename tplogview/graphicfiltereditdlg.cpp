#include "stdafx.h"
#include "filtereditdlg.h"
#include "concretefiltereditdlg.h"
#include "filter.h"
#include "filtercreator.h"

LRESULT CGraphicFilterEditDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_tree.SetComponentCreator(filter_creator::instance());
	m_imgList.Create(16, 16, ILC_COLOR32, 10, 10);

	m_tree.AddComponentName(L"logical_and_filter");
	m_imgList.AddIcon(AtlLoadIcon(IDI_LOGICAL_AND));
	m_tree.AddComponentName(L"logical_or_filter");
	m_imgList.AddIcon(AtlLoadIcon(IDI_LOGICAL_OR));
	m_tree.AddComponentName(L"logical_not_filter");
	m_imgList.AddIcon(AtlLoadIcon(IDI_LOGICAL_NOT));

	m_tree.AddComponentName(L"");
	m_imgList.AddIcon(AtlLoadIcon(IDI_STOP));

	m_tree.AddComponentName(L"logclass_filter");
	m_imgList.AddIcon(AtlLoadIcon(IDI_LEVEL));
	m_tree.AddComponentName(L"logcontent_filter");
	m_imgList.AddIcon(AtlLoadIcon(IDI_CONTENT));
	m_tree.AddComponentName(L"logtag_filter");
	m_imgList.AddIcon(AtlLoadIcon(IDI_TAG));
	m_tree.AddComponentName(L"logprocessname_filter");
	m_imgList.AddIcon(AtlLoadIcon(IDI_PROCESS));
	m_tree.AddComponentName(L"logpid_filter");
	m_imgList.AddIcon(AtlLoadIcon(IDI_PID));
	m_tree.AddComponentName(L"logtid_filter");
	m_imgList.AddIcon(AtlLoadIcon(IDI_TID));
	
	
	//m_tree.SubclassWindow(GetDlgItem(IDC_TREE));
	m_tree.Create(m_hWnd, m_imgList);
	m_tree.SetFont(GetFont());
	return 0;
}

LRESULT CGraphicFilterEditDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	int cx = GET_X_LPARAM(lParam);
	int cy = GET_Y_LPARAM(lParam);

	CRect rc(0, 0, cx, cy);
	m_tree.MoveWindow(&rc);

	return 0;
}

bool CGraphicFilterEditDlg::SetFilter(const filter* f)
{
	m_tree.SetComponent(dynamic_cast<const component*>(f));
	return true;
}

filter* CGraphicFilterEditDlg::GetFilter() const
{
	return dynamic_cast<filter*>(m_tree.GetComponent());
}
