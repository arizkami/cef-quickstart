#include "native_window_controls.hpp"
#include "logger.hpp"

#ifdef _WIN32
    #include <windows.h>
    #include <dwmapi.h>
    #pragma comment(lib, "dwmapi.lib")
#elif defined(__APPLE__)
    #include <Cocoa/Cocoa.h>
#elif defined(__linux__)
    #include <gtk/gtk.h>
    #include <gdk/gdk.h>
    // Only include platform-specific headers if available
    #if defined(GDK_WINDOWING_WAYLAND) && __has_include(<gdk/gdkwayland.h>)
        #include <gdk/gdkwayland.h>
        #define HAS_WAYLAND_SUPPORT 1
    #endif
    #if defined(GDK_WINDOWING_X11) && __has_include(<gdk/gdkx.h>)
        #include <gdk/gdkx.h>
        #include <X11/Xlib.h>
        #define HAS_X11_SUPPORT 1
    #endif
#endif

void NativeWindowControls::SetupNativeControls(CefRefPtr<CefWindow> window) {
    if (!window) {
        Logger::LogMessage("NativeWindowControls: Invalid window reference");
        return;
    }

#ifdef _WIN32
    SetupWindowsControls(window);
#elif defined(__APPLE__)
    SetupMacOSControls(window);
#elif defined(__linux__)
    SetupLinuxControls(window);
#endif
}

