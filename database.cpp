#include "database.h"
#include <fstream>
#include <sys/stat.h>
#include <direct.h>
#include <sstream>
#include <unordered_map>
#include <dirent.h>
#include <string.h>

using namespace std;

Database::Database(const std::string& db_path) : db_path_(db_path) {
    struct stat info;
    if (stat(db_path.c_str(), &info) != 0) {
        _mkdir(db_path.c_str());
    }
    load_metadata();
}

Database::~Database() {
    save_metadata();
}

bool Database::create_table(const std::string& table_name, 
                          const std::vector<std::pair<std::string, std::string>>& columns) {
    std::string table_path = get_table_path(table_name);
    struct stat info;
    if (stat(table_path.c_str(), &info) == 0) {
        return false;
    }

    _mkdir(table_path.c_str());
    
    std::ofstream columns_file(table_path + "/columns");
    for (const auto& col : columns) {
        columns_file << col.first << "|" << col.second << "\n";
    }
    
    
    std::ofstream log_file(db_path_ + "/operations.log", std::ios::app);
    log_file << "CREATE TABLE " << table_name << "\n";
    
    table_metadata_[table_name] = table_path;
    return true;
}



bool Database::drop_table(const std::string& table_name) {
    std::string table_path = get_table_path(table_name);
    struct stat info;
    if (stat(table_path.c_str(), &info) != 0) {
        return false;
    }
    
    std::ofstream log_file(db_path_ + "/operations.log", std::ios::app);
    log_file << "DROP TABLE " << table_name << "\n";
    
    system(("rmdir /s /q " + table_path).c_str());
    table_metadata_.erase(table_name);
    return true;
}



std::string Database::get_table_path(const std::string& table_name) {
    return db_path_ + "/" + table_name;
}

bool Database::load_metadata() {
    std::string meta_path = db_path_ + "/metadata";
    struct stat info;
    if (stat(meta_path.c_str(), &info) != 0) {
        return true;
    }
    
    std::ifstream meta_file(meta_path);
    std::string line;
    while (std::getline(meta_file, line)) {
        size_t pos = line.find('|');
        if (pos != std::string::npos) {
            table_metadata_[line.substr(0, pos)] = line.substr(pos + 1);
        }
    }
    return true;
}



bool Database::save_metadata() {
    std::string meta_path = db_path_ + "/metadata";
    std::ofstream meta_file(meta_path);
    
    for (const auto& entry : table_metadata_) {
        meta_file << entry.first << "|" << entry.second << "\n";
    }
    return true;
}

std::vector<std::unordered_map<std::string, std::string>> 
    Database::select(const std::string& table_name, 
                   const std::vector<std::string>& columns, 
                   const std::string& where_condition) {
    std::vector<std::unordered_map<std::string, std::string>> results;
    std::string table_path = get_table_path(table_name);
    struct stat info;
    if (stat(table_path.c_str(), &info) != 0) {
        return results;
    }
    
    std::ofstream log_file(db_path_ + "/operations.log", std::ios::app);
    log_file << "SELECT ";
    bool first = true;
    for (const auto& col : columns) {
        if (!first) log_file << ", ";
        log_file << col;
        first = false;
    }
    log_file << " FROM " << table_name << " WHERE " << where_condition << "\n";
    
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(table_path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, "columns") != 0) {
                std::string row_file_path = table_path + "/" + ent->d_name;
                std::ifstream row_file(row_file_path);
                std::unordered_map<std::string, std::string> row;
                std::string line;
                while (std::getline(row_file, line)) {
                    size_t pos = line.find('|');
                    if (pos != std::string::npos) {
                        row[line.substr(0, pos)] = line.substr(pos + 1);
                    }
                }
                results.push_back(row);
            }
        }
        closedir(dir);
    }
    
    return results;
}

bool Database::insert(const std::string& table_name, 
               const std::unordered_map<std::string, std::string>& values) {
    try {
        std::string table_path = get_table_path(table_name);
        struct stat info;
        if (stat(table_path.c_str(), &info) != 0) {
            return false;
        }
        
        if (values.find("id") == values.end()) {
            return false;
        }
        
        std::ofstream log_file(db_path_ + "/operations.log", std::ios::app);
        if (!log_file.is_open()) {
            return false;
        }
        log_file << "INSERT INTO " << table_name << " VALUES (";
        bool first = true;
        for (const auto& val : values) {
            if (!first) log_file << ", ";
            log_file << val.second;
            first = false;
        }
        log_file << ")\n";
        
        std::ofstream row_file(table_path + "/" + values.at("id"));
        if (!row_file.is_open()) {
            return false;
        }
        for (const auto& val : values) {
            row_file << val.first << "|" << val.second << "\n";
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool Database::update(const std::string& table_name,
                     const std::unordered_map<std::string, std::string>& values,
                     const std::string& where_condition) {
    try {
        std::string table_path = get_table_path(table_name);
        struct stat info;
        if (stat(table_path.c_str(), &info) != 0) {
            return false;
        }
        
        size_t eq_pos = where_condition.find('=');
        if (eq_pos == std::string::npos) {
            return false;
        }
        std::string where_col = where_condition.substr(0, eq_pos);
        std::string where_val = where_condition.substr(eq_pos + 1);
        
        std::ofstream log_file(db_path_ + "/operations.log", std::ios::app);
        if (!log_file.is_open()) {
            return false;
        }
        log_file << "UPDATE " << table_name << " SET ";
        bool first = true;
        for (const auto& val : values) {
            if (!first) log_file << ", ";
            log_file << val.first << "=" << val.second;
            first = false;
        }
        log_file << " WHERE " << where_condition << "\n";
        
        DIR* dir;
        struct dirent* ent;
        if ((dir = opendir(table_path.c_str())) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 && 
                    strcmp(ent->d_name, "columns") != 0) {
                    
                    std::string row_file_path = table_path + "/" + ent->d_name;
                    std::ifstream in_file(row_file_path);
                    std::unordered_map<std::string, std::string> row;
                    std::string line;
                    
                    while (std::getline(in_file, line)) {
                        size_t pos = line.find('|');
                        if (pos != std::string::npos) {
                            row[line.substr(0, pos)] = line.substr(pos + 1);
                        }
                    }
                    
                    if (row.find(where_col) != row.end() && row[where_col] == where_val) {
                        
                        for (const auto& val : values) {
                            row[val.first] = val.second;
                        }
                        
                        std::ofstream out_file(row_file_path);
                        if (!out_file.is_open()) {
                            closedir(dir);
                            return false;
                        }
                        for (const auto& val : row) {
                            out_file << val.first << "|" << val.second << "\n";
                        }
                    }
                }
            }
            closedir(dir);
        }
        
        return true;
    } catch (...) {
        return false;
    }
}


