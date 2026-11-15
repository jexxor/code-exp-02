#include "appearance.h"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <fstream>
#include <stdexcept>

namespace {
std::filesystem::path appearance_path()
{
    if (const auto* path = std::getenv("SOURCE_CANVAS_CONFIG"); path && *path)
        return path;
    if (const auto* root = std::getenv("XDG_CONFIG_HOME"); root && *root)
        return std::filesystem::path(root) / "source-canvas/config.json";
    if (const auto* home = std::getenv("HOME"); home && *home) {
#ifdef __APPLE__
        return std::filesystem::path(home) /
               "Library/Application Support/source-canvas/config.json";
#else
        return std::filesystem::path(home) / ".config/source-canvas/config.json";
#endif
    }
    return {};
}

[[noreturn]] void invalid(const std::filesystem::path& path, const std::string& reason)
{
    throw std::runtime_error("Invalid appearance config " + path.string() + ": " + reason);
}
} // namespace

Appearance read_appearance(const std::filesystem::path& path)
{
    Appearance result;
    if (path.empty())
        return result;
    std::error_code error;
    if (!std::filesystem::exists(path, error)) {
        if (error)
            invalid(path, error.message());
        return result;
    }
    std::ifstream input(path);
    if (!input)
        invalid(path, "cannot read file");
    const auto config = nlohmann::json::parse(input, nullptr, false);
    if (!config.is_object())
        invalid(path, "expected a JSON object");
    if (config.contains("theme")) {
        if (!config["theme"].is_string())
            invalid(path, "theme must be a string");
        const auto theme = config["theme"].get<std::string>();
        if (theme == "system")
            result.color_scheme = ColorScheme::System;
        else if (theme != "monokai")
            invalid(path, "theme must be monokai or system");
    }
    if (config.contains("font_family")) {
        if (!config["font_family"].is_string())
            invalid(path, "font_family must be a string");
        result.font_family = config["font_family"].get<std::string>();
        if (result.font_family.empty())
            invalid(path, "font_family cannot be empty");
    }
    if (config.contains("font_size")) {
        if (!config["font_size"].is_number_integer())
            invalid(path, "font_size must be an integer");
        if (config["font_size"] < 8 || config["font_size"] > 48)
            invalid(path, "font_size must be between 8 and 48 pixels");
        result.font_size = config["font_size"].get<int>();
    }
    if (config.contains("ui_scale")) {
        if (!config["ui_scale"].is_number() || config["ui_scale"] < 0.5 || config["ui_scale"] > 4.0)
            invalid(path, "ui_scale must be between 0.5 and 4");
        result.ui_scale = config["ui_scale"].get<double>();
    }
    return result;
}

Appearance load_appearance()
{
    return read_appearance(appearance_path());
}

std::array<unsigned, 12> syntax_colors(ColorScheme scheme)
{
    if (scheme == ColorScheme::Monokai)
        return {0xf8f8f2, 0xf92672, 0x66d9ef, 0xe6db74, 0x75715e, 0xae81ff,
                0xfd971f, 0xa6e22e, 0xf8f8f2, 0xfd971f, 0xa6e22e, 0x66d9ef};
    return {0x24292f, 0x0033b3, 0x6f42c1, 0xa31515, 0x39703a, 0x006b6b,
            0x8546a3, 0x795e26, 0x24292f, 0x875a00, 0x005cc5, 0x6f42c1};
}
