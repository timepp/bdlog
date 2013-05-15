#pragma once

/** @file
 *  tplog interface
 *  - printf style
 *  - support log level and tag
 *  - configurable output devices: console, file, pipe, memory, or devices wrote by user
 *  - change settings at runtime
 *  - machine-wide synchronized time, accurate to 10^-6 s
 *
 *  - TPLOG_EXPORT                      defined by tplog provider( which exports 'GetLogController' )
 *  - TPLOG_USE_AS_DLL                  DLL static load
 *  - TPLOG_USE_AS_DLL_DYNAMIC_LOAD     DLL dynamic load
 */

#include <windows.h>
#include <stdio.h>

#ifdef TPLOG_EXPORT
	#define TPLOGAPI extern "C" __declspec(dllexport)
#else
	#if defined(TPLOG_USE_AS_DLL)
		#define TPLOGAPI extern "C" __declspec(dllimport)
		#pragma comment(lib, "tplog.lib")
	#else
		#define TPLOGAPI extern "C"
	#endif
#endif

#define FACILITY_TPLOG 0x10D
#define TPLOG_E_DEVICE_NOT_READY                  MAKE_HRESULT(1, FACILITY_TPLOG, 1)
#define TPLOG_E_NO_REPORTER                       MAKE_HRESULT(1, FACILITY_TPLOG, 2)
#define TPLOG_E_NOT_INITED                        MAKE_HRESULT(1, FACILITY_TPLOG, 3)
#define TPLOG_E_DESTRUCTED                        MAKE_HRESULT(1, FACILITY_TPLOG, 4)
#define TPLOG_E_ALREADY_INITED                    MAKE_HRESULT(1, FACILITY_TPLOG, 5)
#define TPLOG_E_OUTPUT_DEVICE_ALREADY_EXIST       MAKE_HRESULT(1, FACILITY_TPLOG, 6)
#define TPLOG_E_OUTPUT_DEVICE_NOT_FOUND           MAKE_HRESULT(1, FACILITY_TPLOG, 7)
#define TPLOG_E_OUTPUT_DEVICE_FULL                MAKE_HRESULT(1, FACILITY_TPLOG, 8)
#define TPLOG_E_BEFORE_CONSTRUCT                  MAKE_HRESULT(1, FACILITY_TPLOG, 9)
#define TPLOG_E_FUNCTION_UNAVAILBLE               MAKE_HRESULT(1, FACILITY_TPLOG, 10)

enum LogLevel
{
	LL_DIAGNOSE = 0x05,
	LL_DEBUG    = 0x10,
	LL_EVENT    = 0x25,
	LL_ERROR    = 0x40,
};

struct LogItem
{
	UINT64 id;                    /// Unique ID (per process)
	FILETIME time;                /// Time when the log is generated, (not the log is wrote to output device)
	LogLevel level;               /// 
	const wchar_t* tag;           /// Tags are seperated with simicolon(;), e.g. "perf;check;xmpp"
	const wchar_t* content;       /// 

	DWORD pid;                    /// Process ID
	DWORD tid;                    /// Thread ID
	DWORD depth;                  /// Call depth 
};

struct ILogOption
{
	virtual const wchar_t* GetOption(const wchar_t* key, const wchar_t* defaultValue) = 0;
	virtual int GetOptionAsInt(const wchar_t* key, int defaultValue) = 0;
	virtual bool GetOptionAsBool(const wchar_t* key, bool defaultValue) = 0;

	virtual ~ILogOption() {}
};

struct ILogOutputDevice
{
	/** open log device
	 * 
	 *  @param config configuration in string
	 *                配置信息包含空格分开的多个配置项，配置项格式是 KEY:VAL
	 *                KEY=enable及KEY=filter的配置项由日志系统解析，其它配置项由具体的日志输出设备定义和解释
	 */
	virtual HRESULT Open(ILogOption* opt) = 0;

	/// 关闭日志设备
	virtual HRESULT Close() = 0;

	/// 写入日志
	virtual HRESULT Write(const LogItem* item) = 0;

	/// 刷新缓存
	virtual HRESULT Flush() = 0;

	/** 响应配置变化
	 *  有些情况下，比如用户手动修改了日志配置文件，使用这个函数让配置动态生效
	 */
	virtual HRESULT OnConfigChange(ILogOption* opt) = 0;

	virtual void IncreaseRefCounter() = 0;
	virtual void DecreaseRefCounter() = 0;

	virtual ~ILogOutputDevice(){}
};

// pre-defined output device type
enum LogOutputDeviceType
{
	LODT_NULL,
	LODT_CONSOLE,
	LODT_DEBUGOUTPUT,
	LODT_FILE,
	LODT_PIPE,
	LODT_SHARED_MEMORY
};

struct ILogController
{
	/** 初始化日志管理器
	 *  
	 *  @param configname [opt] logcontroller会在HKCU\Software\Baidu\TPLOG\configname下查找配置信息
	 *         configname不能超过32个字符
	 *         configname为NULL表示日志管理器不会从注册表读取config信息
	 */
	virtual HRESULT Init(const wchar_t* configname) = 0;
	virtual HRESULT UnInit() = 0;

	/** 输出日志
	 *  日志管理器会遍历目前启用的日志设备，对满足过滤条件的日志设备依次调用Write方法
	 *  日志ID、时间、线程号等信息由日志管理器自动生成
	 */
	virtual BOOL NeedLog(LogLevel level, const wchar_t* tag) = 0;
	virtual HRESULT Log(LogLevel level, const wchar_t* tag, const wchar_t* content) = 0;

