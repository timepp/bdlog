#pragma once

#include <atlwin.h>
#include <wtl/atlcrack.h>

#define DECLARE_MESSAGE_HANDLER(func) LRESULT func(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
