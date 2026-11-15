#pragma once

#include <cstdint>
#include <filesystem>
#include <system_error>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <vector>
#endif

inline std::filesystem::path executable_path(std::error_code& error)
{
#ifdef __APPLE__
    std::uint32_t length = 0;
    (void)_NSGetExecutablePath(nullptr, &length);
    std::vector<char> path(length);
    if (_NSGetExecutablePath(path.data(), &length) != 0) {
        error = std::make_error_code(std::errc::no_such_file_or_directory);
        return {};
    }
    return std::filesystem::canonical(path.data(), error);
#else
    return std::filesystem::read_symlink("/proc/self/exe", error);
#endif
}

inline std::filesystem::path executable_path()
{
    std::error_code error;
    const auto path = executable_path(error);
    if (error)
        throw std::filesystem::filesystem_error("Could not locate executable", path, error);
    return path;
}
