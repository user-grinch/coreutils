#pragma once
#include "imgui.h"

#include <algorithm>
#include <string>
#include <vector>

class FontMgr
{
  private:
    struct FontInfo
    {
        ImFont *font;
        float multiplier;
        std::string id;
        const char *data;
        bool iconFont;

        FontInfo(ImFont *f, const std::string &i, const char *d, float mul, bool icon = false)
            : font(f), multiplier(mul), id(i), data(d), iconFont(icon)
        {
        }
    };

    static inline std::vector<FontInfo> fonts;

  public:
    FontMgr() = delete;
    FontMgr(const FontMgr &) = delete;

    // Returns font pointer from name
    static ImFont *Get(const char *fontID);

    // Get the glyph ranges for standard and icon fonts
    static const ImWchar *GetGlyphRanges();
    static const ImWchar *GetIconGlyphRanges();

    // Load a font from compressed memory
    static ImFont *LoadFont(const char *fontID, const char *data, float fontMul = 1.0f, bool isIcon = false);

    // Reload all fonts
    static void ReloadAll();

    // Unload all fonts
    static void UnloadAll();
};
