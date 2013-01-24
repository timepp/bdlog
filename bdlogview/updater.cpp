#include "stdafx.h"
#include "updater.h"
#include "helper.h"

unsigned int Updater::CheckAndUpdateWorker(void* param)
{
	helper::DownloadUrlToFile(
		L"http://db-win-dump00.db01.baidu.com:8180/help/bdlogview_ver.txt",
		Updater::GetVersionInfoFilePath()
		);

	Versions v = Updater::ParseVersionInfoFile();
	if (v.size() > 0 && v.front().ver > helper::GetFileVersion(helper::GetModuleFilePath()))
	{
		CStringW strTmpPath = helper::GetModuleFilePath() + L".tmp";
		CStringW strExePath = helper::GetModuleFilePath();
		CStringW strOldPath = Updater::GetOldExePath();

		helper::DownloadUrlToFile(
			L"http://db-win-dump00.db01.baidu.com:8180/help/bdlogview.exe",
			strTmpPath
			);
		
		::MoveFileExW(strExePath, strOldPath, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
		::MoveFileExW(strTmpPath, strExePath, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
	}

	return 0;
}

Versions Updater::ParseVersionInfoFile()
{
	Versions v;
	CStringW verfile = GetVersionInfoFilePath();

	FILE* fp = NULL;
	_wfopen_s(&fp, verfile, L"rt,ccs=UNICODE");
	if (!fp) return v;
	ON_LEAVE_1(fclose(fp), FILE*, fp);

	WCHAR line[1024];
	VersionDetail detail = {0, L""};
	while (fgetws(line, _countof(line), fp))
	{
		bool versionline = true;
		for (WCHAR* p = line; *p; p++)
		{
			if (*p == L'\r' || *p == L'\n') { *p = 0; break; }
			if ((*p < L'0' || *p > L'9') && *p != L'.') versionline = false;
		}
		if (!*line) versionline = false;

		if (versionline)
		{
			if (detail.ver) v.push_back(detail);
			detail.ver = helper::StringVersionToInt(line);
			detail.description = L"";
		}
		else
		{
			detail.description += line;
			detail.description += L"\n";
		}
	}

	if (detail.ver) v.push_back(detail);

	return v;
}

CStringW Updater::GetOldExePath()
{
	return helper::GetModuleFilePath() + L".old";
}
CString Updater::GetVersionInfoFilePath()
{
	return helper::GetConfigDir() + L"\\bdlogview_version.txt";
}

// bBackend为TRUE时，静默，无论有无更新，都不提示用户
// bBackend为FALSE时，如果没有更新，提示用户；如果有更新，更新完毕后再提示用户需要重启后生效
void Updater::CheckAndUpdate(BOOL bBackend)
{
	if (bBackend)
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, CheckAndUpdateWorker, NULL, 0, NULL);
		::CloseHandle(hThread);
	}
	else
	{
		CheckAndUpdateWorker(NULL);
		if (helper::FileExists(GetOldExePath()))
		{
			::MessageBoxW(NULL, L"已经完成更新，重启后生效。", L"更新提示", MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			::MessageBoxW(NULL, L"您使用的是最新版本。", L"提示", MB_OK|MB_ICONINFORMATION);
		}
	}
}

void Updater::NotifyNewVersion()
{
	CStringW strPath = GetOldExePath();
	if (helper::FileExists(strPath))
	{
		// 说明有更新
		UINT64 oldver = helper::GetFileVersion(strPath);
		::DeleteFileW(strPath);

		CStringW info = L"bdlogview已升级，细节如下：\n\n";
		Versions v = Updater::ParseVersionInfoFile();
		for (Versions::const_iterator it = v.begin(); it != v.end(); ++it)
		{
			if (it->ver <= oldver) break;
			info += helper::IntVersionToString(it->ver);
			info += L"\n";
			info += it->description;
			info += L"\n";
		}

		::MessageBoxW(NULL, info, L"bdlogview升级提示", MB_OK|MB_ICONINFORMATION);
	}
}



