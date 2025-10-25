/**
 * @file D3D11Renderer.h
 * @brief Contains the declaration of the D3D11Renderer class.
 * @author Alessandro Bellia
 * @date 10/22/2025
 */

#pragma once

#include <d3d11.h>


/**
 * @class D3D11Renderer
 * @brief Manages the initialization and cleanup of the Direct3D11 device,
 *		  swap chain, and render target.
 */
class D3D11Renderer
{
public:
	D3D11Renderer() = default;
	~D3D11Renderer();
	D3D11Renderer(const D3D11Renderer& other)                = delete;
	D3D11Renderer(D3D11Renderer&& other) noexcept            = delete;
	D3D11Renderer& operator=(const D3D11Renderer& other)     = delete;
	D3D11Renderer& operator=(D3D11Renderer&& other) noexcept = delete;

	/**
	 * @brief Initializes the Direct3D device, swap chain, and render target.
	 * @param[in] hWnd The handle to the window that rendering will target.
	 * @return True if initialization is successful, false otherwise.
	 */
	bool Initialize(
		_In_ HWND hWnd);

	/**
	 * @brief Releases all allocated Direct3D resources
	 */
	void Shutdown();

	// --- Accessors ---

	/**
	 * @brief Gets the Direct3D device.
	 * @return A pointer to the ID3D11Device.
	 */
	[[nodiscard]] ID3D11Device* GetD3DDevice() const { return m_pD3dDevice; }


	/**
	 * @brief Gets the Direct3D device context.
	 * @return A pointer to the ID3D11DeviceContext.
	 */
	[[nodiscard]] ID3D11DeviceContext* GetD3DDeviceContext() const { return m_pD3dDeviceContext; }


	/**
	 * @brief Gets the DXGI Swap Chain.
	 * @return A pointer to the IDXGISwapChain.
	 */
	[[nodiscard]] IDXGISwapChain* GetSwapChain() const { return m_pSwapChain; }

private:
	/**
	 * @brief Creates the main render target view after the device is created.
	 */
	void CreateRenderTarget();

	/**
	 * @brief Cleans up the main render target view.
	 */
	void CleanupRenderTarget();

	ID3D11Device*           m_pD3dDevice           = nullptr;
	ID3D11DeviceContext*    m_pD3dDeviceContext    = nullptr;
	IDXGISwapChain*         m_pSwapChain           = nullptr;
	ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;
};
