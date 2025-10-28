#pragma once

#include "include/cef_base.h"
#include "include/views/cef_window.h"

// Cross-platform native window controls interface
class NativeWindowControls {
public:
    static void SetupNativeControls(CefRefPtr<CefWindow> window);
    static void UpdateControlsVisibility(CefRefPtr<CefWindow> window, bool show);
    static void SetControlsPosition(CefRefPtr<CefWindow> window, int x, int y, int width, int height);
    
private:
    static void SetupWindowsControls(CefRefPtr<CefWindow> window);
    static void SetupMacOSControls(CefRefPtr<CefWindow> window);
    static void SetupLinuxControls(CefRefPtr<CefWindow> window);
};