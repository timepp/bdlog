#pragma once

#include <atlwin.h>
#include "helper.h"

#define WM_CHECKTAG_CHANGED  WM_USER+100

class CTagList: public CWindowImpl<CTagList, CListViewCtrl> 
{
public:
	BEGIN_MSG_MAP(CTagList)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseAction)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseAction)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouseAction)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnMouseAction)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnMouseAction)
	END_MSG_MAP()

private:
	LRESULT OnMouseActivate(UINT, WPARAM, LPARAM, BOOL&)
	{
		return MA_NOACTIVATE;
	}
	LRESULT OnMouseAction(UINT msg, WPARAM , LPARAM lp, BOOL& handled)
	{
		if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN)
		{
			int xPos = GET_X_LPARAM(lp);
			int yPos = GET_Y_LPARAM(lp);
			POINT pt = {xPos, yPos};
			int item = this->HitTest(pt, NULL);
			if (item >= 0)
			{
				SelectItem(item);
				SetCheckState(item, !GetCheckState(item));

				if (item == 0) // 0是全选的意思
				{
					for (int i = 1; i < GetItemCount(); i++)
					{
						SetCheckState(i, GetCheckState(0));
					}
				}
				else
				{
					SetCheckState(0, FALSE);
				}

				GetParent().SendMessage(WM_CHECKTAG_CHANGED, 0, 0);
			}
		}

		handled = TRUE;
		return 0;
	}
};

