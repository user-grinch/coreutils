#pragma once
#include <string>
#include <functional>

enum class eStates {
    Idle,
    Checking,
    Found,
    NotFound,
    Failed,
};

class GHTracker {
private:
    eStates curState = eStates::Idle;
    std::string latestVer, link, localVer;
    bool showMessage = false;

public:
    GHTracker(const std::string& userName, const std::string & repoName, const std::string& currentVer) {
        link = "https://api.github.com/repos/" + userName + "/" + repoName + "/tags";
        localVer = currentVer;
    }

    void CheckUpdate(bool showMSG = false) {
        curState = eStates::Checking;
        showMessage = showMSG;
    }

    _NODISCARD std::string GetUpdateVersion() {
        return latestVer;
    }

    _NODISCARD bool IsUpdateAvailable() {
        return curState == eStates::Found;
    }

    void ResetUpdaterState() {
        curState = eStates::Idle;
    }

    void Process(std::function<void(eStates, bool)> eventCallback, std::string path, bool lockDowngrade = true);
};