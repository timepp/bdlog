#include "stdafx.h"

#include "filtereditdlg.h"
#include "common.h"
#include "concretefilter.h"
#include "servicehelper.h"
#include "helper.h"
#include "logpropdb.h"

CSimpleFilterEditDlg::CSimpleFilterEditDlg() : m_filter(NULL)
{
}

LRESULT CSimpleFilterEditDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CComboBox combo = (HWND)GetDlgItem(IDC_COMBO_LOGCLASS);

	CLogPropertyDB::LevelSet ls = ServiceHelper::GetLogPropertyDB()->GetDefaultLevels();
	ls.insert(0);
	for (CLogPropertyDB::LevelSet::const_iterator it = ls.begin(); it != ls.end(); ++it)
	{
		CStringW name = helper::GetLogLevelDescription(*it);
		int index = combo.AddString(name);
		combo.SetItemData(index, (DWORD_PTR)*it);
	}
	combo.SetCurSel(0);

	CDialogResize<CSimpleFilterEditDlg>::DlgResize_Init(false, false);

	CEdit lc(GetDlgItem(IDC_EDIT_LOGCONTENT));
	lc.SetFocus();
	lc.SetSel(0, -1);

	return TRUE;
}

bool CSimpleFilterEditDlg::SetConcreteFilter(const component* c)
{
	{
		const logclass_filter* f = dynamic_cast<const logclass_filter*>(c);
		if (f)
		{
			if (f->m_class_high != 0) return false;
		
			CComboBox combo = (HWND)GetDlgItem(IDC_COMBO_LOGCLASS);
			CStringW name = helper::GetLogLevelDescription(f->m_class_low);
			if (combo.SelectString(0, name) == CB_ERR)
			{
				combo.SetWindowText(name);
			}
			return true;
		}
	}

	{
		const logcontent_filter* f = dynamic_cast<const logcontent_filter*>(c);
		if (f)
		{
			if (!f->m_ignore_case) return false;

			SetDlgItemText(IDC_EDIT_LOGCONTENT, f->m_matcher.c_str());
			return true;
		}
	}

	{
		const logtag_filter* f = dynamic_cast<const logtag_filter*>(c);
		if (f)
		{
			SetDlgItemText(IDC_EDIT_LOGTAG, f->m_tag.c_str());
			return true;
		}
	}

	{
		const logprocessname_filter* f = dynamic_cast<const logprocessname_filter*>(c);
		if (f)
		{
			SetDlgItemText(IDC_EDIT_PROCESSNAME, f->m_process_name.c_str());
			return true;
		}
	}

	{
		const logpid_filter* f = dynamic_cast<const logpid_filter*>(c);
		if (f)
		{
			SetDlgItemText(IDC_EDIT_PID, tp::cz(L"%u", f->m_pid));
			return true;
		}
	}

	{
		const logtid_filter* f = dynamic_cast<const logtid_filter*>(c);
		if (f)
		{
			SetDlgItemText(IDC_EDIT_TID, tp::cz(L"%u", f->m_tid));
			return true;
		}
	}

	return false;
}

bool CSimpleFilterEditDlg::SetFilter(const filter* flt)
{
	delete m_filter;
	m_filter = NULL;
	ResetControls();

	bool badnode = false;

	const component* comp = dynamic_cast<const component*>(flt);
	if (comp)
	{
		component* c = comp->simplify();
		if (c)
		{
			if (c->classname() == L"logical_and_filter")
			{
				for (size_t i = 0; i < c->child_count(); i++)
				{
					if (!SetConcreteFilter(c->get_child(i)))
					{
						badnode = true;
						m_filter = dynamic_cast<filter*>(c->clone());
						break;
					}
				}
			}
			else
			{
				if (!SetConcreteFilter(c))
				{
					badnode = true;
					m_filter = dynamic_cast<filter*>(c->clone());
				}
			}
		}
	}

	if (badnode)
	{
		ShowAllChild(SW_HIDE);
		CRect rc;
		GetClientRect(&rc);
		GetDlgItem(IDC_STATIC_BADNODE).MoveWindow(rc);
		GetDlgItem(IDC_STATIC_BADNODE).ShowWindow(SW_SHOW);
		return false;
	}
	else
	{
		ShowAllChild(SW_SHOW);
		GetDlgItem(IDC_STATIC_BADNODE).ShowWindow(SW_HIDE);
		return true;
	}
}

filter* CSimpleFilterEditDlg::GetFilter() const
{
	if (m_filter)
	{
		return m_filter;
	}

	logical_and_filter* f = new logical_and_filter;

	{
		CComboBox combo = (HWND)GetDlgItem(IDC_COMBO_LOGCLASS);
		int index = combo.GetCurSel();
		int logclass;
		if (index >= 0)
		{
			logclass = static_cast<int>(combo.GetItemData(index));
		}
		else
		{
			CStringW text;
			combo.GetWindowText(text);
			logclass = _wtoi(text);
		}
		
		if (logclass > 0)
		{
			f->add_child(new logclass_filter(static_cast<UINT>(logclass), 0));
		}
	}

	{
		CStringW strText;
		GetDlgItem(IDC_EDIT_LOGCONTENT).GetWindowTextW(strText);
		if (strText.GetLength() > 0)
		{
			f->add_child(new logcontent_filter((LPCWSTR)strText, true, false));
		}
	}

	{
		CStringW strText;
		GetDlgItem(IDC_EDIT_LOGTAG).GetWindowTextW(strText);
		if (strText.GetLength() > 0)
		{
			f->add_child(new logtag_filter((LPCWSTR)strText));
		}
	}

	{
		CStringW strText;
		GetDlgItem(IDC_EDIT_PROCESSNAME).GetWindowTextW(strText);
		if (strText.GetLength() > 0)
		{
			f->add_child(new logprocessname_filter((LPCWSTR)strText));
		}
	}

	{
		CStringW strText;
		GetDlgItem(IDC_EDIT_PID).GetWindowTextW(strText);
		if (strText.GetLength() > 0)
		{
			DWORD pid;
			swscanf_s((LPCWSTR)strText, L"%u", &pid);
			f->add_child(new logpid_filter(pid));
		}
	}

	{
		CStringW strText;
		GetDlgItem(IDC_EDIT_TID).GetWindowTextW(strText);
		if (strText.GetLength() > 0)
		{
			DWORD tid;
			swscanf_s((LPCWSTR)strText, L"%u", &tid);
			f->add_child(new logtid_filter(tid));
		}
	}

	ON_LEAVE_1(delete f, logical_and_filter*, f);
	return dynamic_cast<filter*>(f->simplify());
}

void CSimpleFilterEditDlg::ShowAllChild(int cmdshow)
{
	CWindow wnd = GetWindow(GW_CHILD);
	while (wnd.IsWindow())
	{
		wnd.ShowWindow(cmdshow);
		wnd = wnd.GetWindow(GW_HWNDNEXT);
	}
}

void CSimpleFilterEditDlg::ResetControls()
{
	GetDlgItem(IDC_EDIT_LOGCONTENT).SetWindowTextW(L"");
	GetDlgItem(IDC_EDIT_PROCESSNAME).SetWindowTextW(L"");
	GetDlgItem(IDC_EDIT_PID).SetWindowTextW(L"");
	GetDlgItem(IDC_EDIT_TID).SetWindowTextW(L"");
	GetDlgItem(IDC_EDIT_LOGTAG).SetWindowTextW(L"");

	CComboBox combo = (HWND)GetDlgItem(IDC_COMBO_LOGCLASS);
	combo.SetCurSel(0);
}