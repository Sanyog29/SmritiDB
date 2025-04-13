# Smriti Database Documentation

## Overview
Smriti is a simple file-based database system implemented in C++. It provides basic database operations like table creation, data insertion, updates, and queries.

## Database Class API

### Constructor
```cpp
Database(const std::string& db_path);
```
- Initializes the database with the specified path
- Creates necessary directory structure if it doesn't exist

### Table Operations
```cpp
bool create_table(const std::string& table_name, 
                 const std::vector<std::pair<std::string, std::string>>& columns);
```
- Creates a new table with specified columns
- Returns true on success, false on failure

```cpp
bool drop_table(const std::string& table_name);
```
- Removes a table and all its data
- Returns true on success, false on failure

### Data Operations
```cpp
bool insert(const std::string& table_name, 
           const std::unordered_map<std::string, std::string>& values);
```
- Inserts a new row into the specified table
- Requires an "id" field in the values map

```cpp
bool update(const std::string& table_name, 
           const std::unordered_map<std::string, std::string>& values, 
           const std::string& where_condition);
```
- Updates rows matching the where condition

```cpp
bool remove(const std::string& table_name, 
           const std::string& where_condition);
```
- Deletes rows matching the where condition

### Query Operation
```cpp
std::vector<std::unordered_map<std::string, std::string>> 
    select(const std::string& table_name, 
          const std::vector<std::string>& columns, 
          const std::string& where_condition);
```
- Returns all rows matching the where condition
- Currently implements simple selection without actual filtering

## File Structure
- `database.h`: Class declaration and interface
- `database.cpp`: Implementation of database operations
- `main.cpp`: Example usage (if exists)
- Database directory structure:
  - `metadata`: Stores table definitions
  - `operations.log`: Logs all database operations
  - Table directories (e.g., `users/`):
    - `columns`: Stores column definitions
    - Row files (named by ID): Store row data

## Example Usage
```cpp
#include "database.h"

int main() {
    Database db("my_database");
    
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "string"},
        {"name", "string"},
        {"age", "int"}
    };
    db.create_table("users", columns);
    
    std::unordered_map<std::string, std::string> user1 = {
        {"id", "1"},
        {"name", "Alice"},
        {"age", "30"}
    };
    db.insert("users", user1);
    
    auto results = db.select("users", {"name", "age"}, "age > 25");
    
    return 0; 
}
```
