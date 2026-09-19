#pragma once

class DownloadManager;

class DownloadBackend {
public:
    virtual ~DownloadBackend() = default;
    DownloadBackend(const DownloadBackend&) = delete;
    DownloadBackend& operator=(const DownloadBackend&) = delete;

    virtual void attach(DownloadManager& manager) = 0;

protected:
    DownloadBackend() = default;
};
