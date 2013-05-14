#pragma once

#include <bdlogprovider.h>
#include <vector>
#include <deque>
#include <atlsync.h>
#include "common.h"
#include "serviceid.h"

struct LogRange
{
	UINT64 idmin;
	UINT64 idmax;
};

class CLogCenterListener
{
public:
	virtual void OnNewLog(const LogRange& range) = 0;
	virtual void OnConnect(){}
	virtual void OnDisconnect(){}
	virtual ~CLogCenterListener(){}
};

class CLogCenter : public tp::service_impl<SID_LogCenter>, public bdlog::log_listener, public bdlog::log_source_support
{
	TP_SET_DEPENDENCIES(create, );
	TP_SET_DEPENDENCIES(destroy, );

public:
	CLogCenter();
	~CLogCenter();

	void Init();
	void Uninit();

	void ConnectPipe();
	void ConnectFile(LPCWSTR pszPath);
	void ConnectShareMemory(LPCWSTR name);
	void Disconnect();
	bool MonitoringPipe() const;

	LogRange GetLogRange();
	LogInfo* GetLog(UINT64 logid);
	void LockLog();
	void UnlockLog();

	void ClearAllLog();
	// 清除logid之前的日志
	void ClearOldLog(UINT64 logid);

	void AddListener(CLogCenterListener* pListener);
	void RemoveListener(CLogCenterListener* pListener);

	virtual void on_new_log(bdlog::logitem* li);
	virtual void on_notify(int notifyid, HRESULT hr);

	virtual size_t source_count() const;
	virtual bdlog::lsi_vec_t get_sources() const;

private:
	static VOID CALLBACK TimerProc(
		__in  HWND hwnd,
		__in  UINT uMsg,
		__in  UINT_PTR idEvent,
		__in  DWORD dwTime
		);
	static VOID CALLBACK NotifyProc(
		__in  HWND hwnd,
		__in  UINT uMsg,
		__in  UINT_PTR idEvent,
		__in  DWORD dwTime
		);

	void MergeBuffer();
	bool EnablePipeDeviceFile(bool bEnable);
	bool EnablePipeDeviceReg(bool bEnable);

	void ShowNotifyMsg();

private:
	// listener
	typedef std::vector<CLogCenterListener*> ListenerList;
	ListenerList m_listeners;
	CCriticalSection m_csListener;

	// buffer
	typedef std::vector<bdlog::logitem*> LogBuffer;
	LogBuffer m_buffer;
	CCriticalSection m_csLogBuffer;

	// db
	typedef std::vector<LogInfo*> LogDB;
	LogDB m_logDB;
	CCriticalSection m_csLogDB;

	// provider
	bdlog::pipe_reader m_logPipeReader;
	bdlog::file_reader m_logFileReader;
	bdlog::sharememory_reader m_logShareMemoryReader;

	// occupytime
	std::map<bdlog::uint32_t, UINT64> m_lastLogInSameThread;

	UINT64 m_logID;

	UINT m_timerID;

	// 收到log provider的notify后，在TIMER中显示
	UINT m_notifyTimerID;
	int m_notifyID;

	bool m_autoEnablePipeDeviceFile;
	CStringW m_cfgPathFile;
	bool m_autoEnablePipeDeviceReg;
	CStringW m_cfgPathReg;
};

TP_DEFINE_GLOBAL_SERVICE(CLogCenter, L"日志内容管理中心");
