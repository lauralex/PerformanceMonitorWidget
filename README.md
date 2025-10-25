# PerformanceMonitorWidget

A lightweight, always-on-top performance monitoring overlay for Windows that displays real-time system metrics directly on your desktop. Built from scratch using modern C++ with Direct3D 11 rendering and WMI integration.

## Overview

PerformanceMonitorWidget creates a transparent, click-through overlay that persistently displays CPU, memory, and disk usage statistics in the corner of your screen. The widget leverages the Desktop Window Manager (DWM) for acrylic transparency effects and integrates deeply with Windows Management Instrumentation for accurate performance data collection.

Unlike traditional system monitors that require switching between windows or occupy valuable taskbar space, this overlay remains visible across all applications without interfering with your workflow. The rendering pipeline is optimized for minimal resource consumption, making it ideal for monitoring system load during gaming, development, or any resource-intensive tasks.

## Features

- **Real-time Performance Monitoring**: Continuous tracking of CPU utilization, memory usage, and disk activity
- **Transparent Overlay**: Utilizes DWM's acrylic blur effect for a modern, semi-transparent appearance
- **Non-intrusive Design**: Click-through window that doesn't interfere with underlying applications
- **System Tray Integration**: Minimizes to system tray with context menu for quick access
- **Always-on-Top**: Ensures metrics remain visible across all workspaces and applications
- **Low Resource Footprint**: Efficient polling mechanism with optimized Direct3D rendering

## Technical Architecture

### Rendering Pipeline

The application employs a Direct3D 11 rendering pipeline with custom integration of Dear ImGui for the user interface. The rendering architecture consists of several key components:

- **D3D11Renderer**: Manages device initialization, swap chain creation, and render target management
- **Gui**: Orchestrates ImGui frame rendering and integrates with the D3D11 backend
- **Custom Shadow Rendering**: Multi-layered shadow implementation for enhanced visual depth

The window is created with specific extended styles (`WS_EX_TOPMOST`, `WS_EX_TRANSPARENT`, `WS_EX_LAYERED`, `WS_EX_TOOLWINDOW`) to achieve the overlay behavior. The `WS_EX_TRANSPARENT` flag allows mouse events to pass through the window, while `WS_EX_LAYERED` enables alpha blending through `SetLayeredWindowAttributes`.

### Performance Data Collection

Performance metrics are retrieved through the WMI (Windows Management Instrumentation) API, specifically utilizing the `Win32_PerfFormattedData_PerfOS_Processor` and `Win32_PerfFormattedData_PerfDisk_PhysicalDisk` classes. The implementation follows COM initialization patterns:

1. **COM Initialization**: Establishes COM security and authentication levels
2. **WMI Connection**: Connects to the `ROOT\CIMV2` namespace via `IWbemLocator`
3. **Query Execution**: Executes WQL (WMI Query Language) queries for performance counters
4. **Data Retrieval**: Parses `VARIANT` types from WMI result sets

Key WQL queries used:
- CPU Load: `SELECT PercentProcessorTime FROM Win32_PerfFormattedData_PerfOS_Processor WHERE Name='_Total'`
- Memory Usage: `SELECT TotalVisibleMemorySize, FreePhysicalMemory FROM Win32_OperatingSystem`
- Disk Activity: `SELECT PercentDiskTime FROM Win32_PerfFormattedData_PerfDisk_PhysicalDisk WHERE Name='_Total'`

### Window Management

The main window procedure (`WndProc`) handles several critical operations:

- **ImGui Message Processing**: Routes Win32 messages to ImGui for input handling
- **Tray Icon Events**: Processes custom `WM_APP_TRY_MSG` messages for system tray interactions
- **Context Menu**: Implements right-click context menu for application control

The system tray integration uses `Shell_NotifyIcon` with `NOTIFYICONDATA` structures to add, remove, and manage tray icon events.

### Transparency Implementation

The overlay achieves its transparent background through a combination of techniques:

1. **DWM Frame Extension**: `DwmExtendFrameIntoClientArea` with negative margins extends the blur region across the entire client area
2. **Layered Window Attributes**: `SetLayeredWindowAttributes` with `LWA_ALPHA` flag for alpha channel support
3. **Clear Color Alpha**: Rendering with a clear color of `(0, 0, 0, 0)` ensures complete transparency where nothing is drawn

## Build Requirements

- **Visual Studio 2022** (or later with C++17 support)
- **Windows SDK 10.0** or higher
- **Direct3D 11** runtime libraries
- Required Windows libraries:
  - `d3d11.lib` - Direct3D 11 graphics
  - `dwmapi.lib` - Desktop Window Manager API
  - `wbemuuid.lib` - WMI object identifiers
  - `shell32.lib` - Shell API for system tray

## Project Structure

