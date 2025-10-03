#include <windows.h>

#include "fontmgr.h"

#include <algorithm>
#include <cstring>

constexpr float BASE_WIDTH = 1366.0f;
constexpr float BASE_HEIGHT = 768.0f;

float FontMgr::GetScaleFactor()
{
    RECT rect;
    HWND hwnd = GetActiveWindow();
    if (!GetClientRect(hwnd, &rect))
        return 1.0f;

    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);
    return min(width / BASE_WIDTH, height / BASE_HEIGHT);
}

const ImWchar *FontMgr::GetGlyphRangesInternal(bool isIcon)
{
    static const ImWchar textRanges[] = {0x0020, 0x00FF, // Basic Latin + Latin Supplement
                                         0x0980, 0x09FF, // Bengali
                                         0x2000, 0x206F, // General Punctuation
                                         0x0400, 0x052F, 0x2DE0, 0x2DFF, 0xA640, 0xA69F, // Cyrillic
                                         0x011E, 0x011F, 0x015E, 0x015F, 0x0130, 0x0131, // Turkish
                                         0};

    static const ImWchar iconRanges[] = {0xF0, 0xFB, 0};

    return isIcon ? iconRanges : textRanges;
}

ImFont *FontMgr::Get(const char *fontID)
{
    auto it = std::find_if(fonts.begin(), fonts.end(), [&](const FontInfo &f) { return f.id == fontID; });
    return it != fonts.end() ? it->font : ImGui::GetIO().FontDefault;
}

ImFont *FontMgr::LoadFont(const char *fontID, const char *data, float fontMul, bool isIcon)
{
    ImGuiIO &io = ImGui::GetIO();
    float fontSize = GetScaleFactor() * fontMul;

    ImFont *font =
        io.Fonts->AddFontFromMemoryCompressedBase85TTF(data, fontSize, nullptr, GetGlyphRangesInternal(isIcon));

    fonts.emplace_back(font, fontID, data, fontMul, isIcon);
    io.Fonts->Build();
    return font;
}

void FontMgr::UnloadAll()
{
    ImGui::GetIO().Fonts->Clear();
}

void FontMgr::ReloadAll()
{
    UnloadAll();
    ImGuiIO &io = ImGui::GetIO();

    std::vector<FontInfo> reloaded;
    for (const auto &f : fonts)
    {
        ImFont *font = LoadFont(f.id.c_str(), f.data, f.multiplier, f.iconFont);
        reloaded.emplace_back(font, f.id, f.data, f.multiplier, f.iconFont);
    }

    fonts = std::move(reloaded);

    if (ImFont *defaultFont = Get("text"))
    {
        io.FontDefault = defaultFont;
    }

    io.Fonts->Build();
}