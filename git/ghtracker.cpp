#include "ghtracker.h"
#include <Urlmon.h>
#include <fstream>
#include <sstream>

GHTracker::GHTracker(const std::string& userName, const std::string& repoName, const std::string& currentVer)
    : localVer(currentVer)
{
    link = "https://api.github.com/repos/" + userName + "/" + repoName + "/tags";
}

void GHTracker::CheckUpdate(bool showMSG) {
    curState = eStates::Checking;
    showMessage = showMSG;
}

std::string GHTracker::GetUpdateVersion() const {
    return latestVer;
}

bool GHTracker::IsUpdateAvailable() const {
    return curState == eStates::Found;
}

void GHTracker::ResetUpdaterState() {
    curState = eStates::Idle;
}

static bool IsVersionNewer(const std::string& latest, const std::string& current) {
    return latest > current; // Replace with semantic version comparison if needed
}

// --- Minimal JSON "name" extractor ---
static std::string ExtractFirstTagName(const std::string& jsonText) {
    const std::string key = "\"name\"";
    size_t pos = jsonText.find(key);
    if (pos == std::string::npos) return {};

    // Find the first quote after "name":
    pos = jsonText.find('"', pos + key.size());
    if (pos == std::string::npos) return {};
    size_t start = pos + 1;

    // Find the closing quote
    size_t end = jsonText.find('"', start);
    if (end == std::string::npos) return {};

    return jsonText.substr(start, end - start);
}

void GHTracker::Process(std::function<void(eStates, bool)> eventCallback, const std::string& path, bool lockDowngrade) {
    if (curState != eStates::Checking) return;

    curState = eStates::Failed;

    if (URLDownloadToFile(nullptr, link.c_str(), path.c_str(), 0, nullptr) != S_OK) {
        eventCallback(curState, showMessage);
        return;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        eventCallback(curState, showMessage);
        return;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string jsonText = buffer.str();
    latestVer = ExtractFirstTagName(jsonText);

    if (latestVer.empty()) {
        curState = eStates::NotFound;
    } else {
        bool isNewer = IsVersionNewer(latestVer, localVer);
        curState = (isNewer || !lockDowngrade) ? eStates::Found : eStates::NotFound;
    }

    eventCallback(curState, showMessage);
}