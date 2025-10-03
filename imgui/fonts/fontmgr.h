#pragma once
#include "imgui.h"

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

        FontInfo(ImFont *f, std::string i, const char *d, float mul, bool icon = false)
            : font(f), multiplier(mul), id(std::move(i)), data(d), iconFont(icon)
        {
        }
    };

    static inline std::vector<FontInfo> fonts;

    static const ImWchar *GetGlyphRangesInternal(bool isIcon);

    static float GetScaleFactor();

  public:
    FontMgr() = delete;
    FontMgr(const FontMgr &) = delete;

    static ImFont *Get(const char *fontID);
    static ImFont *LoadFont(const char *fontID, const char *data, float fontMul = 1.0f, bool isIcon = false);
    static void ReloadAll();
    static void UnloadAll();
};
