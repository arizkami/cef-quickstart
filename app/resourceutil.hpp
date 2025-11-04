#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "include/cef_stream.h"

namespace ResourceUtil {
    // Resource ID definitions (single HTML file with inlined CSS/JS)
    enum ResourceId {
        IDR_HTML_INDEX = 100
    };
    
    // Preloaded resource structure
    struct PreloadedResource {
        std::vector<uint8_t> data;
        std::string mimeType;
        bool loaded;
        
        PreloadedResource() : loaded(false) {}
    };
    
    // Initialize preloaded resources at startup
    void InitializePreloadedResources();
    
    // Get resource ID from path
    int GetResourceId(const std::string& path);
    
    // Load binary resource by ID (with preload support)
    std::vector<uint8_t> LoadBinaryResource(int resource_id);
    
    // Get preloaded resource by path
    const PreloadedResource* GetPreloadedResource(const std::string& path);
    
    // Get MIME type from file extension
    std::string GetMimeType(const std::string& path);
    
    // Create CEF stream reader from resource data
    CefRefPtr<CefStreamReader> CreateResourceReader(const std::vector<uint8_t>& data);
    
    // Check if resources are initialized
    bool AreResourcesInitialized();
    
    // Get preload statistics
    struct PreloadStats {
        int totalResources;
        int loadedResources;
        size_t totalBytes;
        bool allLoaded;
    };
    PreloadStats GetPreloadStats();
}