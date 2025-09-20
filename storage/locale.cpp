#include "locale.h"
#include <filesystem>

Locale::eReturnCodes Locale::Init(const char* path, const char* def, const char* fallback) {
    m_path = path;
    if (m_path.back() != '/') {
        m_path += '/';
    }

    if (!std::filesystem::exists(m_path)) {
        return eReturnCodes::DIR_NOT_FOUND;
    }

    for (auto& entry : std::filesystem::directory_iterator(m_path)) {
        if (entry.path().extension() == ".toml") {
            std::string fileName = entry.path().stem().string();
            m_locales.push_back(fileName);

            if (!strcmp(fallback, fileName.c_str())) {
                std::string localePath = m_path + fileName;

                if (m_pCallbackData) {
                    delete m_pCallbackData;
                    m_pCallbackData = nullptr;
                }
                m_pCallbackData = new ConfigStorage(localePath);
            }
        }
    }

    if (m_locales.empty()) {
        return eReturnCodes::NO_LOCALE_FOUND;
    }

    std::vector<std::string>& vec = Locale::GetLocaleList();

    size_t index = 0;
    for (std::string& locale : vec) {
        if (locale == def) {
            Locale::SetLocale(index);
            break;
        }

        index++;
    }

    if (!m_pData) {
        return eReturnCodes::DEF_LOCALE_NOT_FOUND;
    }

    return eReturnCodes::SUCCESS;
}

std::vector<std::string>& Locale::GetLocaleList() {
    return m_locales;
}

size_t Locale::GetCurrentLocaleIndex() {
    return localeIndex;
}

void Locale::SetDefaultLocale() {
    SetLocaleByName("English");
}

Locale::eReturnCodes Locale::SetLocale(size_t index) {
    if (m_pData) {
        delete m_pData;
        m_pData = nullptr;
    }

    if (index < 0 || index >= m_locales.size()) {
        return eReturnCodes::INVALID_INDEX;
    }
    m_pData = new ConfigStorage(m_path + m_locales[index] + ".toml");
    localeIndex = index;
    return eReturnCodes::SUCCESS;
}

Locale::eReturnCodes Locale::SetLocaleByName(const std::string& name) {
    for (size_t i = 0; i < m_locales.size(); ++i) {
        if (m_locales[i] == name) {
            return SetLocale(i);
        }
    }

    return eReturnCodes::NO_LOCALE_FOUND;
}

std::string Locale::GetText(std::string&& key, std::string&& defaultValue) {
    if (m_pData == nullptr) {
        return defaultValue;
    }

    if (defaultValue.empty()) {
        defaultValue = "#" + key;
    }

    std::string rtn = m_pData->Get(key.c_str(), defaultValue);

    if (rtn == defaultValue) {
        return m_pCallbackData->Get(key.c_str(), defaultValue);
    }

    return rtn;
}