#include "binaryresourceprovider.hpp"
#include "resourceutil.hpp"
#include "include/cef_response.h"
#include "include/cef_stream.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "logger.hpp"
#include <string>

BinaryResourceProvider::BinaryResourceProvider() {
}

CefRefPtr<CefResourceHandler> BinaryResourceProvider::Create(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& scheme_name,
    CefRefPtr<CefRequest> request) {
    
    CEF_REQUIRE_IO_THREAD();
    
    std::string url = request->GetURL();
    Logger::LogMessage("BinaryResourceProvider: Handling URL: " + url);
    
    // Only handle miko://app requests
    if (url.find("miko://app") != 0) {
        Logger::LogMessage("BinaryResourceProvider: URL does not start with miko://app");
        return nullptr;
    }
    
    // Extract the path from the URL (remove "miko://app")
    std::string path = url.substr(10); // Remove "miko://app"
    if (path.empty() || path == "/") {
        path = "/index.html";
    }
    Logger::LogMessage("BinaryResourceProvider: Extracted path: " + path);
    
    // For single-file HTML build, all requests should go to index.html
    // since CSS and JS are inlined
    std::string actualPath = path;
    if (path != "/index.html") {
        // Redirect all other requests to index.html for single-file build
        actualPath = "/index.html";
        Logger::LogMessage("BinaryResourceProvider: Redirecting " + path + " to " + actualPath + " (single-file build)");
    }
    
    // Try to get preloaded resource first
    const ResourceUtil::PreloadedResource* preloaded = ResourceUtil::GetPreloadedResource(actualPath);
    std::vector<uint8_t> resource_data;
    
    if (preloaded && preloaded->loaded) {
        Logger::LogMessage("BinaryResourceProvider: Using preloaded resource for path: " + actualPath);
        resource_data = preloaded->data;
    } else {
        // Fallback to traditional resource loading
        int resource_id = ResourceUtil::GetResourceId(actualPath);
        Logger::LogMessage("BinaryResourceProvider: Resource ID: " + std::to_string(resource_id));
        if (resource_id == -1) {
            Logger::LogMessage("BinaryResourceProvider: Resource not found for path: " + actualPath);
            return nullptr; // Resource not found
        }
        
        resource_data = ResourceUtil::LoadBinaryResource(resource_id);
        if (resource_data.empty()) {
            Logger::LogMessage("BinaryResourceProvider: Failed to load resource data for path: " + actualPath);
            return nullptr;
        }
    }
    
    // Create stream reader
    CefRefPtr<CefStreamReader> stream = ResourceUtil::CreateResourceReader(resource_data);
    if (!stream) {
        return nullptr;
    }
    
    // Get MIME type (always HTML for single-file build)
    std::string mime_type = ResourceUtil::GetMimeType(actualPath);
    
    // Create and return the resource handler
    return new CefStreamResourceHandler(mime_type, stream);
}