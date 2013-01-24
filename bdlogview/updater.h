#pragma once

/************************************************************************
简单的更新策略：自动检查、自动下载、自动安装。
详细流程：
1. 检查更新
2. 若有更新，把新版本下载到bdlogview.exe.new
3. 把bdlogview.exe 改名为 bdlogview.exe.old
4. 把bdlogview.exe.new 改名为 bdlogview.exe
5. 下次启动时生效
6. 启动时，若发现bdlogview.exe.old，就删除之，并展示升级提示，显示新版本的功能
************************************************************************/

#include <vector>

struct VersionDetail
{
	UINT64 ver;
	CStringW description;
};

typedef std::vector<VersionDetail> Versions;

struct Updater
{
	static void CheckAndUpdate(BOOL bBackend);

	/// 启动时调用，展示新版本详细信息
	static void NotifyNewVersion();

	static CStringW GetVersionInfoFilePath();

private:
	static Versions ParseVersionInfoFile();
	static CStringW GetOldExePath();
	static unsigned int __stdcall CheckAndUpdateWorker(void* param);
};