void NativeWindowControls::UpdateControlsVisibility(CefRefPtr<CefWindow> window, bool show) {
    if (!window) return;

#ifdef _WIN32
    HWND hwnd = window->GetWindowHandle();
    if (hwnd) {
        // Show/hide native Windows controls
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        if (show) {
            style |= (WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        } else {
            style &= ~(WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        }
        SetWindowLong(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
#elif defined(__APPLE__)
    // macOS native controls are always visible, but we can customize them
    NSWindow* nsWindow = (__bridge NSWindow*)window->GetWindowHandle();
    if (nsWindow) {
        if (show) {
            [nsWindow setStyleMask:[nsWindow styleMask] | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable];
        } else {
            [nsWindow setStyleMask:[nsWindow styleMask] & ~(NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable)];
        }
    }
#elif defined(__linux__)
    // Linux: Use GTK/GDK to control window decorations
    GtkWidget* gtkWindow = GTK_WIDGET(window->GetWindowHandle());
    if (gtkWindow && GTK_IS_WINDOW(gtkWindow)) {
        gtk_window_set_decorated(GTK_WINDOW(gtkWindow), show);
    }
#endif
}

void NativeWindowControls::SetControlsPosition(CefRefPtr<CefWindow> window, int x, int y, int width, int height) {
    if (!window) return;

#ifdef _WIN32
    HWND hwnd = window->GetWindowHandle();
    if (hwnd) {
        // Windows: Extend frame into client area for custom title bar
        MARGINS margins = {0, 0, height, 0}; // Extend top margin
        DwmExtendFrameIntoClientArea(hwnd, &margins);
        Logger::LogMessage("Windows: Extended frame into client area");
    }
#elif defined(__APPLE__)
    // macOS: Set title bar height and appearance
    NSWindow* nsWindow = (__bridge NSWindow*)window->GetWindowHandle();
    if (nsWindow) {
        [nsWindow setTitlebarAppearsTransparent:YES];
        [nsWindow setTitleVisibility:NSWindowTitleHidden];
        Logger::LogMessage("macOS: Configured transparent title bar");
    }
#elif defined(__linux__)
    // Linux: Configure window manager hints
    GtkWidget* gtkWindow = GTK_WIDGET(window->GetWindowHandle());
    if (gtkWindow && GTK_IS_WINDOW(gtkWindow)) {
        // Set window to be client-side decorated
        gtk_window_set_decorated(GTK_WINDOW(gtkWindow), FALSE);
        
        // Set up custom header bar if using GTK4
        #if GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* headerBar = gtk_header_bar_new();
        gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(headerBar), TRUE);
        gtk_window_set_titlebar(GTK_WINDOW(gtkWindow), headerBar);
        #endif
        
        Logger::LogMessage("Linux: Configured client-side decorations");
    }
#endif
}

void NativeWindowControls::SetupWindowsControls(CefRefPtr<CefWindow> window) {
#ifdef _WIN32
    HWND hwnd = window->GetWindowHandle();
    if (!hwnd) {
        Logger::LogMessage("Windows: Failed to get window handle");
        return;
    }

    // Enable native Windows controls
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style |= (WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
    SetWindowLong(hwnd, GWL_STYLE, style);

    // Enable DWM composition for modern appearance
    BOOL enable = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &enable, sizeof(enable));
    
    // Set up custom title bar area
    MARGINS margins = {0, 0, 32, 0}; // 32px title bar height
    HRESULT hr = DwmExtendFrameIntoClientArea(hwnd, &margins);
    
    if (SUCCEEDED(hr)) {
        Logger::LogMessage("Windows: Successfully set up native controls with extended frame");
    } else {
        Logger::LogMessage("Windows: Failed to extend frame into client area");
    }

    // Apply changes
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
}

void NativeWindowControls::SetupMacOSControls(CefRefPtr<CefWindow> window) {
#ifdef __APPLE__
    NSWindow* nsWindow = (__bridge NSWindow*)window->GetWindowHandle();
    if (!nsWindow) {
        Logger::LogMessage("macOS: Failed to get NSWindow handle");
        return;
    }

    // Configure window style for native controls
    NSWindowStyleMask styleMask = [nsWindow styleMask];
    styleMask |= (NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable);
    [nsWindow setStyleMask:styleMask];

    // Make title bar transparent and hide title
    [nsWindow setTitlebarAppearsTransparent:YES];
    [nsWindow setTitleVisibility:NSWindowTitleHidden];
    
    // Enable full-size content view
    [nsWindow setStyleMask:[nsWindow styleMask] | NSWindowStyleMaskFullSizeContentView];
    
    Logger::LogMessage("macOS: Successfully set up native controls with transparent title bar");
#endif
}

void NativeWindowControls::SetupLinuxControls(CefRefPtr<CefWindow> window) {
#ifdef __linux__
    // Initialize GTK if not already done
    if (!gtk_is_initialized()) {
        gtk_init();
    }

    GtkWidget* gtkWindow = GTK_WIDGET(window->GetWindowHandle());
    if (!gtkWindow || !GTK_IS_WINDOW(gtkWindow)) {
        Logger::LogMessage("Linux: Failed to get GTK window handle");
        return;
    }

    // Disable default window decorations
    gtk_window_set_decorated(GTK_WINDOW(gtkWindow), FALSE);

    #if GTK_CHECK_VERSION(4, 0, 0)
    // GTK4: Create custom header bar with native controls
    GtkWidget* headerBar = gtk_header_bar_new();
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(headerBar), TRUE);
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(headerBar), NULL);
    
    // Set the header bar as the title bar
    gtk_window_set_titlebar(GTK_WINDOW(gtkWindow), headerBar);
    
    Logger::LogMessage("Linux: Successfully set up GTK4 native controls with header bar");
    #else
    // GTK3: Set window manager hints for client-side decorations
    GdkWindow* gdkWindow = gtk_widget_get_window(gtkWindow);
    if (gdkWindow) {
        gdk_window_set_decorations(gdkWindow, GDK_DECOR_BORDER | GDK_DECOR_RESIZEH);
    }
    
    Logger::LogMessage("Linux: Successfully set up GTK3 native controls");
    #endif

    // Handle platform-specific setup
    #if defined(HAS_WAYLAND_SUPPORT) || defined(HAS_X11_SUPPORT)
    GdkDisplay* display = gdk_display_get_default();
    
    #ifdef HAS_WAYLAND_SUPPORT
    if (GDK_IS_WAYLAND_DISPLAY(display)) {
        Logger::LogMessage("Linux: Running on Wayland, using compositor decorations");
        // Wayland compositors handle decorations
    }
    #endif

    #ifdef HAS_X11_SUPPORT
    if (GDK_IS_X11_DISPLAY(display)) {
        Logger::LogMessage("Linux: Running on X11, using window manager decorations");
        // X11 window manager handles decorations
    }
    #endif
    #else
    Logger::LogMessage("Linux: Using basic GTK window decorations");
    #endif
#endif
}