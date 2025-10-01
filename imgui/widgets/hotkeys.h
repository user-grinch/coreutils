#pragma once
#include <imgui.h>
#include <imgui_internal.h>

#include <string>

extern class ConfigStorage;
class Hotkey {
private:
  static inline std::string current;
  bool wPressed = false;
  ConfigStorage *config;
  ImGuiKey codes[2], defaultCodes[2];
  std::string path;
  double lastUpdate;

public:
  Hotkey(const std::string& name, ConfigStorage *config = nullptr, ImGuiKey key1 = ImGuiKey_None, ImGuiKey key2 = ImGuiKey_None);

  void Draw(const char* label);

  std::string GetKeyCombo();
  bool Pressed(bool noDelay = false);
};