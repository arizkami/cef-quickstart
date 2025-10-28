#include "app.hpp"

// SimpleRenderProcessHandler implementation
SimpleRenderProcessHandler::SimpleRenderProcessHandler() {
    // Create the renderer-side router for query handling
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterRendererSide::Create(config);
}

void SimpleRenderProcessHandler::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                                 CefRefPtr<CefFrame> frame,
                                                 CefRefPtr<CefV8Context> context) {
    // Register JavaScript functions with the new context
    message_router_->OnContextCreated(browser, frame, context);
}

void SimpleRenderProcessHandler::OnContextReleased(CefRefPtr<CefBrowser> browser,
                                                   CefRefPtr<CefFrame> frame,
                                                   CefRefPtr<CefV8Context> context) {
    // Clean up context
    message_router_->OnContextReleased(browser, frame, context);
}

bool SimpleRenderProcessHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                         CefRefPtr<CefFrame> frame,
                                                         CefProcessId source_process,
                                                         CefRefPtr<CefProcessMessage> message) {
    // Handle process messages
    return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
}

// SimpleApp implementation
SimpleApp::SimpleApp() {
    render_process_handler_ = new SimpleRenderProcessHandler();
}

void SimpleApp::OnBeforeCommandLineProcessing(const CefString& process_type,
                                            CefRefPtr<CefCommandLine> command_line) {
    // Disable extensions to prevent Chrome UI interference with keyboard shortcuts
    command_line->AppendSwitch("disable-extensions");
    
    // Hide standard Chrome UI elements for Views-based applications
    command_line->AppendSwitch("hide-controls");
    command_line->AppendSwitch("hide-overlays");
    
    // Enable Chrome runtime for better UI control
    command_line->AppendSwitch("enable-chrome-runtime");
    
    // Disable default browser check to prevent unwanted dialogs
    command_line->AppendSwitch("no-default-browser-check");
    
    // Disable first run experience
    command_line->AppendSwitch("no-first-run");
    
    // Disable developer tools and debugging features
    command_line->AppendSwitch("disable-dev-tools");
    command_line->AppendSwitch("disable-extensions-http-throttling");
    command_line->AppendSwitch("disable-plugins-discovery");
    
    // Disable context menu and right-click options
    command_line->AppendSwitch("disable-default-apps");
    
    // Memory and performance optimizations for smaller footprint
    command_line->AppendSwitch("disable-web-security");
    
    // Disable various Chrome features that could expose UI
    command_line->AppendSwitch("disable-background-mode");
    command_line->AppendSwitch("disable-background-timer-throttling");
    command_line->AppendSwitch("disable-renderer-backgrounding");
    command_line->AppendSwitch("disable-backgrounding-occluded-windows");
    
    // Disable print preview and save page functionality
    command_line->AppendSwitch("disable-print-preview");
    
    // Disable native window frame for custom title bar implementation
    command_line->AppendSwitch("disable-features=WindowControlsOverlay");
    
    // Enable custom title bar area for native window controls
    command_line->AppendSwitch("enable-experimental-web-platform-features");
    
    // Allow web content to extend into title bar area
    command_line->AppendSwitchWithValue("enable-blink-features", 
        "CSSEnvironmentVariables");
    
    command_line->AppendSwitchWithValue("enable-features",
    "WindowControlsOverlay,WebAppWindowControlsOverlay");
    command_line->AppendSwitch("enable-experimental-web-platform-features");
    command_line->AppendSwitchWithValue("enable-blink-features",
    "WindowControlsOverlay,CSSEnvironmentVariables");

}

void SimpleApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) {
    // Register the miko:// custom scheme
    registrar->AddCustomScheme("miko", 
        CEF_SCHEME_OPTION_STANDARD | 
        CEF_SCHEME_OPTION_LOCAL | 
        CEF_SCHEME_OPTION_CORS_ENABLED |
        CEF_SCHEME_OPTION_SECURE);
}