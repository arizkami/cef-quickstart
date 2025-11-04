#include "loading_manager.hpp"
#include "logger.hpp"
#include <sstream>

LoadingManager::LoadingManager() 
    : current_state_(INITIALIZING)
    , progress_(0.0f)
    , content_loaded_(false)
    , window_shown_(false) {
}

LoadingManager::~LoadingManager() {
}

LoadingManager& LoadingManager::GetInstance() {
    static LoadingManager instance;
    return instance;
}

void LoadingManager::SetState(LoadingState state, const std::string& message) {
    current_state_.store(static_cast<int>(state));
    state_message_ = message;
    
    // Log state changes
    std::string state_name;
    switch (state) {
        case INITIALIZING:
            state_name = "INITIALIZING";
            SetProgress(0.1f);
            break;
        case PRELOADING_RESOURCES:
            state_name = "PRELOADING_RESOURCES";
            SetProgress(0.3f);
            break;
        case CREATING_WINDOW:
            state_name = "CREATING_WINDOW";
            SetProgress(0.5f);
            break;
        case LOADING_CONTENT:
            state_name = "LOADING_CONTENT";
            SetProgress(0.8f);
            break;
        case READY:
            state_name = "READY";
            SetProgress(1.0f);
            break;
        case ERROR_STATE:
            state_name = "ERROR";
            break;
    }
    
    std::string log_message = "Loading State: " + state_name;
    if (!message.empty()) {
        log_message += " - " + message;
    }
    Logger::LogMessage(log_message);
    
    // Auto-show window when ready
    if (state == READY && content_loaded_.load() && !window_shown_.load()) {
        ShowWindowWhenReady();
    }
}

LoadingManager::LoadingState LoadingManager::GetState() const {
    return static_cast<LoadingState>(current_state_.load());
}

std::string LoadingManager::GetStateMessage() const {
    return state_message_;
}

void LoadingManager::SetWindow(CefRefPtr<CefWindow> window) {
    window_ = window;
    Logger::LogMessage("LoadingManager: Window reference set");
}

void LoadingManager::ShowWindowWhenReady() {
    if (window_ && !window_shown_.load() && GetState() == READY) {
        Logger::LogMessage("LoadingManager: Showing window - all loading complete");
        
        // Center and show the window
        window_->CenterWindow(CefSize(1200, 800));
        window_->Show();
        window_shown_.store(true);
        
        Logger::LogMessage("LoadingManager: Window is now visible");
    }
}

void LoadingManager::OnContentLoaded() {
    content_loaded_.store(true);
    Logger::LogMessage("LoadingManager: Content loaded");
    
    // If we're already in READY state, show the window
    if (GetState() == READY && !window_shown_.load()) {
        ShowWindowWhenReady();
    }
}

void LoadingManager::SetProgress(float progress) {
    progress_ = std::max(0.0f, std::min(1.0f, progress));
    
    // Log progress milestones
    static float last_logged_progress = -1.0f;
    float current_progress = progress_.load();
    
    if (current_progress - last_logged_progress >= 0.2f || current_progress >= 1.0f) {
        std::ostringstream oss;
        oss << "Loading Progress: " << static_cast<int>(current_progress * 100) << "%";
        Logger::LogMessage(oss.str());
        last_logged_progress = current_progress;
    }
}

float LoadingManager::GetProgress() const {
    return progress_.load();
}

void LoadingManager::SetError(const std::string& error) {
    error_message_ = error;
    SetState(ERROR_STATE, error);
    Logger::LogMessage("LoadingManager ERROR: " + error);
}

bool LoadingManager::HasError() const {
    return GetState() == ERROR_STATE;
}

std::string LoadingManager::GetError() const {
    return error_message_;
}