#include "stdafx.h"
#include "vislogicdlg.h"
#include "servicehelper.h"

static CVisLogicDlg g_dlg;

#define IDT_REFRESH 1001

static int TimeIntervalToPixel(INT64 us, int usec_per_pixel)
{
	return static_cast<int>(us / usec_per_pixel);
}

CVisLogicDlg::CVisLogicDlg()
{

}

CVisLogicDlg::~CVisLogicDlg()
{
	if (g_dlg.IsWindow())
	{
		g_dlg.DestroyWindow();
	}
}

void CVisLogicDlg::Show()
{
	if (!g_dlg.IsWindow())
	{
		g_dlg.Create(NULL);
	}
	g_dlg.ShowWindow(SW_SHOW);
}

LRESULT CVisLogicDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CVisLogicDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CDialogResize<CVisLogicDlg>::DlgResize_Init();

	m_pen[0].CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
	m_pen[1].CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	m_pen[2].CreatePen(PS_SOLID, 1, RGB(192, 128, 0));
	m_pen[3].CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	m_pen[4].CreatePen(PS_DOT, 1, RGB(192, 192, 192));

	m_lat.Init();
	SetTimer(IDT_REFRESH, 1000);
	UpdateCombo();

	return 0;
}

LRESULT CVisLogicDlg::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (wParam == IDT_REFRESH)
	{
		if (IsWindowVisible())
		{
			UpdateCombo();
			Refresh();
		}
	}

	return 0;
}

void CVisLogicDlg::Refresh()
{
	if (!m_process) return;

	// 如果日志源信息里没有此PID，或者当前未在监视状态，说明进程已经退出，不使用当前时间做为T2，否则使用当前时间做为T2
	bool use_current_time_as_end_time = false;
	if (ServiceHelper::GetLogCenter()->MonitoringPipe())
	{
		tplog::lsi_vec_t src = ServiceHelper::GetLogCenter()->get_sources();
		for (size_t i = 0; i < src.size(); i++)
		{
			if (src[i].pid == m_process->pid)
			{
				use_current_time_as_end_time = true;
				break;
			}
		}
	}

	if (m_process->threads.empty()) return;

	time_range range;
	range.t1 = m_process->threads[0]->life.t1;

	if (use_current_time_as_end_time)
	{
//		AccurateTime t = m_lat.GetTime();
//		range.t2.sec = t.unix_time;
//		range.t2.usec = t.micro_second;
	}
	else
	{
		range.t2 = m_process->life.t2;
	}

	CRect rc;
	GetDlgItem(IDC_STATIC_CANVAS).GetWindowRect(&rc);
	ScreenToClient(&rc);

	int usec_per_pixel = static_cast<int>((range.t2 - range.t1) / rc.Width());

	CClientDC dc(m_hWnd);
	CMemoryDC mdc(dc, rc);

	CRect rcClient;
	GetClientRect(&rcClient);
	
	
	DrawThreads(range, usec_per_pixel, &mdc, rc);
}

void CVisLogicDlg::UpdateCombo()
{
	CComboBox combo = (HWND)GetDlgItem(IDC_COMBO_PID);
	const process_map_t& m = ServiceHelper::GetVisualLogic()->GetProcessInfo();
	for (process_map_t::const_iterator it = m.begin(); it != m.end(); ++it)
	{
		UINT32 pid = it->second->pid;
		wchar_t spid[32];
		_snwprintf_s(spid, _TRUNCATE, L"%u", pid);

		int index = combo.FindString(0, spid);
		if (index < 0)
		{
			index = combo.AddString(spid);
			combo.SetItemData(index, pid);
		}
	}

	if (combo.GetCurSel() < 0 && combo.GetCount() > 0)
	{
		combo.SetCurSel(0);
	}
}

void CVisLogicDlg::SetThreadStyle(const thread_info* ti, WTL::CDC* pDC)
{
	if (ti->name == L"主线程")
	{
		pDC->SelectPen(m_pen[0]);
		pDC->SetDCPenColor(RGB(0, 0, 255));
	}
	else if (ti->name.find(L"DB") != std::wstring::npos)
	{
		pDC->SelectPen((HPEN)GetStockObject(DC_PEN));
		pDC->SetDCPenColor(RGB(0, 128, 0));
	}
	else
	{
		pDC->SelectPen(m_pen[2]);
	}
}

