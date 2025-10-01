#include "hotkeys.h"
#include "storage/configstorage.h"

inline bool IsKeyDown(ImGuiKey key)
{
    return key == ImGuiKey_None || ImGui::IsKeyDown(key);
}

Hotkey::Hotkey(const std::string &name, ConfigStorage *config, ImGuiKey key1, ImGuiKey key2)
{
    codes[0] = defaultCodes[0] = key1;

    if (key2 == ImGuiKey_None)
    {
        codes[1] = defaultCodes[1] = key1;
    }
    else
    {
        codes[1] = defaultCodes[1] = key2;
    }

    path = "Hotkeys." + name;
    this->config = config;

    if (config)
    {
        codes[0] = static_cast<ImGuiKey>(config->Get((path + ".Key1").c_str(), static_cast<int>(codes[0])));
        codes[1] = static_cast<ImGuiKey>(config->Get((path + ".Key2").c_str(), static_cast<int>(codes[1])));
    }
}

void Hotkey::Draw(const char *label)
{
    bool active = (current == label);
    bool pressed = false;
    static bool wasPressed = false;
    ImGuiStyle &style = ImGui::GetStyle();

    // Check for pressed keys
    if (active)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_FrameBgActive]);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_FrameBgActive]);

        int c = 0;
        for (ImGuiKey k = ImGuiKey_Tab; k <= ImGuiKey_GamepadRStickDown;
             (k = static_cast<ImGuiKey>(static_cast<int>(k) + 1)))
        {
            if (IsKeyDown(k))
            {
                codes[c++] = k;
                pressed = true;
                if (c >= 2)
                    break;
            }
        }

        if (codes[1] == ImGuiKey_None)
        {
            codes[1] = codes[0];
        }
    }

    ImGui::Text(label);
    ImVec2 resetButtonSize = ImGui::CalcTextSize("ResetReset");
    float button1Width = ImGui::GetContentRegionAvail().x - resetButtonSize.x - ImGui::GetStyle().ItemSpacing.x * 3.0f;
    float height = ImGui::GetFrameHeight() * 1.25f;
    if (ImGui::Button(std::format("{}##{}", GetKeyCombo(), label).c_str(), ImVec2(button1Width, height)) && !active)
    {
        current = label;
    }

    // Show a tooltip for active state above the hotkey widget
    if (active)
    {
        if (wasPressed && !IsKeyDown(codes[0]) && !IsKeyDown(codes[1]))
        {
            current = "";
            wasPressed = pressed = false;

            if (config)
            {
                config->Set((path + ".Key1").c_str(), static_cast<int>(codes[0]));
                config->Set((path + ".Key2").c_str(), static_cast<int>(codes[1]));
            }

            lastUpdate = ImGui::GetTime();
        }

        ImVec2 windowPos = ImGui::GetCurrentWindow()->Pos;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        pos.y = pos.y > windowPos.y ? pos.y : windowPos.y;
        pos.y -= ImGui::GetTextLineHeightWithSpacing() + height + style.WindowPadding.y * 2;

        ImGui::SetNextWindowPos(pos);
        if (!pressed && current == label)
        {
            codes[0] = ImGuiKey_None;
            codes[1] = ImGuiKey_None;
        }
        ImGui::SetTooltip(pressed ? "Release the keys to set as hotkey" : "Press a new key combination");
    }

    if (active)
    {
        ImGui::PopStyleColor(2);
        wasPressed = pressed;
    }
    ImGui::SameLine();
    if (ImGui::Button(std::format("Reset##{}", label).c_str(), ImVec2(resetButtonSize.x, height)))
    {
        current = "";
        codes[0] = defaultCodes[0];
        codes[1] = defaultCodes[1];
        config->Set((path + ".Key1").c_str(), static_cast<int>(codes[0]));
        config->Set((path + ".Key2").c_str(), static_cast<int>(codes[1]));
    }
    ImGui::Dummy(ImVec2(0, 10));
}

bool Hotkey::Pressed(bool noDelay)
{
    if (ImGui::GetTime() - lastUpdate < 2.0)
        return false;

    if (noDelay)
    {
        return IsKeyDown(codes[0]) && IsKeyDown(codes[1]);
    }
    else
    {
        if (IsKeyDown(codes[0]) && IsKeyDown(codes[1]))
        {
            wPressed = true;
        }
        else
        {
            if (wPressed)
            {
                wPressed = false;
                return current == "";
            }
        }
    }
    return false;
}

std::string Hotkey::GetKeyCombo()
{
    if (codes[0] == codes[1])
    {
        return ImGui::GetKeyName(codes[0]);
    }
    else
    {
        return std::format("{} + {}", ImGui::GetKeyName(codes[0]), ImGui::GetKeyName(codes[1]));
    }
}
