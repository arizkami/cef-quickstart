#include "window_mode_manager.hpp"
#include "logger.hpp"
#include "config.hpp"

#ifdef _WIN32
    #include <windows.h>
    #include <dwmapi.h>
    #pragma comment(lib, "dwmapi.lib")
#elif defined(__APPLE__)
    #include <Cocoa/Cocoa.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
    #include <X11/Xutil.h>
    #ifdef CEF_USE_GTK
        #include <gtk/gtk.h>
        #include <gdk/gdk.h>
        #include <gdk/gdkx.h>
    #endif
#endif

// Static member initialization - FORCE BORDERLESS MODE
WindowMode WindowModeManager::current_mode_ = WindowMode::BORDERLESS;
bool WindowModeManager::initialized_ = false;

void WindowModeManager::Initialize() {
    if (initialized_) return;
    
    // Force borderless mode - ignore config
    current_mode_ = WindowMode::BORDERLESS;
    
    initialized_ = true;
    Logger::LogMessage("WindowModeManager: Initialized with FORCED borderless mode");
}

void WindowModeManager::ApplyWindowMode(CefRefPtr<CefWindow> window, WindowMode mode) {
    if (!window) {
        Logger::LogMessage("WindowModeManager: Invalid window reference");
        return;
    }

    if (!initialized_) {
        Initialize();
    }

    if (current_mode_ == mode) {
        Logger::LogMessage("WindowModeManager: Already in requested mode");
        return;
    }

    current_mode_ = mode;
    std::string mode_str = (mode == WindowMode::WINDOWED) ? "windowed" : "borderless";
    Logger::LogMessage("WindowModeManager: Applying " + mode_str + " mode");

    // Note: For now, we don't persist the mode change
    // This could be implemented with a config file or registry in the future

#ifdef _WIN32
    ApplyWindowsMode(window, mode);
#elif defined(__APPLE__)
    ApplyMacOSMode(window, mode);
#elif defined(__linux__)
    ApplyLinuxMode(window, mode);
#endif
}

void WindowModeManager::ToggleWindowMode(CefRefPtr<CefWindow> window) {
    WindowMode newMode = (current_mode_ == WindowMode::WINDOWED) ? 
        WindowMode::BORDERLESS : WindowMode::WINDOWED;
    ApplyWindowMode(window, newMode);
}

WindowMode WindowModeManager::GetCurrentMode(CefRefPtr<CefWindow> window) {
    if (!initialized_) {
        Initialize();
    }
    return current_mode_;
}

bool WindowModeManager::IsWindowed() {
    return current_mode_ == WindowMode::WINDOWED;
}

bool WindowModeManager::IsBorderless() {
    return current_mode_ == WindowMode::BORDERLESS;
}

std::string WindowModeManager::GetModeString() {
    return (current_mode_ == WindowMode::WINDOWED) ? "windowed" : "borderless";
}

void WindowModeManager::SetWindowedMode(CefRefPtr<CefWindow> window) {
    ApplyWindowMode(window, WindowMode::WINDOWED);
}

void WindowModeManager::SetBorderlessMode(CefRefPtr<CefWindow> window) {
    ApplyWindowMode(window, WindowMode::BORDERLESS);
}

