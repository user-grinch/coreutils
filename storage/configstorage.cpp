#include "storage/configstorage.h"
#include <filesystem>

ConfigStorage::ConfigStorage(const std::string& filePathWithExt) noexcept {
    path = filePathWithExt;
    if (std::filesystem::exists(path)) {
        toml::parse_result result = toml::parse_file(path);

        if (result) {
            pTable = std::make_unique<toml::table>(std::move(result));
            return;
        }
    }

    if (pTable == nullptr) {
        pTable = std::make_unique<toml::table>();

        // if (fileName == FILE_NAME) {
        //     gLogger->info("Creating {}{}", fileName, fileExt);
        // } else {
        //     gLogger->warn("Error parsing {}{}", fileName, fileExt);
        // }
    }
}

bool ConfigStorage::Contains(const char* ele) noexcept {
    if (pTable) {
        return pTable->contains(ele);
    }

    return false;
}

ConfigStorage::Table& ConfigStorage::Items() noexcept {
    return pTable->as_table()->ref<ConfigStorage::Table>();
}

void ConfigStorage::RemoveTable(const char* key) noexcept {
    if (pTable) {
        pTable->erase(key);
    }
}

void ConfigStorage::Remove(const char* path) noexcept {
    if (pTable) {
        std::string str = path;
        auto pos = str.find_last_of('.');
        if (pos != std::string::npos) {
            std::string parent = str.substr(0, pos);
            std::string key = str.substr(pos + 1);
            toml::table* tbl = pTable->at_path(parent).as_table();
            if (tbl) {
                tbl->erase(key);
            }
        } else {
            pTable->erase(path);
        }
    }
}

void ConfigStorage::RemoveKey(const char* key, const char* entry) noexcept {
    if (pTable) {
        Table *tbl = pTable->at_path(key).as_table();
        if (tbl) {
            tbl->erase(entry);
        }
    }
}

void ConfigStorage::Save() noexcept {
    if (pTable) {
        std::ofstream file(path);
        if (file.good()) {
            file << *pTable << std::endl;
            file.close();
        }
    }
}