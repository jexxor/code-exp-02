#pragma once

#include <array>
#include <filesystem>
#include <string>

enum class ColorScheme { System, Monokai };

struct Appearance {
    ColorScheme color_scheme = ColorScheme::Monokai;
    std::string font_family = "Victor Mono";
    int font_size = 16;
};

Appearance read_appearance(const std::filesystem::path& path);
Appearance load_appearance();
std::array<unsigned, 12> syntax_colors(ColorScheme scheme);
