#ifndef BDCLIENT_LOG_H
#define BDCLIENT_LOG_H

/** @file
 *  日志接口
 *  - printf风格语法
 *  - 日志级别和日志标签(每条日志可关联多个标签，方便后期查看和分析)
 *  - 可随意配置的日志输出设备(控制台、文件、管道、共享内存等)，第三方日志输出设备支持
 *  - 运行时通过INI文件调整日志设置和日志过滤器
 *
 *  控制bdlog使用方式的宏（在使用bdlog的工程里定义）
 *  - BDLOG_USE_AS_STATIC_LIB         作为静态库使用
 *  - BDLOG_USE_AS_DLL_DYNAMIC_LOAD   作为DLL使用，动态加载
 *  - BDLOG_USE_AS_DLL                作为DLL使用，静态加载。如果未定义上述两开关之一，将缺省定义此开关
 */

#include <windows.h>
#include <stdio.h>

#ifdef BDLOG_SELF_BUILD
#	ifdef BDLOG_STATIC_LIB
#		define BDLOGAPI extern "C"
#	else
#		define BDLOGAPI extern "C" __declspec(dllexport)
#	endif
#else
#	if !defined(BDLOG_USE_AS_STATIC_LIB) && !defined(BDLOG_USE_AS_DLL_DYNAMIC_LOAD)
#		define BDLOG_USE_AS_DLL
#	endif
#	if defined(BDLOG_USE_AS_DLL)
#		define BDLOGAPI extern "C" __declspec(dllimport)
#		pragma comment(lib, "bdlog.lib")
#	elif defined(BDLOG_USE_AS_STATIC_LIB)
#		define BDLOGAPI extern "C"
#	elif defined(BDLOG_USE_AS_DLL_DYNAMIC_LOAD)
#		define BDLOGAPI extern "C"
#	endif
#endif


/// 日志级别
enum LogLevel
{
	LL_DIAGNOSE = 0x05,
	LL_DEBUG    = 0x10,
	LL_EVENT    = 0x25,
	LL_ERROR    = 0x40,
};

#define FACILITY_BDLOG 0x10D
#define BDLOG_E_DEVICE_NOT_READY                  MAKE_HRESULT(1, FACILITY_BDLOG, 1)
#define BDLOG_E_NO_REPORTER                       MAKE_HRESULT(1, FACILITY_BDLOG, 2)
#define BDLOG_E_NOT_INITED                        MAKE_HRESULT(1, FACILITY_BDLOG, 3)
#define BDLOG_E_DESTRUCTED                        MAKE_HRESULT(1, FACILITY_BDLOG, 4)
#define BDLOG_E_ALREADY_INITED                    MAKE_HRESULT(1, FACILITY_BDLOG, 5)
#define BDLOG_E_OUTPUT_DEVICE_ALREADY_EXIST       MAKE_HRESULT(1, FACILITY_BDLOG, 6)
#define BDLOG_E_OUTPUT_DEVICE_NOT_FOUND           MAKE_HRESULT(1, FACILITY_BDLOG, 7)
#define BDLOG_E_OUTPUT_DEVICE_FULL                MAKE_HRESULT(1, FACILITY_BDLOG, 8)
#define BDLOG_E_BEFORE_CONSTRUCT                  MAKE_HRESULT(1, FACILITY_BDLOG, 9)
#define BDLOG_E_FUNCTION_UNAVAILBLE               MAKE_HRESULT(1, FACILITY_BDLOG, 10)

struct LogItem
{
	unsigned __int64 id;          // 每条日志产生时分配，进程内唯一

	__int64 unixTime;             // 日志产生时的时间（秒），含义同time()函数的返回值
	int microSecond;              // 日志产生时的时间：微秒
	LogLevel level;               // 日志级别
	const wchar_t* tag;           // 日志标签，一个日志可以关联多个标签，分号分隔。
	const wchar_t* content;       // 日志正文

	unsigned int tid;             // 日志产生时的线程ID
	unsigned int pid;             // 日志产生时的进程ID
	int depth;                    // 日志深度，每个线程的日志深度是独立的。
	const wchar_t* userContext;   // 用户指定的上下文，每条日志都相同
};

struct ILogOption
{
	virtual const wchar_t* GetOption(const wchar_t* key, const wchar_t* defaultValue) = 0;
	virtual int GetOptionAsInt(const wchar_t* key, int defaultValue) = 0;
	virtual bool GetOptionAsBool(const wchar_t* key, bool defaultValue) = 0;
};

struct ILogOutputDevice
{
	/** 打开一个日志设备
	 * 
	 *  @param config 一个字符串形式传入的配置信息。
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
	 *  @param configname [opt] logcontroller会在HKCU\Software\Baidu\BDLOG\configname下查找配置信息
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
	virtual HRESULT SetLogUserContext(const wchar_t* prefix) = 0;

	/** 监视日志配置 
	 *
	 *  日志管理器会启动一个新的线程，监视注册表项的改动
	 */
	virtual HRESULT MonitorConfigChange() = 0;
};

// 日志输出接口
BDLOGAPI ILogController* GetLogController();

#ifdef BDLOG_USE_AS_DLL_DYNAMIC_LOAD
namespace bdlog
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
			m_hDll = ::LoadLibraryW(L"bdlog.dll");
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
BDLOGAPI inline ILogController* GetLogController()
{
	return bdlog::DLLLoader::Instance().GetLogController();
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

#ifdef BDLOG_DISABLE_ALL
#define BDLOG_ARGV(l, t, f)
#else
#define BDLOG_ARGV(l, t, f) va_list args;va_start(args, f); LogV(l, t.tag, f, args); va_end(args);
#endif

#if (_MSC_VER >= 1500)
inline void Log(LogLevel level, LogTag tag, __in_z _Printf_format_string_ const wchar_t* fmt, ...)
{
	BDLOG_ARGV(level, tag, fmt);
}
#else
#ifdef STATIC_CODE_ANALYSIS
// VS2005不支持_Printf_format_string_,不能用CodeAnalysis检测出printf样式格式串的错误(如参数个数和类型不匹配等),
// 所以在静态代码检查的时候使用宏的形式借助wprintf让CodeAnalysis检测错误
inline void LogF(LogLevel level, LogTag tag, __in_z const wchar_t* fmt, ...)
{
	BDLOG_ARGV(level, tag, fmt);
}
#define Log(level, tag, ...) LogF(level, tag, __VA_ARGS__); wprintf(__VA_ARGS__);
#else
inline void Log(LogLevel level, LogTag tag, __in_z const wchar_t* fmt, ...)
{
	BDLOG_ARGV(level, tag, fmt);
}
#endif // STATIC_CODE_ANALYSIS
#endif // _MSC_VER



#endif // BDCLIENT_LOG_H
