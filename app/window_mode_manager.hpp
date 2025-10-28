#pragma once

#include <string>
#include "include/views/cef_window.h"
#include "config.hpp"

// Cross-platform window mode manager
class WindowModeManager {
public:
    // Initialization
    static void Initialize();
    
    // Mode management
    static void ApplyWindowMode(CefRefPtr<CefWindow> window, WindowMode mode);
    static void ToggleWindowMode(CefRefPtr<CefWindow> window);
    static WindowMode GetCurrentMode(CefRefPtr<CefWindow> window);
    static void SetWindowedMode(CefRefPtr<CefWindow> window);
    static void SetBorderlessMode(CefRefPtr<CefWindow> window);
    
    // Utility methods
    static bool IsWindowed();
    static bool IsBorderless();
    static std::string GetModeString();
    static bool CanToggleMode();
    
    // State persistence
    static void SaveWindowState(CefRefPtr<CefWindow> window);
    static void RestoreWindowState(CefRefPtr<CefWindow> window);
    
private:
    // Platform-specific implementations
    static void ApplyWindowsMode(CefRefPtr<CefWindow> window, WindowMode mode);
    static void ApplyMacOSMode(CefRefPtr<CefWindow> window, WindowMode mode);
    static void ApplyLinuxMode(CefRefPtr<CefWindow> window, WindowMode mode);
    
    // State management
    static WindowMode current_mode_;
    static bool initialized_;
};