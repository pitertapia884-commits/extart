#pragma once

#include <filesystem>
#include <string>

class Profile {
public:
    Profile();
    ~Profile() = default;

    Profile(const Profile&) = delete;
    Profile& operator=(const Profile&) = delete;

    const std::filesystem::path& data_directory() const { return data_directory_; }
    const std::filesystem::path& cache_directory() const { return cache_directory_; }
    const std::filesystem::path& cookies_path() const { return cookies_path_; }

private:
    std::filesystem::path data_directory_;
    std::filesystem::path cache_directory_;
    std::filesystem::path cookies_path_;
};