```
PerformanceMonitorWidget/
├── PerformanceOverlay/
│   ├── main.cpp                    # Application entry point and window management
│   ├── D3D11Renderer.cpp/.h        # Direct3D device and swap chain management
│   ├── Gui.cpp/.h                  # ImGui integration and UI rendering
│   ├── PerformanceMonitor.cpp/.h   # WMI integration and metrics collection
│   ├── PerformanceOverlay.vcxproj  # Visual Studio project file
│   └── resource.h                  # Windows resource definitions
├── libs/
│   └── imgui/                      # Dear ImGui library (v1.89+)
├── PerformanceOverlay.slnx         # Solution file
└── README.md
```

## Building the Project

1. Clone the repository:
   ```bash
   git clone https://github.com/lauralex/PerformanceMonitorWidget.git
   cd PerformanceMonitorWidget
   ```

2. Open `PerformanceOverlay.slnx` in Visual Studio 2022 or later

3. Configure build settings:
   - Platform: x64 (recommended) or x86
   - Configuration: Release (for optimal performance) or Debug

4. Build the solution (Ctrl+Shift+B)

5. The executable will be generated in:
   - Release: `./x64/Release/PerformanceOverlay.exe`
   - Debug: `./x64/Debug/PerformanceOverlay.exe`

## Usage

Run the compiled executable. The overlay will appear in the top-right corner of your screen displaying:
- **CPU**: Processor utilization percentage
- **MEM**: Physical memory usage percentage
- **DISK**: Disk activity percentage

The application runs with a system tray icon. Right-click the tray icon to access the exit option.

## Implementation Details

### PerformanceMonitor Class

The `PerformanceMonitor` class encapsulates all WMI operations and maintains connection state:

- **RAII Pattern**: Constructor/destructor manage COM initialization and cleanup
- **Interface Pointers**: Maintains `IWbemLocator` and `IWbemServices` for WMI access
- **Error Handling**: Comprehensive HRESULT checking throughout the WMI call chain
- **Property Extraction**: Generic `GetWmiPropertyValue` template for retrieving various metric types

### D3D11Renderer Class

Manages the Direct3D rendering context with proper resource lifecycle management:

- **Device Creation**: Initializes D3D11 device with hardware acceleration flags
- **Swap Chain Configuration**: Configures DXGI swap chain with BGRA format and flip model
- **Render Target Management**: Automatically creates and destroys render target views on resize
- **Resource Cleanup**: Ensures proper COM reference counting to prevent memory leaks

### Gui Class

Coordinates the rendering loop and integrates ImGui with Direct3D:

- **Frame Management**: Orchestrates ImGui frame lifecycle (NewFrame → Render → EndFrame)
- **Backend Integration**: Properly initializes Win32 and DirectX 11 backends for ImGui
- **Style Customization**: Applies custom color schemes for the "geek aesthetic"
- **Shadow Effects**: Implements layered shadow rendering for visual enhancement

## Performance Considerations

The application is designed for minimal overhead:

- **VSync-Limited Rendering**: Swap chain presents at monitor refresh rate to conserve resources
- **Efficient WMI Polling**: Performance queries are executed once per frame, not per metric
- **No Thread Synchronization**: Single-threaded architecture eliminates synchronization overhead
- **Optimized ImGui Configuration**: Minimal draw calls through batch rendering

Typical resource usage:
- CPU: <1% on modern processors
- Memory: ~10-15 MB working set
- GPU: Negligible (simple 2D rendering)

## Customization

The overlay appearance can be customized by modifying `Gui::RenderPerformanceWindow()`:

- **Position**: Adjust `ImGui::SetNextWindowPos()` anchor point
- **Size**: Modify `ImGui::SetNextWindowSize()` dimensions
- **Colors**: Change `ImGui::PushStyleColor()` values for different color schemes
- **Refresh Rate**: Adjust VSync parameter in `Present()` call (currently locked to monitor refresh)

## Known Limitations

- Windows-only (relies on Win32 API, DWM, and WMI)
- Requires administrator privileges for certain performance counters on some systems
- Single monitor support (displays on primary monitor only)
- Fixed position overlay (not user-draggable in current implementation)

## Dependencies

- **Dear ImGui** (v1.89.9 or later): Immediate mode GUI library
  - `imgui`, `imgui_impl_win32`, `imgui_impl_dx11`
- **Windows API**: Win32, COM, WMI, Desktop Window Manager
- **DirectX**: Direct3D 11, DXGI

All dependencies are either included in the repository or part of the Windows SDK.

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for full details.

## Author

Alessandro Bellia - October 2025

## Acknowledgments

- Dear ImGui by Omar Cornut for the excellent immediate mode GUI framework
- Microsoft for comprehensive WMI and DirectX documentation
- The Windows development community for transparent overlay techniques
