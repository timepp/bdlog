#include "stdafx.h"

LARGE_INTEGER CProfiler::m_freq = {};
BOOL CProfiler::m_avaliable = ::QueryPerformanceFrequency(&CProfiler::m_freq);

// ²âÊÔbdlogµÄÐÔÄÜ

#define BATCH_COUNT 1
static int PerformanceTest(const wchar_t* /*description*/)
{
	CProfiler pf(true);
	for (size_t i = 0; i < BATCH_COUNT; i++)
	{
		Log(LL_DEBUG, TAG(L"aa;bb"), L"aaa", 1);
		Log(LL_DEBUG, NOTAG, L"simple log text with code analysis warning %d", L"hhh");
		Log(LL_DEBUG, TAG(L"kernel;perf;logic"), L"the logic control flow: %s%d", L"aaa", 333);
	}
	return pf.GetElapsedMicroSeconds();
}

static void RunPerformanceTest(const wchar_t* desc, int times = 1)
{
	int total_us = 0;
	for (int i = 0; i < times; i++)
	{
		total_us += PerformanceTest(desc);
	}
	int total_count = times * BATCH_COUNT * 3;
	double avg_us = static_cast<double>(total_us) / total_count;
	wprintf_s(L"%-40s %16d %16d %f\n", desc, total_count, total_us, avg_us);
}

void TEST_Performance()
{
	ILogController* ctrl = GetLogController();
	ctrl->RemoveOutputDevice(NULL);

	ctrl->AddOutputDevice(L"pipe", LODT_PIPE, L"");
	ctrl->AddOutputDevice(L"sm", LODT_SHARED_MEMORY, L"enable:1 filter:level>32");
	RunPerformanceTest(L"EndUser_Normal");

	ctrl->ChangeOutputDeviceConfig(L"sm", L"filter:level>0");
	RunPerformanceTest(L"EndUser_ManyError");

	ctrl->AddOutputDevice(L"file", LODT_FILE, L"enable:1 async:1 path:log_file0.txt");
	RunPerformanceTest(L"InternalUser_FileEnabled");

	ctrl->ChangeOutputDeviceConfig(L"file", L"path:log_file1.txt");
	RunPerformanceTest(L"InternalUser_FileEnabled_Async");

	ctrl->ChangeOutputDeviceConfig(L"file", L"share_read:0 path:log_file2.txt");
	RunPerformanceTest(L"InternalUser_FileEnabled_NoShareRead");

	ctrl->ChangeOutputDeviceConfig(L"file", L"buffer_size:0 path:log_file3.txt");
	RunPerformanceTest(L"InternalUser_FileEnabled_NoBuffer");

	ctrl->ChangeOutputDeviceConfig(L"file", L"buffer_size:0 async:1 path:log_file4.txt");
	RunPerformanceTest(L"InternalUser_FileEnabled_NoBuffer_Async");
}
