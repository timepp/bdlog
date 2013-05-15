#include "stdafx.h"
#include "filtereditdlg.h"

CAdvancedFilterEditDlg::CAdvancedFilterEditDlg() : m_filter(NULL)
{
}

bool CAdvancedFilterEditDlg::SetFilter(const filter* f)
{
	delete m_filter;
	m_filter = NULL;

	const component* comp = dynamic_cast<const component*>(f);
	if (comp)
	{
		m_filter = dynamic_cast<filter*>(comp->clone());
	}
	return false;
}

filter* CAdvancedFilterEditDlg::GetFilter() const
{
	return m_filter;
}