	/** 日志输出设备管理
	 *
	 *  @param name 日志设备名字，以后唯一标识这个设备。要求长度小于32个字符
	 *  @param type 日志设备的类型
	 *  @param config 日志设备的配置串
	 *  @note 同一种类型的日志设备可以有不同名字的多个实例
	 */
	virtual HRESULT AddOutputDevice(const wchar_t* name, LogOutputDeviceType type, const wchar_t* config) = 0;
	virtual HRESULT AddCustomOutputDevice(const wchar_t* name, ILogOutputDevice* device, const wchar_t* config) = 0;
	virtual HRESULT RemoveOutputDevice(const wchar_t* name) = 0;
	virtual HRESULT ChangeOutputDeviceConfig(const wchar_t* name, const wchar_t* config) = 0;

	/// 改变日志深度 
	virtual HRESULT IncreaseCallDepth() = 0;
	virtual HRESULT DecreaseCallDepth() = 0;

	/** 监视日志配置 
	 *
	 *  日志管理器会启动一个新的线程，监视注册表项的改动
	 */
	virtual HRESULT MonitorConfigChange() = 0;
};

// 日志输出接口
TPLOGAPI ILogController* GetLogController();

#ifdef TPLOG_USE_AS_DLL_DYNAMIC_LOAD
namespace tplog
{
	class DLLLoader
	{
		typedef ILogController* (*PFUNC_GetLogController)();

		HMODULE m_hDll;
		ILogController* m_ctrl;

		template <typename T>
		struct GlobalData
		{
			static DLLLoader s_loader;
		};

		DLLLoader() : m_hDll(NULL)
		{
			m_hDll = ::LoadLibraryW(L"tplog.dll");
			if (m_hDll)
			{
				PFUNC_GetLogController pFunc;
#pragma warning(push)
#pragma warning(disable: 4191)
				pFunc = reinterpret_cast<PFUNC_GetLogController>(::GetProcAddress(m_hDll, "GetLogController"));
#pragma warning(pop)
				m_ctrl = pFunc();
			}
		}
		~DLLLoader()
		{
			if (m_hDll)
			{
				::FreeLibrary(m_hDll);
				m_hDll = NULL;
			}
			m_ctrl = NULL;
		}

	public:
		static DLLLoader& Instance()
		{
			return GlobalData<int>::s_loader;
		}
		ILogController* GetLogController() const
		{
			return m_ctrl;
		}
	};

	DLLLoader DLLLoader::GlobalData<int>::s_loader;
};
TPLOGAPI inline ILogController* GetLogController()
{
	return tplog::DLLLoader::Instance().GetLogController();
}
#endif

// 日志标签
struct LogTag
{
	explicit LogTag(const wchar_t* t): tag(t) {}
	const wchar_t* tag;
};

#ifndef LOGTAG_MODULE
#define LOGTAG_MODULE L""
#endif

#ifndef LOGTAG_FILE
#define LOGTAG_FILE L""
#endif

#define LOGTAG_EXTRA LOGTAG_MODULE LOGTAG_FILE

#define TAG_DEFAULT                LogTag(LOGTAG_EXTRA)
#define TAG(t)                     LogTag(t  L";" LOGTAG_EXTRA)
#define TAG2(t1, t2)               LogTag(t1 L";" t2 L";" LOGTAG_EXTRA)
#define TAG3(t1, t2, t3)           LogTag(t1 L";" t2 L";" t3 L";" LOGTAG_EXTRA)
#define TAG4(t1, t2, t3, t4)       LogTag(t1 L";" t2 L";" t3 L";" t4 L";" LOGTAG_EXTRA)

#define NOTAG TAG_DEFAULT

inline void LogV(LogLevel level, const wchar_t* tag, const wchar_t* fmt, va_list args) 
{
	ILogController* ctrl = GetLogController();
	if (!ctrl)
	{
		return;
	}
	if (!ctrl->NeedLog(level, tag))
	{
		return;
	}

	wchar_t buf[2048];
	_vsnwprintf_s(buf, _TRUNCATE, fmt, args);
	ctrl->Log(level, tag, buf);
}

#ifdef TPLOG_DISABLE_ALL
#define TPLOG_ARGV(l, t, f)
#else
#define TPLOG_ARGV(l, t, f) va_list args;va_start(args, f); LogV(l, t.tag, f, args); va_end(args);
#endif

#if (_MSC_VER >= 1500)
inline void Log(LogLevel level, LogTag tag, __in_z _Printf_format_string_ const wchar_t* fmt, ...)
{
	TPLOG_ARGV(level, tag, fmt);
}
#else
#ifdef STATIC_CODE_ANALYSIS
// VS2005不支持_Printf_format_string_,不能用CodeAnalysis检测出printf样式格式串的错误(如参数个数和类型不匹配等),
// 所以在静态代码检查的时候使用宏的形式借助wprintf让CodeAnalysis检测错误
inline void LogF(LogLevel level, LogTag tag, __in_z const wchar_t* fmt, ...)
{
	TPLOG_ARGV(level, tag, fmt);
}
#define Log(level, tag, ...) LogF(level, tag, __VA_ARGS__); wprintf(__VA_ARGS__);
#else
inline void Log(LogLevel level, LogTag tag, __in_z const wchar_t* fmt, ...)
{
	TPLOG_ARGV(level, tag, fmt);
}
#endif // STATIC_CODE_ANALYSIS
#endif // _MSC_VER

