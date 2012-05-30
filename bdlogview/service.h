#pragma once

#include <string>
#include <Windows.h>
#include <atlbase.h>

typedef int SERVICE_ID;

/// 服务框架
#if 0
template <typename T>
class CServicePtr
{
public:
	CServicePtr() : m_p(0)
	{
	}

	CServicePtr(CServicePtr<T>& rhs)
	{
		*this = rhs;
	}

	CServicePtr& operator= (CServicePtr<T>& rhs)
	{
		if (rhs.m_p != this->m_p)
		{
			m_p = rhs.m_p;
			rhs.m_p = NULL;
		}

		return *this;
	}

	CServicePtr(T* p)
	{
		m_p = p;
		m_p->AddRef();
	}

	~CServicePtr()
	{
		if (m_p)
		{
			m_p->Release();
			m_p = NULL;
		}
	}

	T* operator->() const
	{
		return m_p;
	}

	T** operator&()
	{
		return &m_p;
	}

private:
	T* m_p;
};
#endif

#define CServicePtr CComPtr
typedef HRESULT (*ServiceCreator)(IUnknown** s);

class ServiceMgr
{
public:
	static ServiceMgr& Instance()
	{
		__pragma(warning(push))
		__pragma(warning(disable:4640))
		static ServiceMgr mgr;
		__pragma(warning(pop))
		return mgr;
	}

	HRESULT SetServiceInfo(SERVICE_ID sid, ServiceCreator f, const wchar_t* name)
	{
		size_t index = static_cast<size_t>(sid);
		if (index >= _countof(m_serviceMap))
		{
			return E_FAIL; // TODO
		}

		ServiceInfo& si = m_serviceMap[index];
		if (si.creator || si.service)
		{
			return E_FAIL; // TODO
		}

		si.name = name;
		si.creator = f;
		si.service = NULL;

		return S_OK;
	}

	HRESULT GetService(SERVICE_ID sid, IUnknown** s)
	{
		ServiceInfo& si = m_serviceMap[sid];
		if (!si.service)
		{
			si.creator(&si.service);
			if (si.service)
			{
				si.service->AddRef();
			}
		}

		if (!si.service)
		{
			return E_FAIL; // TODO
		}

		*s = si.service;
		si.service->AddRef();
		return S_OK;
	}

	template <typename T>
	static CServicePtr<T> GetService()
	{
		IUnknown* p = NULL;
		ServiceMgr::Instance().GetService(T::ServiceID, &p);
		CServicePtr<T> ptr = dynamic_cast<T*>(p);
		p->Release();
		return ptr;
	}

	HRESULT DestroyAllServices()
	{
		m_destroying = true;

		for (size_t i = 0; i < _countof(m_serviceMap); i++)
		{
			ServiceInfo& si = m_serviceMap[i];
			if (si.service)
			{
				si.service->Release();
			}
		}

		for (size_t i = 0; i < _countof(m_serviceMap); i++)
		{
			ServiceInfo& si = m_serviceMap[i];
			if (si.service && !si.destroyed)
			{
				wchar_t buffer[1024];
				_snwprintf_s(buffer, _TRUNCATE, L"服务[%s] 在退出时没有被析构！\n", si.name);
				::OutputDebugStringW(buffer);
			}
		}


		m_destroying = false;
		return S_OK;
	}

	HRESULT NotifyServiceDestroy(SERVICE_ID sid)
	{
		if (!m_destroying)
		{
			// 不是由服务管理器显式释放的, 证明存在BUG
			// TODO
		}

		size_t index = static_cast<size_t>(sid);
		if (index >= _countof(m_serviceMap))
		{
			return E_FAIL; // TODO
		}

		ServiceInfo& si = m_serviceMap[index];
		si.destroyed = true;

		return S_OK;
	}

	~ServiceMgr()
	{
		// TODO 在servicemap中存在未释放的服务时警告
	}

private:
	ServiceMgr() : m_destroying(false)
	{
	}

	ServiceMgr(const ServiceMgr& rhs);
	ServiceMgr& operator=(const ServiceMgr& rhs);


private:
	struct ServiceInfo
	{
		const wchar_t* name;
		IUnknown* service;
		ServiceCreator creator;
		bool destroyed;
	};
	ServiceInfo m_serviceMap[256];

	bool m_destroying;
};


template <int ID>
class Service : public IUnknown
{
public:
	enum { ServiceID = ID };

	Service() : m_refCount(0)
	{
	}

	virtual ~Service()
	{
	}

	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return (ULONG) ::InterlockedIncrement(&m_refCount);
	}

	virtual ULONG STDMETHODCALLTYPE Release()
	{
		LONG ret = ::InterlockedDecrement(&m_refCount);
		if (ret == 0)
		{
			if (FAILED(ServiceMgr::Instance().NotifyServiceDestroy(ServiceID)))
			{
				return (ULONG)::InterlockedIncrement(&m_refCount);
			}
			else
			{
				delete this;	
			}
		}
		return (ULONG)ret;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		return E_NOTIMPL;
	}

private:
	LONG m_refCount;
};



#define DEFINE_SERVICE(classname, name) \
	template <typename T> struct ServiceInitHelper_##classname { \
		static HRESULT ServiceCreator_##classname(IUnknown** s) { *s = (IUnknown*) new classname(); return S_OK; 	} \
		ServiceInitHelper_##classname() { ServiceMgr::Instance().SetServiceInfo(classname::ServiceID, &ServiceCreator_##classname, name); } \
		static ServiceInitHelper_##classname<int> s_initializer; \
	}; \
	ServiceInitHelper_##classname<int> ServiceInitHelper_##classname<int>::s_initializer;

