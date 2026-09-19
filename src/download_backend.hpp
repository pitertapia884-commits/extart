#pragma once

#include <memory>

class DownloadManager;
struct _WebKitNetworkSession;

class DownloadBackend {
public:
    virtual ~DownloadBackend() = default;
    DownloadBackend(const DownloadBackend&) = delete;
    DownloadBackend& operator=(const DownloadBackend&) = delete;
    virtual void attach(DownloadManager& manager) = 0;
protected:
    DownloadBackend() = default;
};

std::unique_ptr<DownloadBackend> make_webkit_download_backend(_WebKitNetworkSession* session);