void WindowModeManager::ApplyWindowsMode(CefRefPtr<CefWindow> window, WindowMode mode) {
#ifdef _WIN32
    HWND hwnd = window->GetWindowHandle();
    if (!hwnd || !IsWindow(hwnd)) {
        Logger::LogMessage("Windows: Invalid window handle");
        return;
    }

    // Store current window position and size
    RECT rect;
    GetWindowRect(hwnd, &rect);
    
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (style == 0 || exStyle == 0) {
        Logger::LogMessage("Windows: Failed to get window styles");
        return;
    }

    if (mode == WindowMode::WINDOWED) {
        // Windowed mode: Show title bar and borders
        style |= (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
        exStyle &= ~WS_EX_TOPMOST;
        
        // Disable DWM frame extension
        MARGINS margins = {0, 0, 0, 0};
        HRESULT hr = DwmExtendFrameIntoClientArea(hwnd, &margins);
        if (FAILED(hr)) {
            Logger::LogMessage("Windows: Failed to reset DWM frame extension");
        }
        
        Logger::LogMessage("Windows: Applied windowed mode");
    } else {
        // Borderless mode: Hide title bar, keep resize capability
        style &= ~(WS_CAPTION | WS_SYSMENU);
        style |= (WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
        
        // Extend frame into client area for custom title bar
        MARGINS margins = {0, 0, 32, 0}; // 32px title bar height
        HRESULT hr = DwmExtendFrameIntoClientArea(hwnd, &margins);
        if (FAILED(hr)) {
            Logger::LogMessage("Windows: Failed to extend DWM frame");
        }
        
        Logger::LogMessage("Windows: Applied borderless mode");
    }

    if (SetWindowLong(hwnd, GWL_STYLE, style) == 0) {
        Logger::LogMessage("Windows: Failed to set window style");
        return;
    }
    
    if (SetWindowLong(hwnd, GWL_EXSTYLE, exStyle) == 0) {
        Logger::LogMessage("Windows: Failed to set extended window style");
        return;
    }
    
    // Apply changes and restore window position
    if (!SetWindowPos(hwnd, NULL, rect.left, rect.top, 
                     rect.right - rect.left, rect.bottom - rect.top,
                     SWP_NOZORDER | SWP_FRAMECHANGED)) {
        Logger::LogMessage("Windows: Failed to apply window position changes");
    }
#endif
}

void WindowModeManager::ApplyMacOSMode(CefRefPtr<CefWindow> window, WindowMode mode) {
#ifdef __APPLE__
    NSWindow* nsWindow = (__bridge NSWindow*)window->GetWindowHandle();
    if (!nsWindow) {
        Logger::LogMessage("macOS: Failed to get NSWindow handle");
        return;
    }

    NSWindowStyleMask styleMask = [nsWindow styleMask];

    if (mode == WindowMode::WINDOWED) {
        // Windowed mode: Show standard title bar
        styleMask |= (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | 
                     NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable);
        styleMask &= ~NSWindowStyleMaskFullSizeContentView;
        
        [nsWindow setTitlebarAppearsTransparent:NO];
        [nsWindow setTitleVisibility:NSWindowTitleVisible];
        
        Logger::LogMessage("macOS: Applied windowed mode");
    } else {
        // Borderless mode: Hide title bar, enable full-size content
        styleMask |= (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | 
                     NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable |
                     NSWindowStyleMaskFullSizeContentView);
        
        [nsWindow setTitlebarAppearsTransparent:YES];
        [nsWindow setTitleVisibility:NSWindowTitleHidden];
        
        Logger::LogMessage("macOS: Applied borderless mode");
    }

    [nsWindow setStyleMask:styleMask];
#endif
}

void WindowModeManager::ApplyLinuxMode(CefRefPtr<CefWindow> window, WindowMode mode) {
#ifdef __linux__
    // Try to get the native window handle
    CefWindowHandle handle = window->GetWindowHandle();
    if (!handle) {
        Logger::LogMessage("Linux: Failed to get window handle");
        return;
    }

#ifdef CEF_USE_GTK
    // GTK-based approach
    GtkWidget* gtk_window = GTK_WIDGET(handle);
    if (GTK_IS_WINDOW(gtk_window)) {
        if (mode == WindowMode::WINDOWED) {
            gtk_window_set_decorated(GTK_WINDOW(gtk_window), TRUE);
            Logger::LogMessage("Linux: Applied windowed mode (GTK)");
        } else {
            gtk_window_set_decorated(GTK_WINDOW(gtk_window), FALSE);
            Logger::LogMessage("Linux: Applied borderless mode (GTK)");
        }
        return;
    }
#endif

    // X11-based approach as fallback
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        Logger::LogMessage("Linux: Failed to open X11 display");
        return;
    }

    Window x11_window = (Window)handle;
    Atom wm_hints = XInternAtom(display, "_MOTIF_WM_HINTS", False);
    
    if (wm_hints != None) {
        struct {
            unsigned long flags;
            unsigned long functions;
            unsigned long decorations;
            long input_mode;
            unsigned long status;
        } hints = {0};

        hints.flags = 2; // MWM_HINTS_DECORATIONS
        hints.decorations = (mode == WindowMode::WINDOWED) ? 1 : 0;

        XChangeProperty(display, x11_window, wm_hints, wm_hints, 32,
                       PropModeReplace, (unsigned char*)&hints, 5);
        
        std::string mode_str = (mode == WindowMode::WINDOWED) ? "windowed" : "borderless";
        Logger::LogMessage("Linux: Applied " + mode_str + " mode (X11)");
    }

    XCloseDisplay(display);
#endif
}
void WindowModeManager::SaveWindowState(CefRefPtr<CefWindow> window) {
    if (!window) return;
    
    // For now, just log the action
    // TODO: Implement persistent storage (config file, registry, etc.)
    Logger::LogMessage("WindowModeManager: Window state save requested (not implemented)");
}

void WindowModeManager::RestoreWindowState(CefRefPtr<CefWindow> window) {
    if (!window) return;
    
    // Set default window size and position
    CefRect bounds(100, 100, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    window->SetBounds(bounds);
    
    Logger::LogMessage("WindowModeManager: Applied default window state");
}

bool WindowModeManager::CanToggleMode() {
#ifdef __linux__
    // On Linux, mode changes might require window recreation
    // Check if we're in a supported environment
    return true; // For now, assume it's supported
#else
    return true;
#endif
}