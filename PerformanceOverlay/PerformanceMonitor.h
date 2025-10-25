/**
 * @file PerformanceMonitor.h
 * @brief Contains the declaration of the PerformanceMonitor class.
 * @author Alessandro Bellia
 * @date 10/22/2025
 */

#pragma once

#include <WbemIdl.h>

/**
 * @class PerformanceMonitor
 * @brief Establishes a connection to WMI and queries for performance data such as
 *		  CPU load, memory usage, and disk activity.
 */
class PerformanceMonitor
{
public:
	PerformanceMonitor();
	~PerformanceMonitor();

	PerformanceMonitor(const PerformanceMonitor& other)     = delete;
	PerformanceMonitor(PerformanceMonitor&& other) noexcept = delete;

	PerformanceMonitor& operator=(const PerformanceMonitor& other)     = delete;
	PerformanceMonitor& operator=(PerformanceMonitor&& other) noexcept = delete;
	/**
	 * @brief Initializes COM and connects to the WMI service.
	 * @return True if initialization and connection are successful, false otherwise.
	 */
	bool Initialize();

	/**
	 * @brief Shuts down all COM interfaces and uninitializes COM.
	 */
	void Shutdown();

	/**
	 * @brief Executes WMI queries to refresh all performance data.
	 */
	void Update();


	// --- Accessors ---

	/**
	 * @brief Gets the current CPU load percentage.
	 * @return The CPU load as a float.
	 */
	[[nodiscard]] float GetCpuLoad() const { return m_cpuLoad; }

	/**
	 * @brief Gets the current memory usage percentage.
	 * @return The memory usage as a float.
	 */
	[[nodiscard]] float GetMemoryUsage() const { return m_memoryUsage; }

	/**
	 * @brief Gets the current disk activity percentage.
	 * @return The disk activity as a float.
	 */
	[[nodiscard]] float GetDiskUsage() const { return m_diskUsage; }

private:
	_Success_(return)
	/**
	 * @brief A helper function to execute a WMI query and retrieve a single property value.
	 * @param[in] wqlQuery The WQL query string.
	 * @param[in] propertyName The name of the property to retrieve from the result.
	 * @param[out] value The retrieved value is stored here.
	 * @return True if the query and value retrieval were successful, false otherwise.
	 */
	bool GetWmiPropertyValue(
		_In_ BSTR           wqlQuery,
		_In_ const wchar_t* propertyName,
		_Out_ ULONG&        value) const;

	IWbemLocator*  m_pLocator;
	IWbemServices* m_pServices;

	float m_cpuLoad;
	float m_memoryUsage;
	float m_diskUsage;
};
