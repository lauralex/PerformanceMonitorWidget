/**
 * @file D3D11Renderer.cpp
 * @brief Contains the implementation of the D3D11Renderer class.
 * @author Alessandro Bellia
 * @date 10/22/2025
 */

#include "D3D11Renderer.h"


D3D11Renderer::~D3D11Renderer()
{
	Shutdown();
}

_Use_decl_annotations_
bool D3D11Renderer::Initialize(
	const HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount                        = 2;
	sd.BufferDesc.Width                   = 0;
	sd.BufferDesc.Height                  = 0;
	sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator   = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow                       = hWnd;
	sd.SampleDesc.Count                   = 1;
	sd.SampleDesc.Quality                 = 0;
	sd.Windowed                           = TRUE;
	sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

	constexpr UINT createDeviceFlags = 0;
	// createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL           featureLevel;
	constexpr D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
	const HRESULT               res                  = ::D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		featureLevelArray,
		2,
		D3D11_SDK_VERSION,
		&sd,
		&m_pSwapChain,
		&m_pD3dDevice,
		&featureLevel,
		&m_pD3dDeviceContext
	);

	if (FAILED(res))
	{
		return false;
	}

	CreateRenderTarget();
	return true;
}

void D3D11Renderer::Shutdown()
{
	CleanupRenderTarget();

	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}
	if (m_pD3dDeviceContext)
	{
		m_pD3dDeviceContext->Release();
		m_pD3dDeviceContext = nullptr;
	}
	if (m_pD3dDevice)
	{
		m_pD3dDevice->Release();
		m_pD3dDevice = nullptr;
	}
}

void D3D11Renderer::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (pBackBuffer)
	{
		m_pD3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
		pBackBuffer->Release();
	}
}

void D3D11Renderer::CleanupRenderTarget()
{
	if (m_mainRenderTargetView)
	{
		m_mainRenderTargetView->Release();
		m_mainRenderTargetView = nullptr;
	}
}
