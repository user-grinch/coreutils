#include <windows.h>

#include "fontmgr.h"

#include <algorithm>
#include <cstring>
#include <imgui_internal.h>

constexpr float BASE_WIDTH = 1366.0f;
constexpr float BASE_HEIGHT = 768.0f;

float FontMgr::GetScaleFactor(float w, float h)
{
    RECT rect;
    HWND hwnd = GetActiveWindow();
    if (!GetClientRect(hwnd, &rect))
    {
        return 1.0f;
    }

    if (w == -1) w = static_cast<float>(rect.right - rect.left);
    if (h == -1) h = static_cast<float>(rect.bottom - rect.top);
    return min(w / BASE_WIDTH * 0.5f, h / BASE_HEIGHT * 0.5f);
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
    auto it = std::find_if(vecLoadedFonts.begin(), vecLoadedFonts.end(), [&](const FontInfo &f) { return f.sID == fontID; });
    return it != vecLoadedFonts.end() ? it->pFont : ImGui::GetIO().FontDefault;
}

ImFont *FontMgr::LoadFont(const char *fontID, const char *data, float fontMul, bool isIcon)
{
    ImGuiIO &io = ImGui::GetIO();
    float fontSize = GetScaleFactor() * fontMul;

    ImFont *font =
        io.Fonts->AddFontFromMemoryCompressedBase85TTF(data, fontSize, nullptr, GetGlyphRangesInternal(isIcon));

    vecLoadedFonts.emplace_back(font, fontID, data, fontMul, isIcon);
    vecLoadedFonts.back().fBaseSize = fontSize;
    io.Fonts->Build();
    return font;
}

void FontMgr::UnloadAll()
{
    ImGui::GetIO().Fonts->Clear();
}

void FontMgr::RescaleFonts(float w, float h)
{
    float factor = GetScaleFactor(w, h);
    for (const auto &f : vecLoadedFonts)
    {
        ImGui::SetCurrentFont(f.pFont, f.fBaseSize, f.fMul * factor);
    }
}
