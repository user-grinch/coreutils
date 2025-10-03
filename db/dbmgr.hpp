#pragma once

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

enum class ConfigMode
{
    UnifiedObject,
    TableMap
};

template <typename T>
concept Serializable = requires(nlohmann::json j, T a) {
    { j.get<T>() } -> std::same_as<T>;
    { nlohmann::json(a) } -> std::same_as<nlohmann::json>;
};

template <Serializable T> class Database
{
  public:
    Database(const std::string &path, ConfigMode mode = ConfigMode::TableMap) : _filePath(path), _mode(mode)
    {
        load();
    }

    ConfigMode mode() const
    {
        return _mode;
    }

    // Unified object mode
    T &object()
    {
        return _object;
    }
    void setObject(const T &obj)
    {
        _object = obj;
    }

    // Table operations
    void addToTable(const std::string &key, const T &entry)
    {
        if (_tables.find(key) == _tables.end())
            _tableNames.push_back(key);
        _tables[key].push_back(entry);
    }

    std::vector<T> getTable(const std::string &key) const
    {
        auto it = _tables.find(key);
                return (it != _tables.end()) ? it->second : std::vector<T>{};
    }

    void clearTable(const std::string &key)
    {
        auto it = _tables.find(key);
        if (it != _tables.end())
            it->second.clear();
    }

    void removeTable(const std::string &key)
    {
        if (_tables.erase(key))
        {
            _tableNames.erase(std::remove(_tableNames.begin(), _tableNames.end(), key), _tableNames.end());
        }
    }

    void renameTable(const std::string &oldName, const std::string &newName)
    {
        auto it = _tables.find(oldName);
        if (it != _tables.end())
        {
            _tables[newName] = std::move(it->second);
            _tables.erase(it);

            auto nIt = std::find(_tableNames.begin(), _tableNames.end(), oldName);
            if (nIt != _tableNames.end())
                *nIt = newName;
        }
    }

    const std::unordered_map<std::string, std::vector<T>> &tables() const
    {
        return _tables;
    }

    const std::vector<std::string> &tableNames() const
    {
        return _tableNames;
    }

    // Save/load
    void save() const
    {
        nlohmann::json j;
        if (_mode == ConfigMode::UnifiedObject)
        {
            j = _object;
        }
        else
        {
            for (const auto &[key, entries] : _tables)
                j[key] = entries;
        }

        std::ofstream out(_filePath);
        if (!out)
            throw std::runtime_error("Failed to write: " + _filePath);
        out << j.dump(4);
    }

    void load()
    {
        std::ifstream in(_filePath);
        if (!in)
            return;

        nlohmann::json j;
        in >> j;

        if (_mode == ConfigMode::UnifiedObject)
        {
            _object = j.get<T>();
        }
        else
        {
            _tables.clear();
            _tableNames.clear();

            for (auto it = j.begin(); it != j.end(); ++it)
            {
                _tables[it.key()] = it.value().get<std::vector<T>>();
                _tableNames.push_back(it.key());
            }
        }
    }

    void removeByID(const std::string &id)
    {
        for (auto &[key, entries] : _tables)
        {
            entries.erase(
                std::remove_if(entries.begin(), entries.end(), [&id](const T &entry) { return entry.toID() == id; }),
                entries.end());
        }
    }

    void removeEmptyTables()
    {
        for (auto it = _tables.begin(); it != _tables.end();)
        {
            if (it->second.empty())
            {
                _tableNames.erase(std::remove(_tableNames.begin(), _tableNames.end(), it->first), _tableNames.end());
                it = _tables.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

  private:
    std::string _filePath;
    ConfigMode _mode;
    T _object;
    std::unordered_map<std::string, std::vector<T>> _tables;
    std::vector<std::string> _tableNames; // internal only, not serialized
};
