#pragma once
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

template <typename T>
concept Serializable = requires(nlohmann::json j, T a) {
    { j.get<T>() } -> std::same_as<T>;
    { nlohmann::json(a) } -> std::same_as<nlohmann::json>;
};

template <Serializable T> 
class ConfigLoader
{
  public:
    ConfigLoader(const std::string &path) : _filePath(path)
    {
        load();
    }

    bool save(const T& obj) const
    {
        std::ofstream out(_filePath);
        if (!out) {
            MessageBox(NULL, std::format("Error occured trying to save {}", _filePath).c_str(), "Save Error", NULL);
            return false;
        }
        out << nlohmann::json(obj).dump(4);
        return true;
    }

    std::optional<T> load()
    {
        std::ifstream in(_filePath);
        if (!in) {
            return std::nullopt;
        }
        nlohmann::json j;
        in >> j;
        return j.get<T>();
    }

  private:
    std::string _filePath;
};

template <Serializable T> class TableLoader
{
  public:
    TableLoader(const std::string &path) : _filePath(path)
    {
        load();
    }

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
        if (auto it = _tables.find(key); it != _tables.end())
            it->second.clear();
    }

    void removeTable(const std::string &key)
    {
        if (_tables.erase(key))
            _tableNames.erase(std::remove(_tableNames.begin(), _tableNames.end(), key), _tableNames.end());
    }

    void removeByID(const std::string &id)
    {
        for (auto &[key, entries] : _tables)
        {
            entries.erase(
                std::remove_if(entries.begin(), entries.end(), [&id](const T &entry) { return entry.toString() == id; }),
                entries.end());
        }
    }

    void updateByID(const std::string &id, const T &updatedEntry)
    {
        for (auto &[key, entries] : _tables)
        {
            for (auto &entry : entries)
            {
                if (entry.toString() == id)
                {
                    entry = updatedEntry;
                    return;
                }
            }
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

    bool save() const
    {
        nlohmann::json j;
        for (const auto &[key, entries] : _tables)
        {
            j[key] = entries;
        }

        std::ofstream out(_filePath);
        if (!out)
        {
            MessageBox(NULL, std::format("Error occured trying to save {}", _filePath).c_str(), "Save Error", NULL);
            return false;
        }
        out << j.dump(4);
        return true;
    }

    bool load()
    {
        std::ifstream in(_filePath);
        if (!in) {
            return false;
        }

        nlohmann::json j;
        in >> j;

        _tables.clear();
        _tableNames.clear();

        for (auto it = j.begin(); it != j.end(); ++it)
        {
            _tables[it.key()] = it.value().get<std::vector<T>>();
            _tableNames.push_back(it.key());
        }
        return true;
    }

  private:
    std::string _filePath;
    std::unordered_map<std::string, std::vector<T>> _tables;
    std::vector<std::string> _tableNames;
};
