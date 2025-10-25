/**
 * @file main.cpp
 * @brief Entry point for the performance overlay application.
 * 
 * @author Alessandro Bellia
 * @date 10/22/2025
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#include "D3D11Renderer.h"
#include "Gui.h"
#include "resource.h"
#include "imgui.h"


// --- Defines for Tray Icon ---
#define WM_APP_TRY_MSG (WM_APP + 1) // Custom message for Tray Icon events
#define TRAY_ICON_ID 1				// Unique ID for Tray Icon
#define IDM_EXIT 1001				// Menu item ID for "Exit"


/**
 * @brief The main window procedure.
 * @param[in] hWnd A handle to the window.
 * @param[in] msg The message.
 * @param[in] wParam Additional message information.
 * @param[in] lParam Additional message information.
 * @return The result of the message processing.
 */
static LRESULT CALLBACK WndProc(
	_In_ HWND   hWnd,
	_In_ UINT   msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

/**
 * @brief Adds a tray icon to the system tray.
 * 
 * @param[in] hWnd A handle to the window.
 * @param[in] hInst A handle to the application instance.
 * @return void
 */
static void AddTrayIcon(
	_In_ HWND      hWnd,
	_In_ HINSTANCE hInst);

/**
 * @brief Removes the tray icon from the system tray.
 * 
 * @param[in] hWnd A handle to the window.
 * @return void
 */
static void RemoveTrayIcon(
	_In_ HWND hWnd);

/**
 * @brief Shows the context menu for the tray icon.
 * 
 * @param[in] hWnd A handle to the window.
 * @return void
 */
static void ShowContextMenu(
	_In_ HWND hWnd);

/**
 * @brief The entrypoint of the Windows application.
 * @param[in] hInstance A handle to the current instance of the application.
 * @param[in] hPrevInstance A handle to the previous instance of the application. This is always NULL.
 * @param[in] lpCmdLine The command line for the application, excluding the program name.
 * @param[in] nShowCmd Controls how the window is to be shown.
 * @return If the function succeeds, terminating when it receives a WM_QUIT message, it should return the exit value
 *		   contained in that message's wParam parameter. If the function terminates before entering the message loop, it should return
 *		   zero.
 */
int WINAPI WinMain(
	_In_ const HINSTANCE     hInstance,
	_In_opt_ const HINSTANCE hPrevInstance,
	_In_ const LPSTR         lpCmdLine,
	_In_ const int           nShowCmd)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

	// Create the application window
	// WS_EX_TOPMOST: Ensures the window is always on top.
	// WS_EX_TRANSPARENT: Allows mouse events to "fall through" the window.
	// WS_POPUP: Creates a borderless window.
	const WNDCLASSEX wc = {
		sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
		GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
		L"PerfOverlay",
		nullptr
	};

	::RegisterClassEx(&wc);
	const HWND hwnd = ::CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		wc.lpszClassName, L"Performance Overlay",
		WS_POPUP,
		0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN),
		nullptr, nullptr, wc.hInstance, nullptr);

	if (!hwnd)
	{
		return 1;
	}

	// --- Setup Tray Icon ---
	AddTrayIcon(hwnd, wc.hInstance);

	// --- Enable Acrylic "Glass" Effect ---
	// This requires linking against dwmapi.lib
	// This modern approach replaces the old LWA_COLORKEY transparency method.
	constexpr MARGINS margins{-1};
	(void)::DwmExtendFrameIntoClientArea(hwnd, &margins);

	// Set the window to be fully transparent where it is black.
	// Since our clear color is black, this makes the background invisible.
	::SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

	// Initialize Direct3D
	D3D11Renderer renderer;
	if (!renderer.Initialize(hwnd))
	{
		renderer.Shutdown();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}


	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui
	Gui gui(hwnd, renderer.GetD3DDevice(), renderer.GetD3DDeviceContext(), renderer.GetSwapChain());
	if (!gui.Initialize())
	{
		// Handle initialization failure
		gui.Shutdown();
		renderer.Shutdown();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}


	// Main loop
	MSG msg;
	msg.message = WM_NULL;
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE) != 0)
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			gui.Render();
		}
	}

	// Cleanup
	//gui.Shutdown();
	//renderer.Shutdown();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return static_cast<int>(msg.wParam);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND   hWnd,
	UINT   msg,
	WPARAM wParam,
	LPARAM lParam);

_Use_decl_annotations_
LRESULT CALLBACK WndProc(
	const HWND   hWnd,
	const UINT   msg,
	const WPARAM wParam,
	const LPARAM lParam)
{
	// Allow ImGui to process messages.
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}

	switch (msg)
	{
	case WM_APP_TRY_MSG:
		switch (lParam)
		{
		case WM_RBUTTONUP:
			ShowContextMenu(hWnd);
			break;
		}
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		}
		return 0;
	case WM_DESTROY:
		RemoveTrayIcon(hWnd);
		::PostQuitMessage(0);
		return 0;
	}

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

_Use_decl_annotations_
void AddTrayIcon(
	const HWND      hWnd,
	const HINSTANCE hInst)
{
	NOTIFYICONDATA nid   = {};
	nid.cbSize           = sizeof(NOTIFYICONDATA);
	nid.hWnd             = hWnd;
	nid.uID              = TRAY_ICON_ID;
	nid.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_APP_TRY_MSG;
	nid.hIcon            = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	wcscpy_s(nid.szTip, L"Performance Overlay");

	// Add the icon to the system tray
	::Shell_NotifyIcon(NIM_ADD, &nid);
}

_Use_decl_annotations_
void RemoveTrayIcon(
	const HWND hWnd)
{
	NOTIFYICONDATA nid = {};
	nid.cbSize         = sizeof(NOTIFYICONDATA);
	nid.hWnd           = hWnd;
	nid.uID            = TRAY_ICON_ID;
	// Remove the icon from the system tray
	::Shell_NotifyIcon(NIM_DELETE, &nid);
}

_Use_decl_annotations_
void ShowContextMenu(
	const HWND hWnd)
{
	POINT pt;
	GetCursorPos(&pt);
	const HMENU hMenu = ::CreatePopupMenu();
	if (hMenu)
	{
		::InsertMenu(hMenu, -1, MF_BYPOSITION, IDM_EXIT, L"Exit");
		// Set the foreground window to ensure the menu is displayed correctly
		::SetForegroundWindow(hWnd);
		// Display the context menu at the cursor position
		::TrackPopupMenu(
			hMenu,
			TPM_BOTTOMALIGN | TPM_LEFTALIGN,
			pt.x,
			pt.y,
			0,
			hWnd,
			nullptr);
		::DestroyMenu(hMenu);
	}
}
