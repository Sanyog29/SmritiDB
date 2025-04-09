#pragma once

#include <string>
#include <vector>
#include <unordered_map>

/**
 * Simple file-based database implementation
 */
class Database {
public:
    /**
     * @param db_path 
     */
    Database(const std::string& db_path);
    
   
    ~Database();

    /**
     * @param table_name
     * @param columns 
     * @return
     */
    bool create_table(const std::string& table_name, 
                     const std::vector<std::pair<std::string, std::string>>& columns);
    
    /**
     * @param table_name 
     * @return 
     */
    bool drop_table(const std::string& table_name);

    /**
     * @param table_name 
     * @param values 
     * @return
     */
    bool insert(const std::string& table_name, 
               const std::unordered_map<std::string, std::string>& values);
    
    /**
     * @param table_name
     * @param values
     * @param where_condition
     * @return 
     */
    bool update(const std::string& table_name, 
               const std::unordered_map<std::string, std::string>& values, 
               const std::string& where_condition);
    
    /**
     * @param table_name 
     * @param where_condition 
     * @return 
     */
    bool remove(const std::string& table_name, 
               const std::string& where_condition);
    


    /**
     * @param table_name 
     * @param columns
     * @param where_condition 
     * @return
     */
    std::vector<std::unordered_map<std::string, std::string>> 
        select(const std::string& table_name, 
              const std::vector<std::string>& columns, 
              const std::string& where_condition);

private:
    std::string db_path_;
    std::unordered_map<std::string, std::string> table_metadata_;

    std::string get_table_path(const std::string& table_name);
    bool load_metadata();
    bool save_metadata();
};