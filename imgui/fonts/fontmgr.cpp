#include <windows.h>

#include "fontmgr.h"

#include <algorithm>
#include <cstring>

ImFont *FontMgr::Get(const char *fontID)
{
    for (auto &f : fonts)
    {
        if (f.id == fontID)
            return f.font;
    }
    return ImGui::GetIO().FontDefault;
}

const ImWchar *FontMgr::GetGlyphRanges()
{
    static const ImWchar ranges[] = {0x0020, 0x00FF, // Basic Latin + Latin Supplement
                                     0x0980, 0x09FF, // Bengali
                                     0x2000, 0x206F, // General Punctuation

                                     // Cyrillic
                                     0x0400, 0x052F, 0x2DE0, 0x2DFF, 0xA640, 0xA69F,

                                     // Turkish
                                     0x011E, 0x011F, 0x015E, 0x015F, 0x0130, 0x0131, 0};
    return ranges;
}

const ImWchar *FontMgr::GetIconGlyphRanges()
{
    static const ImWchar ranges[] = {0xF0, 0xFB, 0};
    return ranges;
}

inline float GetScaleFactor()
{
    RECT rect;
    HWND hwnd = GetActiveWindow();
    if (!GetClientRect(hwnd, &rect))
        return 1.0f;

    float width = rect.right - rect.left;
    float height = rect.bottom - rect.top;

    return min(width / 1366.0f, height / 768.0f);
}

ImFont *FontMgr::LoadFont(const char *fontID, const char *data, float fontMul, bool isIcon)
{
    ImGuiIO &io = ImGui::GetIO();
    float fontSize = GetScaleFactor() * fontMul;
    ImFont *font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(data, fontSize, nullptr,
                                                                  isIcon ? GetIconGlyphRanges() : GetGlyphRanges());

    fonts.emplace_back(font, fontID, data, fontMul, isIcon);
    io.Fonts->Build();
    return font;
}

void FontMgr::UnloadAll()
{
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();
}

void FontMgr::ReloadAll()
{
    UnloadAll();
    ImGuiIO &io = ImGui::GetIO();

    for (auto &f : fonts)
    {
        float scaleFactor = min(io.DisplaySize.x / 1366.0f, io.DisplaySize.y / 768.0f);
        float fontSize = scaleFactor * f.multiplier;

        f.font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(f.data, fontSize, nullptr,
                                                                f.iconFont ? GetIconGlyphRanges() : GetGlyphRanges());
    }

    // Reset default font if available
    ImFont *defaultFont = Get("text");
    if (defaultFont)
        io.FontDefault = defaultFont;

    io.Fonts->Build();
}