void CVisLogicDlg::DrawThreads(time_range range, int usec_per_pixel, CDC* pDC, CRect rc)
{
	// 缩放因子使用usec/pixel，即微秒每像素表示
	memset(&m_channel, 0, sizeof(m_channel));

	pDC->FillSolidRect(&rc, RGB(255, 255, 255));

	for (thread_infos_t::const_iterator it = m_process->threads.begin(); it != m_process->threads.end(); ++it)
	{
		const thread_info* ti = *it;
		time_range r = ti->life;

		// 如果不在绘制区域内，直接返回
		// if (r.t1 > range.t2) continue;
		// if (r.t2.sec != 0 && r.t2 < range.t1) continue;

		// 首先找到一条空闲的通道
		int i = 0;
		for (; i < _countof(m_channel); i++)
		{
			if (m_channel[i] == 0) break;
			thread_map_t::const_iterator it2 = m_process->cache.find(m_channel[i]);
			if (it2 != m_process->cache.end() && it2->second->life.t2.sec != 0 && it2->second->life.t2 + 5 * usec_per_pixel < ti->life.t1)
			{
				break;
			}
		}

		m_channel[i] = ti->tid;
		
		// 计算thread life和range重叠的部分
		if (r.t1 < range.t1)
		{
			r.t1 = range.t1;
		}
		if (r.t2 > range.t2 || r.t2.sec == 0)
		{
			r.t2 = range.t2;
		}

		// 没有重叠部分，忽略
		if (r.t1 > r.t2) continue;

		// 若有重叠部分，绘制
		SetThreadStyle(ti, pDC);
		int line_start = TimeIntervalToPixel(r.t1 - range.t1, usec_per_pixel);
		int line_end = TimeIntervalToPixel(r.t2 - range.t1, usec_per_pixel);
		int y = i * 10 + 5 + rc.top;
		pDC->MoveTo(line_start + rc.left, y);
		pDC->LineTo(line_end + rc.left, y);

		// 绘制向parent tid的连接线
		if (ti->parent_tid != 0)
		{
			for (int j = 0; j < _countof(m_channel); j++)
			{
				if (m_channel[j] == ti->parent_tid)
				{
					int y1 = j * 10 + 5 + rc.top;
					int y2 = i * 10 + 5 + rc.top;
					int x = line_start + rc.left;
					pDC->SelectPen(m_pen[4]);
					pDC->MoveTo(x, y1);
					pDC->LineTo(x, y2);
					break;
				}
			}
		}

		// 绘制tasks
		pDC->SelectPen(m_pen[3]);
		for (task_infos_t::const_iterator it = ti->m_tasks.begin(); it != ti->m_tasks.end(); ++it)
		{
			const task_info& task = *it;
			time_range r = task.life;
			int line_start = TimeIntervalToPixel(r.t1 - range.t1, usec_per_pixel);
			int line_end = TimeIntervalToPixel(r.t2 - range.t1, usec_per_pixel);
			int y = i * 10 + 5 + rc.top;
			pDC->MoveTo(line_start + rc.left, y);
			pDC->LineTo(line_end + rc.left, y);
		}
	}
}

LRESULT CVisLogicDlg::OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint pt(LOWORD(lParam), HIWORD(lParam));

	CRect rcCanvas;
	GetDlgItem(IDC_STATIC_CANVAS).GetWindowRect(&rcCanvas);
	ScreenToClient(&rcCanvas);

	int index = (pt.y - rcCanvas.top - 7) / 10;
	thread_map_t::const_iterator it = m_process->cache.find(m_channel[index]);
	if (it != m_process->cache.end())
	{
		const thread_info* ti = it->second;
		CStringW info;
		CMenu mu;
		mu.CreatePopupMenu();

		info.Format(L"%u:%s", ti->tid, ti->name.c_str());
		mu.AppendMenuW(MF_BYCOMMAND, (UINT_PTR)0, info);
		
		ClientToScreen(&pt);
		mu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
	}

	return 0;
}

LRESULT CVisLogicDlg::OnCbnSelchangeComboPid(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CComboBox combo = (HWND)GetDlgItem(IDC_COMBO_PID);
	int index = combo.GetCurSel();
	if (index < 0) return 0;

	UINT32 pid = combo.GetItemData(index);
	const process_map_t& m = ServiceHelper::GetVisualLogic()->GetProcessInfo();
	process_map_t::const_iterator it = m.find(pid);
	if (it == m.end()) return 0;

	m_process = it->second;

	Refresh();
	return 0;
}

LRESULT CVisLogicDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);
	return 0;
}
