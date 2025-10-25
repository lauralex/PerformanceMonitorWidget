/**
 * @file PerformanceMonitor.cpp
 * @brief Contains the implementation of the PerformanceMonitor class.
 * @author Alessandro Bellia
 * @date 10/22/2025
 */

#include "PerformanceMonitor.h"
#include <comdef.h>

PerformanceMonitor::PerformanceMonitor()
	: m_pLocator(nullptr),
	  m_pServices(nullptr),
	  m_cpuLoad(0.0f),
	  m_memoryUsage(0.0f),
	  m_diskUsage(0.0f)
{
}
PerformanceMonitor::~PerformanceMonitor()
{
	Shutdown();
}

bool PerformanceMonitor::Initialize()
{
	// Step 1: Initialize COM.
	HRESULT hRes = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hRes))
	{
		return false; // "Failed to initialize COM library"
	}

	// Step 2: Set general COM security levels.
	hRes = ::CoInitializeSecurity(
		nullptr,
		-1,                          // COM Authentication
		nullptr,                     // Authentication Services
		nullptr,                     // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default Authentication
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
		nullptr,                     // Authentication Info
		EOAC_NONE,                   // Additional Capabilities
		nullptr                      // Reserved
	);

	if (FAILED(hRes))
	{
		::CoUninitialize();
		return false; // "Failed to initialize security"
	}

	// Step 3: Obtain the initial locator to WMI.
	hRes = ::CoCreateInstance(
		CLSID_WbemLocator,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, reinterpret_cast<LPVOID*>(&m_pLocator)
	);

	if (FAILED(hRes))
	{
		::CoUninitialize();
		return false; // "Failed to create IWbemLocator object"
	}

	// Step 4: Connect to WMI through IWbemLocator::ConnectServer method
	hRes = m_pLocator->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		nullptr,                 // Username. NULL = current user
		nullptr,                 // User password. NULL = current
		nullptr,                 // Locale. NULL indicates current
		NULL,                    // Security flags.
		nullptr,                 // Authority (e.g., Kerberos)
		nullptr,                 // Context object
		&m_pServices             // Pointer to IWbemServices proxy
	);

	if (FAILED(hRes))
	{
		m_pLocator->Release();
		::CoUninitialize();
		return false; // "Could not connect"
	}

	// Step 5: Set security levels on the proxy.
	hRes = ::CoSetProxyBlanket(
		m_pServices,                 // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		nullptr,                     // Server principal name
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		nullptr,                     // Client identity
		EOAC_NONE                    // Proxy capabilities
	);

	if (FAILED(hRes))
	{
		m_pServices->Release();
		m_pLocator->Release();
		::CoUninitialize();
		return false; // "Could not set proxy blanket"
	}

	return true;
}

void PerformanceMonitor::Shutdown()
{
	if (m_pServices)
	{
		m_pServices->Release();
		m_pServices = nullptr;
	}

	if (m_pLocator)
	{
		m_pLocator->Release();
		m_pLocator = nullptr;
	}

	::CoUninitialize();
}

void PerformanceMonitor::Update()
{
	if (!m_pServices)
	{
		return;
	}

	// Update the CPU load
	ULONG cpuLoadPercent = 0;
	if (GetWmiPropertyValue(
		bstr_t(L"SELECT PercentProcessorTime FROM Win32_PerfFormattedData_PerfOS_Processor WHERE Name='_Total'"),
		L"PercentProcessorTime",
		cpuLoadPercent))
	{
		m_cpuLoad = static_cast<float>(cpuLoadPercent);
	}

	// Update memory usage
	// For memory, we need to query two properties from the same object
	IEnumWbemClassObject* pEnumerator = nullptr;
	const HRESULT         hRes        = m_pServices->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT TotalVisibleMemorySize, FreePhysicalMemory FROM Win32_OperatingSystem"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		nullptr,
		&pEnumerator);

	if (SUCCEEDED(hRes) && pEnumerator)
	{
		IWbemClassObject* pClsObj = nullptr;
		ULONG             uReturn = 0;
		if (pEnumerator->Next(WBEM_INFINITE, 1, &pClsObj, &uReturn) == WBEM_S_NO_ERROR)
		{
			VARIANT vtProp{};
			(void)pClsObj->Get(L"TotalVisibleMemorySize", 0, &vtProp, nullptr, nullptr);
			const ULONG totalMemory = _wtol(vtProp.bstrVal);
			(void)::VariantClear(&vtProp);

			(void)pClsObj->Get(L"FreePhysicalMemory", 0, &vtProp, nullptr, nullptr);
			const ULONG freeMemory = _wtol(vtProp.bstrVal);
			(void)::VariantClear(&vtProp);

			pClsObj->Release();

			if (totalMemory > 0)
			{
				m_memoryUsage = (static_cast<float>(totalMemory - freeMemory) / static_cast<float>(totalMemory)) *
					100.0f;
			}
		}

		pEnumerator->Release();
	}

	// Update disk usage
	ULONG diskUsagePercent = 0;
	if (GetWmiPropertyValue(
		bstr_t(L"SELECT PercentDiskTime FROM Win32_PerfFormattedData_PerfDisk_PhysicalDisk WHERE Name='_Total'"),
		L"PercentDiskTime",
		diskUsagePercent))
	{
		m_diskUsage = static_cast<float>(diskUsagePercent);
	}
}

_Use_decl_annotations_
bool PerformanceMonitor::GetWmiPropertyValue(
	const BSTR     wqlQuery,
	const wchar_t* propertyName,
	ULONG&         value) const
{
	IEnumWbemClassObject* pEnumerator = nullptr;
	HRESULT               hRes        = m_pServices->ExecQuery(
		bstr_t("WQL"),
		wqlQuery,
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		nullptr,
		&pEnumerator);

	if (FAILED(hRes))
	{
		return false;
	}

	IWbemClassObject* pClsObj = nullptr;
	ULONG             uReturn = 0;
	bool              success = false;

	if (pEnumerator->Next(WBEM_INFINITE, 1, &pClsObj, &uReturn) == WBEM_S_NO_ERROR)
	{
		VARIANT vtProp;

		// Get the value of the specified property
		hRes = pClsObj->Get(propertyName, 0, &vtProp, nullptr, nullptr);
		if (SUCCEEDED(hRes))
		{
			// WMI performance properties can be returned as strings, which need converting.
			if (vtProp.vt == VT_BSTR)
			{
				value   = _wtol(vtProp.bstrVal);
				success = true;
			}
			else if (vtProp.vt == VT_I4)
			{
				value   = vtProp.ulVal;
				success = true;
			}
			(void)VariantClear(&vtProp);
		}
		pClsObj->Release();
	}

	pEnumerator->Release();
	return success;
}
