#pragma once

#include <vector>
#include <string>
#include "configstorage.h"

class Locale {
private:
    static inline std::vector<std::string> m_locales;
    static inline std::string m_path;
    static inline ConfigStorage* m_pData = nullptr;
    static inline ConfigStorage* m_pCallbackData = nullptr;
    static inline size_t localeIndex;

public:
    enum class eReturnCodes {
        DIR_NOT_FOUND = 0,
        NO_LOCALE_FOUND = 1,
        INVALID_INDEX = 2,
        SUCCESS = 3,
        DEF_LOCALE_NOT_FOUND = 3,
    };
    
    Locale() = delete;
    Locale(Locale const&) = delete;
    void operator=(Locale const&) = delete;

    static eReturnCodes Init(const char* path, const char* def = "English", const char* fallback = "");
    static std::vector<std::string>& GetLocaleList();
    static size_t GetCurrentLocaleIndex();
    static std::string GetText(std::string&& key, std::string&& defaultValue = "");
    static eReturnCodes SetLocale(size_t index);
    static eReturnCodes SetLocaleByName(const std::string& name);
    static void SetDefaultLocale();
};