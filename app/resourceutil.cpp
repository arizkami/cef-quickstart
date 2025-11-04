#include "resourceutil.hpp"
#include "webapp.hpp"
#include <map>
#include <algorithm>
#include <mutex>
#include "include/cef_stream.h"

namespace ResourceUtil {
    
    // Global preloaded resources storage
    static std::map<std::string, PreloadedResource> g_preloadedResources;
    static std::mutex g_resourceMutex;
    static bool g_resourcesInitialized = false;
    
    // Resource path to ID mapping (single HTML file with inlined assets)
    static const std::map<std::string, int> resourceMap = {
        {"/index.html", IDR_HTML_INDEX}
    };
    
    void InitializePreloadedResources() {
        std::lock_guard<std::mutex> lock(g_resourceMutex);
        
        if (g_resourcesInitialized) {
            return; // Already initialized
        }
        
        // Initialize HTML resource (contains inlined CSS and JS)
        PreloadedResource htmlResource;
        const char* html_content = GetWebAppHTML();
        unsigned int html_size = GetWebAppHTMLSize();
        
        if (html_content && html_size > 0) {
            htmlResource.data = std::vector<uint8_t>(html_content, html_content + html_size);
            htmlResource.mimeType = GetMimeType("/index.html");
            htmlResource.loaded = true;
            g_preloadedResources["/index.html"] = std::move(htmlResource);
            
            // Log successful preload
            std::string sizeStr = std::to_string(html_size);
            // Note: We can't use Logger here as it might not be initialized yet
            // Logger will be used in main.cpp after this function returns
        } else {
            // Log failure - resource not available
            // Note: We can't use Logger here as it might not be initialized yet
        }
        
        g_resourcesInitialized = true;
    }
    
    int GetResourceId(const std::string& path) {
        auto it = resourceMap.find(path);
        if (it != resourceMap.end()) {
            return it->second;
        }
        return -1; // Resource not found
    }
    
    std::vector<uint8_t> LoadBinaryResource(int resource_id) {
        // Ensure resources are initialized
        if (!g_resourcesInitialized) {
            InitializePreloadedResources();
        }
        
        // Load HTML content from webapp.hpp
        if (resource_id == IDR_HTML_INDEX) {
            const char* html_content = GetWebAppHTML();
            unsigned int html_size = GetWebAppHTMLSize();
            
            std::vector<uint8_t> data(html_content, html_content + html_size);
            return data;
        }
        
        // Return empty vector for unknown resources
        return std::vector<uint8_t>();
    }
    
    const PreloadedResource* GetPreloadedResource(const std::string& path) {
        std::lock_guard<std::mutex> lock(g_resourceMutex);
        
        // Ensure resources are initialized
        if (!g_resourcesInitialized) {
            InitializePreloadedResources();
        }
        
        auto it = g_preloadedResources.find(path);
        if (it != g_preloadedResources.end() && it->second.loaded) {
            return &it->second;
        }
        
        return nullptr;
    }
    
    bool AreResourcesInitialized() {
        std::lock_guard<std::mutex> lock(g_resourceMutex);
        return g_resourcesInitialized;
    }
    
    PreloadStats GetPreloadStats() {
        std::lock_guard<std::mutex> lock(g_resourceMutex);
        
        PreloadStats stats = {};
        stats.totalResources = static_cast<int>(g_preloadedResources.size());
        stats.loadedResources = 0;
        stats.totalBytes = 0;
        
        for (const auto& pair : g_preloadedResources) {
            if (pair.second.loaded) {
                stats.loadedResources++;
                stats.totalBytes += pair.second.data.size();
            }
        }
        
        stats.allLoaded = (stats.loadedResources == stats.totalResources) && (stats.totalResources > 0);
        return stats;
    }
    
    std::string GetMimeType(const std::string& path) {
        std::string lower_path = path;
        std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::tolower);
        
        // Helper function to check if string ends with suffix
        auto ends_with = [](const std::string& str, const std::string& suffix) {
            return str.size() >= suffix.size() && 
                   str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        };
        
        if (ends_with(lower_path, ".html") || ends_with(lower_path, ".htm")) {
            return "text/html";
        } else if (ends_with(lower_path, ".css")) {
            return "text/css";
        } else if (ends_with(lower_path, ".js")) {
            return "application/javascript";
        } else if (ends_with(lower_path, ".json")) {
            return "application/json";
        } else if (ends_with(lower_path, ".png")) {
            return "image/png";
        } else if (ends_with(lower_path, ".jpg") || ends_with(lower_path, ".jpeg")) {
            return "image/jpeg";
        } else if (ends_with(lower_path, ".svg")) {
            return "image/svg+xml";
        }
        
        return "application/octet-stream";
    }
    
    CefRefPtr<CefStreamReader> CreateResourceReader(const std::vector<uint8_t>& data) {
        if (data.empty()) {
            return nullptr;
        }
        
        return CefStreamReader::CreateForData(
            const_cast<void*>(static_cast<const void*>(data.data())), 
            data.size()
        );
    }
}