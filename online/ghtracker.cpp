#include "ghtracker.h"
#include <fstream>
#include <sstream>
#include <Urlmon.h>
#include "storage/json.hpp"
#include "storage/registry.h"

extern RegistryWrapper gRegistry;

void GHTracker::Process(std::function<void(eStates, bool)> eventCallback, std::string path, bool lockDowngrade) {
    if (curState != eStates::Checking) {
        return;
    }
    curState = eStates::Failed;

    HRESULT res = URLDownloadToFile(nullptr, link.c_str(), path.c_str(), 0, nullptr);
    if (res == S_OK) {
        std::ifstream file(path);
        if (!file.is_open()) {
            curState = eStates::Failed;
            return;
        }
        nlohmann::json data = nlohmann::json::parse(file);
        file.close();

        if (!data.empty()) {
            latestVer = data[0]["name"];
            if (lockDowngrade) {
                std::string regVer = std::to_string(gRegistry.GetValue(REGISTRY_KEY));
                if (regVer == "" | latestVer > localVer || regVer > localVer) {
                    curState = eStates::Found;
                    gRegistry.SetValue(REGISTRY_KEY, std::stoi(GetUpdateVersion()));
                } else {
                    curState = eStates::NotFound;
                }
            } else {
                if (latestVer > localVer) {
                    curState = eStates::Found;
                } else {
                    curState = eStates::NotFound;
                }
            }
        }
    }
    eventCallback(curState, showMessage);
}