#include <Windows.h>
#include <string>
#include <stdexcept>

#define REGISTRY_PATH L"SOFTWARE\\Grinch_\\GTA\\Trainer\\"
#define REGISTRY_KEY L"LatestVersion"

class RegistryWrapper {
private:
    HKEY hKey;
    std::wstring subkey;

public:
    RegistryWrapper(const std::wstring& subkey) : subkey(subkey), hKey(NULL) {
        LONG openResult = RegCreateKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, NULL, 0, KEY_WRITE | KEY_READ, NULL, &hKey, NULL);
        if (openResult != ERROR_SUCCESS) {
            MessageBoxW(NULL, L"Failed to open key", L"Error", MB_ICONERROR | MB_OK);
        }
    }

    ~RegistryWrapper() {
        if (hKey != NULL) {
            RegCloseKey(hKey);
        }
    }

    void SetValue(const std::wstring& valueName, DWORD value) {
        LONG setResult = RegSetValueExW(hKey, valueName.c_str(), 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        if (setResult != ERROR_SUCCESS) {
            MessageBoxW(NULL, L"Failed set update", L"Error", MB_ICONERROR | MB_OK);
        }
    }

    DWORD GetValue(const std::wstring& valueName) {
        DWORD value;
        DWORD dataSize = sizeof(value);
        LONG queryResult = RegQueryValueExW(hKey, valueName.c_str(), NULL, NULL, (LPBYTE)&value, &dataSize);
        if (queryResult != ERROR_SUCCESS) {
            return NULL;
        }
        return value;
    }
};
