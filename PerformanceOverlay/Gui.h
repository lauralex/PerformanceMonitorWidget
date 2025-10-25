/**
 * @file Gui.h
 * @brief Contains the declaration of the Gui class.
 * @author Alessandro Bellia
 * @date 10/22/2025
 */


#pragma once

#include "PerformanceMonitor.h"
#include <d3d11.h>

struct ImGuiContext;

/**
 * @class Gui
 * @brief Manages the Dear ImGui user interface.
 */
class Gui
{
public:
	/**
	 * @brief Constructs a new Gui object.
	 * @param[in] hwnd The handle to the window.
	 * @param[in] pDevice A pointer to the ID3D11Device.
	 * @param[in] pDeviceContext A pointer to the ID3D11DeviceContext.
	 * @param[in] pSwapChain A pointer to the IDXGISwapChain.
	 */
	Gui(
		_In_ HWND                 hwnd,
		_In_ ID3D11Device*        pDevice,
		_In_ ID3D11DeviceContext* pDeviceContext,
		_In_ IDXGISwapChain*      pSwapChain);
	~Gui();

	/**
	 * @brief Initializes the ImGui context, backends, and the Performance Monitor.
	 * @return True if initialization is successful, false otherwise.
	 */
	bool Initialize();

	/**
	 * @brief Shuts down the ImGui backends, context, and the Performance Monitor.
	 */
	void Shutdown();

	/**
	 * @brief Renders a single frame of the GUI.
	 */
	void Render();

private:
	/**
	 * @brief Renders the main performance overlay window.
	 */
	void RenderPerformanceWindow() const;


	HWND                    m_hWnd;
	ID3D11Device*           m_pDevice;
	ID3D11DeviceContext*    m_pDeviceContext;
	IDXGISwapChain*         m_pSwapChain;
	PerformanceMonitor      m_perfMonitor;
	ID3D11RenderTargetView* m_mainRenderTargetView;
};
