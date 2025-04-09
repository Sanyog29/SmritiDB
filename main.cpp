#include "database.h"
#include <iostream>
#include <sstream>
#include <algorithm>

std::vector<std::string> split_string(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void process_command(Database& db, const std::string& command) {
    std::vector<std::string> parts = split_string(command, ' ');
    if (parts.empty()) return;

    std::string operation = parts[0];
    std::transform(operation.begin(), operation.end(), operation.begin(), ::tolower);

    try {
        if (operation == "create" && parts.size() > 3 && parts[1] == "table") {
            std::string table_name = parts[2];
            std::vector<std::pair<std::string, std::string>> columns;
            for (size_t i = 3; i < parts.size(); i += 2) {
                if (i + 1 >= parts.size()) {
                    throw std::runtime_error("Invalid column definition. Expected format: column_name column_type");
                }
                columns.push_back({parts[i], parts[i+1]});
            }
            if (db.create_table(table_name, columns)) {
                std::cout << "Table '" << table_name << "' created successfully!\n";
            } else {
                std::cout << "Failed to create table '" << table_name << "'\n";
            }
        }
        else if (operation == "drop" && parts.size() == 3 && parts[1] == "table") {
            std::string table_name = parts[2];
            if (db.drop_table(table_name)) {
                std::cout << "Table '" << table_name << "' dropped successfully!\n";
            } else {
                std::cout << "Failed to drop table '" << table_name << "'\n";
            }
        }
        else if (operation == "insert" && parts.size() > 3 && parts[1] == "into") {
            std::string table_name = parts[2];
            std::unordered_map<std::string, std::string> values;
            for (size_t i = 3; i < parts.size(); i += 2) {
                if (i + 1 >= parts.size()) {
                    throw std::runtime_error("Invalid value pair. Expected format: column_name value");
                }
                values[parts[i]] = parts[i+1];
            }
            if (values.find("id") == values.end()) {
                throw std::runtime_error("Missing required 'id' field in INSERT command");
            }
            if (db.insert(table_name, values)) {
                std::cout << "Data inserted into '" << table_name << "' successfully!\n";
            } else {
                std::cout << "Failed to insert data into '" << table_name << "'\n";
            }
        }
        else if (operation == "select" && parts.size() > 3) {
            std::string table_name;
            std::vector<std::string> columns;
            std::string where_condition;
            bool from_found = false;
            bool where_found = false;
            
            for (size_t i = 1; i < parts.size(); i++) {
                if (parts[i] == "from") {
                    from_found = true;
                    table_name = parts[i+1];
                    i++;
                } else if (from_found && parts[i] == "where") {
                    where_found = true;
                    where_condition = parts[i+1];
                    i++;
                } else if (!from_found) {
                    columns.push_back(parts[i]);
                }
            }
            
            if (!from_found || columns.empty()) {
                throw std::runtime_error("Invalid SELECT syntax. Expected format: SELECT column1 column2 ... FROM table [WHERE condition]");
            }
            
            auto results = db.select(table_name, columns, where_condition);
            if (results.empty()) {
                std::cout << "No records found in '" << table_name << "'\n";
            } else {
                for (const auto& row : results) {
                    for (const auto& col : columns) {
                        if (row.find(col) != row.end()) {
                            std::cout << col << ": " << row.at(col) << " ";
                        }
                    }
                    std::cout << "\n";
                }
            }
        }
        else if (operation == "update" && parts.size() > 5 && parts[1] == "set") {
            std::string table_name;
            std::unordered_map<std::string, std::string> updates;
            std::string condition;
            
            size_t i = 2;
            while (i < parts.size() && parts[i] != "where") {
                if (i + 2 >= parts.size() || parts[i+1] != "=") {
                    throw std::runtime_error("Invalid UPDATE syntax in SET clause. Expected format: column=value");
                }
                updates[parts[i]] = parts[i+2];
                i += 3;
            }
            
            if (i >= parts.size() || parts[i] != "where" || i+1 >= parts.size()) {
                throw std::runtime_error("Invalid UPDATE syntax. Missing WHERE clause");
            }
            table_name = parts[i-1];
            condition = parts[i+1];
            
            if (db.update(table_name, updates, condition)) {
                std::cout << "Record updated in '" << table_name << "' successfully!\n";
            } else {
                std::cout << "Failed to update record in '" << table_name << "'\n";
            }
        }
        else if (operation == "select" && parts.size() > 5) {
            std::string table_name;
            std::vector<std::string> columns;
            std::string order_by;
            bool from_found = false;
            bool order_found = false;
            
            for (size_t i = 1; i < parts.size(); i++) {
                if (parts[i] == "from") {
                    from_found = true;
                    table_name = parts[i+1];
                    i++;
                } else if (from_found && parts[i] == "order" && i+2 < parts.size() && parts[i+1] == "by") {
                    order_found = true;
                    order_by = parts[i+2];
                    i += 2;
                } else if (!from_found) {
                    columns.push_back(parts[i]);
                }
            }
            
            if (!from_found || columns.empty()) {
                throw std::runtime_error("Invalid SELECT syntax. Expected format: SELECT column1 column2 ... FROM table [ORDER BY column]");
            }
            
            auto results = db.select(table_name, columns, order_found ? order_by : "");
            if (results.empty()) {
                std::cout << "No records found in '" << table_name << "'\n";
            } else {
                for (const auto& row : results) {
                    for (const auto& col : columns) {
                        std::cout << col << ": " << row.at(col) << " ";
                    }
                    std::cout << "\n";
                }
            }
        }
        else {
            throw std::runtime_error("Invalid command. Supported commands:\n"
                                    "  CREATE TABLE <name> <col1> <type1> <col2> <type2>...\n"
                                    "  INSERT INTO <table> <col1> <val1> <col2> <val2>...\n"
                                    "  SELECT <col1> <col2>... FROM <table> [ORDER BY column]\n"
                                    "  DELETE FROM <table> WHERE <column>=<value>\n"
                                    "  UPDATE <table> SET <column1>=<value1> <column2>=<value2>... WHERE <column>=<value>\n"
                                    "Note: Column definitions must be separated by spaces (e.g. 'id int name string')");
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}


void run_tests(Database& db) {
    std::cout << "\nRunning CREATE TABLE test with valid input...\n";
    process_command(db, "create table users id string name string age int");
    
    std::cout << "\nRunning CREATE TABLE test with invalid column definitions...\n";
    process_command(db, "create table invalid_table id string name");
    
    std::cout << "\nRunning INSERT test with valid data...\n";
    process_command(db, "insert into users id 1 name John age 25");
    process_command(db, "insert into users id 2 name Alice age 30");
    
    std::cout << "\nRunning INSERT test with missing ID field...\n";
    process_command(db, "insert into users name Bob age 35");
    
    std::cout << "\nRunning SELECT test for all columns...\n";
    process_command(db, "select id name age from users");
    
    std::cout << "\nRunning SELECT test for specific columns...\n";
    process_command(db, "select name age from users");
    
    std::cout << "\nRunning SELECT test with invalid syntax...\n";
    process_command(db, "select from users");
    
    std::cout << "\nRunning invalid command test...\n";
    process_command(db, "invalid command");
}

int main() {
    Database db("my_database");
    std::cout << "SmritiDB CLI (Type 'exit' to quit or 'test' to run tests)\n";
    
    while (true) {
        std::cout << "smritidb> ";
        std::string command;
        std::getline(std::cin, command);
        
        if (command == "exit") break;
        if (command == "test") {
            run_tests(db);
            continue;
        }
        process_command(db, command);
    }
    
    return 0;
}