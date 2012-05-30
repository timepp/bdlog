#pragma once

#include "outputdevice_common.h"
#include <bdsharemem.h>

#pragma pack(push, 1)
struct MemLogQuene
{
	struct MemLogRecord
	{
		INT64 unixTime;
		INT32 milliSecond;
		UINT32 tid;
		WCHAR content[120];
	};

	UINT32 totalLength;                      //共享内存的大小
	UINT32 recordCount;                 //日志队列大小
	UINT32 recordLength;                      //单条日志大小
	UINT32 head;                         //队列头
	UINT32 rear;                         //队列尾
	MemLogRecord record[1000]; //定长队列
};
#pragma pack(pop)

class CLOD_ShareMemory : public CLogOutputDeviceBase
{
public:
	virtual HRESULT Open(ILogOption* opt);
	virtual HRESULT Close();
	virtual HRESULT Write(const LogItem* item);
	virtual HRESULT Flush();
	virtual HRESULT OnConfigChange(ILogOption* opt);

	CLOD_ShareMemory();

private:
	static void GetShareMemoryName(ILogOption* opt, wchar_t* name, size_t len);
	CSharingMemory m_shareMemory;
	wchar_t m_smName[256];
};

