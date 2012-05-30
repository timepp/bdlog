// lgui.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "config.h"
#include "mainframe.h"
#include <time.h>
#include "firsttimedlg.h"
#include "logicvis.h"
#include "service.h"
#include "servicehelper.h"

CAppModule _Module;
HWND g_activeModlessDlg = NULL;

static void UpdateThread(void*)
{
	CStringW strVersion = helper::GetLatestVersion();
	CStringW strInfo;
	if (!strVersion.IsEmpty() && strVersion != helper::GetVersion())
	{
		CStringW strDllPath = helper::GetModuleFilePath();

		CStringW strErr = helper::UpdateSelf();
		if (strErr.IsEmpty())
		{
			strInfo.Format(L"已升级到新版本: %s, \n重启后才能生效。", (LPCWSTR)strVersion);
			MessageBox(NULL, strInfo, L"信息", MB_ICONINFORMATION|MB_OK);
		}
		else
		{
			strInfo.Format(L"升级失败: %s", (LPCWSTR)strErr);
			MessageBox(NULL, strInfo, L"信息", MB_ICONWARNING|MB_OK);
		}
	}
	else
	{
		strInfo.Format(L"最新版本为: %s, 您已是最新版本.", (LPCWSTR)strVersion);
		//		MessageBox(NULL, strInfo, L"信息", MB_ICONINFORMATION|MB_OK);
	}
}


int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	CModlessDlgKeyProcessor mdkp;

	if (CConfig::Instance()->GetConfig().product_name.empty())
	{
		CFirsttimeDlg dlg;
		dlg.DoModal();
	}
							
	_Module.AddMessageLoop(&theLoop);
	theLoop.AddMessageFilter(&mdkp);

	ServiceHelper::GetVisualLogic()->Init();
	ServiceHelper::GetLogPropertyDB()->Init();
	
	CMainFrame g_mainframe;
	if(g_mainframe.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	g_mainframe.ShowWindow(nCmdShow);

	// update thread
	if (_time64(NULL) - CConfig::Instance()->GetConfig().syscfg.last_check_update_time > 86400 * 3)
	{
		CConfig::Instance()->GetConfig().syscfg.last_check_update_time = _time64(NULL);
	//	_beginthread(UpdateThread, 0, NULL);
	}

	int nRet = theLoop.Run();

	ServiceHelper::GetVisualLogic()->Uninit();

	theLoop.RemoveMessageFilter(&mdkp);
	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR lpstrCmdLine, int nCmdShow)
{
	::OleInitialize(NULL);
	::LoadLibrary(L"Riched20.dll");

//	RunScript();

	CConfig::Instance()->Load(L"");

	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof(iccx);
	iccx.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES;
	::InitCommonControlsEx(&iccx);


	GD.module = hInstance;
	int argc = 0;
	LPWSTR* argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

	if (argc >= 2)
	{
		GD.strXLogFile = argv[1];
	}

	HRESULT hRes = _Module.Init(NULL, hInstance);
	(hRes);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();

	CConfig::Instance()->Save();
	::OleUninitialize();

	ServiceMgr::Instance().DestroyAllServices();

	return nRet;
}
