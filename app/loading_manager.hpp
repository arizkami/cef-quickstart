#pragma once

#include <atomic>
#include <string>
#include "include/views/cef_window.h"

// Loading state manager for Electron-like behavior
class LoadingManager {
public:
    enum LoadingState {
        INITIALIZING = 0,
        PRELOADING_RESOURCES = 1,
        CREATING_WINDOW = 2,
        LOADING_CONTENT = 3,
        READY = 4,
        ERROR_STATE = 5
    };

    // Singleton access
    static LoadingManager& GetInstance();
    
    // State management
    void SetState(LoadingState state, const std::string& message = "");
    LoadingState GetState() const;
    std::string GetStateMessage() const;
    
    // Window management
    void SetWindow(CefRefPtr<CefWindow> window);
    void ShowWindowWhenReady();
    void OnContentLoaded();
    
    // Progress tracking
    void SetProgress(float progress); // 0.0 to 1.0
    float GetProgress() const;
    
    // Error handling
    void SetError(const std::string& error);
    bool HasError() const;
    std::string GetError() const;

private:
    LoadingManager();
    ~LoadingManager();
    LoadingManager(const LoadingManager&);
    LoadingManager& operator=(const LoadingManager&);
    
    std::atomic<int> current_state_;
    std::atomic<float> progress_;
    std::string state_message_;
    std::string error_message_;
    CefRefPtr<CefWindow> window_;
    std::atomic<bool> content_loaded_;
    std::atomic<bool> window_shown_;
};