/**
 * @file Gui.cpp
 * @brief Contains the implementation of the Gui class.
 * @author Alessandro Bellia
 * @date 10/22/2025
 */


#include "Gui.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <string>


/**
 * @brief Renders a soft, multi-layered shadow behind a rectangle.
 * @param[in] pos The top-left position of the rectangle.
 * @param[in] size The width and height of the rectangle.
 * @param[in] color The base color of the shadow (alpha will be modulated).
 * @param[in] thickness The thickness or spread of the shadow
 */
static void RenderShadow(
	_In_ const ImVec2& pos,
	_In_ const ImVec2& size,
	_In_ ImU32         color,
	_In_ float         thickness);


_Use_decl_annotations_
Gui::Gui(
	const HWND           hwnd,
	ID3D11Device*        pDevice,
	ID3D11DeviceContext* pDeviceContext,
	IDXGISwapChain*      pSwapChain)
	: m_hWnd(hwnd),
	  m_pDevice(pDevice),
	  m_pDeviceContext(pDeviceContext),
	  m_pSwapChain(pSwapChain),
	  m_mainRenderTargetView(nullptr)
{
}

Gui::~Gui()
{
	Shutdown();
}

bool Gui::Initialize()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends 
	if (!ImGui_ImplWin32_Init(m_hWnd) ||
		!ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext))
	{
		return false;
	}

	if (!m_perfMonitor.Initialize())
	{
		return false; // Failed to initialize WMI
	}

	// Create render target
	ID3D11Texture2D* pBackBuffer;
	(void)m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	(void)m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
	pBackBuffer->Release();

	return true;
}

void Gui::Shutdown()
{
	m_perfMonitor.Shutdown();

	if (m_mainRenderTargetView)
	{
		m_mainRenderTargetView->Release();
		m_mainRenderTargetView = nullptr;
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	if (ImGui::GetCurrentContext())
	{
		ImGui::DestroyContext();
	}
}

void Gui::Render()
{
	// Pool for new performance data
	m_perfMonitor.Update();

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	// Render the main overlay window
	RenderPerformanceWindow();

	// Rendering
	// The clear color must have 0 alpha for the DWM Acrylic effect to be visible.
	constexpr float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_pDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
	m_pDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, clearColor);
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	(void)m_pSwapChain->Present(1, 0); // Present with vsync
}

void Gui::RenderPerformanceWindow() const
{
	// Set styles for a more "geek" look
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.10f, 0.2f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.2f, 0.2f));
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.7f, 0.3f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

	// Position the window at the top-right of the screen
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(
		ImVec2(mainViewport->WorkPos.x + mainViewport->WorkSize.x - 20, mainViewport->WorkPos.y + 20),
		ImGuiCond_Always, ImVec2(1.0f, 0.0f));

	ImGui::SetNextWindowSize(ImVec2(250, 0));

	// Define window flags for a simple, non-interactive overlay
	constexpr ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove;

	ImGui::Begin("System Performance", nullptr, windowFlags);

	// --- Render Shadow ---
	// This is drawn before the content to appear behind it.
	RenderShadow(ImGui::GetWindowPos(), ImGui::GetWindowSize(), IM_COL32(0, 0, 0, 100), 10.0f);

	// --- CPU Usage ---
	const float cpu = m_perfMonitor.GetCpuLoad();
	char        cpuBuf[32];
	(void)sprintf_s(cpuBuf, "%.1f%%", cpu);
	ImGui::Text("CPU");
	ImGui::ProgressBar(cpu / 100.0f, ImVec2(-1.0f, 0.0f), cpuBuf); // Show CPU usage bar
	// CPU usage history graph
	static float cpuHistory[90] = {}; // 90 frames of history
	static int   historyIndex   = 0;
	static bool  filled         = false;
	// Update history buffer
	cpuHistory[historyIndex] = cpu;
	if (historyIndex == 89)
	{
		filled = true;
	}
	historyIndex = (historyIndex + 1) % 90;
	// Render graph
	ImGui::PlotLines("", cpuHistory, 90, filled ? historyIndex : 0, "CPU Graph", 0.0f, 100.0f,
	                 ImVec2(-1.0f, 50.0f));

	ImGui::Spacing();

	// --- Memory Usage ---
	const float mem = m_perfMonitor.GetMemoryUsage();
	char        memBuf[32];
	(void)sprintf_s(memBuf, "%.1f%%", mem);
	ImGui::Text("MEM");
	ImGui::ProgressBar(mem / 100.0f, ImVec2(-1.0f, 0.0f), memBuf); // Show Memory usage bar

	ImGui::Spacing();

	// --- Disk Usage ---
	const float disk = m_perfMonitor.GetDiskUsage();
	char        diskBuf[32];
	(void)sprintf_s(diskBuf, "%.1f%%", disk);
	ImGui::Text("DISK");
	ImGui::ProgressBar(disk / 100.0f, ImVec2(-1.0f, 0.0f), diskBuf); // Show Disk usage bar

	ImGui::End();

	// Restore default styles
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(4);
}


_Use_decl_annotations_
static void RenderShadow(
	const ImVec2& pos,
	const ImVec2& size,
	const ImU32   color,
	const float   thickness)
{
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	const float rounding = ImGui::GetStyle().WindowRounding;

	// Draw several layers of rectangles with increasing transparency and size
	// to create a soft "penumbra" effect.
	for (int i = 0; i < 4; i++)
	{
		const float shadowAlpha    = static_cast<float>((color >> 24) & 0xFF) / 255.0f;
		const float modulatedAlpha = shadowAlpha * (1.0f - static_cast<float>(i) / 4.0f) * 0.5f;

		const ImU32 shadowColor = (color & 0x00FFFFFF) | (static_cast<ImU32>(modulatedAlpha * 255.0f) << 24);
		const float spread      = thickness * (static_cast<float>(i) / 4.0f);

		drawList->AddRectFilled(
			ImVec2(pos.x - spread, pos.y - spread),
			ImVec2(pos.x + size.x + spread, pos.y + size.y + spread),
			shadowColor,
			rounding + spread
		);
	}
}
