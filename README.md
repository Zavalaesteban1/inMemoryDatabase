# In-Memory Database Implementation

A simplified in-memory database implementation in C++ with support for multiple levels of functionality.

## Features

### Level 1: Basic Operations
- **Set**: Store field-value pairs for records identified by unique string IDs
- **Get**: Retrieve field values from records (returns `std::optional<string>`)
- **Delete**: Remove individual fields or entire records
- **Query**: Check record existence and get field lists

### Level 2: Filtering
- **Filter by field-value**: Find all records matching a specific field-value combination
- **Record discovery**: Get lists of records based on criteria

### Level 3: TTL (Time-To-Live)
- **Expiration management**: Set TTL for records with automatic expiration
- **Cleanup**: Manual and automatic removal of expired records
- **Time-based queries**: Expired records are automatically excluded from operations

### Level 4: Backup and Restore
- **Serialization**: Create string-based backups of the entire database state
- **Restoration**: Restore database from backup data
- **Data integrity**: Maintains TTL information across backup/restore cycles

## Project Structure

```
├── src/
│   ├── in_memory_db.hpp           # Abstract interface definition
│   ├── in_memory_db_imp.hpp       # Implementation class header
│   └── in_memory_db_imp.cpp       # Implementation source code
├── test_db.cpp                    # Comprehensive test suite
├── demo.cpp                       # Interactive demonstration
├── run_single_test.sh            # Test runner script
├── Makefile                      # Build configuration
└── README.md                     # This documentation
```

## Building and Running

### Using Make (Recommended)

```bash
# Compile everything
make

# Run tests
make test

# Run interactive demo
make demo

# Clean build artifacts
make clean
```

### Using the Test Runner Script

```bash
# Run all tests
bash run_single_test.sh all

# Run specific level tests (currently runs all tests)
bash run_single_test.sh level1
bash run_single_test.sh level2
bash run_single_test.sh level3
bash run_single_test.sh level4

# Just compile
bash run_single_test.sh compile
```

### Manual Compilation

```bash
# Create build directory
mkdir -p build

# Compile tests
g++ -std=c++17 -Wall -Wextra -O2 -I. test_db.cpp src/in_memory_db_imp.cpp -o build/test_db

# Compile demo
g++ -std=c++17 -Wall -Wextra -O2 -I. demo.cpp src/in_memory_db_imp.cpp -o build/demo

# Run
./build/test_db
./build/demo
```

## API Reference

### Core Operations

```cpp
// Create database instance
InMemoryDBImpl db;

// Set field values
db.set("user_001", "name", "Alice");
db.set("user_001", "email", "alice@example.com");

// Get field values
auto name = db.get("user_001", "name");  // Returns std::optional<string>
if (name.has_value()) {
    std::cout << "Name: " << name.value() << std::endl;
}

// Delete operations
db.deleteField("user_001", "email");    // Delete single field
db.deleteRecord("user_001");            // Delete entire record
```

### Filtering Operations

```cpp
// Find records by field-value
auto engineers = db.getRecordsByFieldValue("department", "engineering");
for (const auto& recordId : engineers) {
    // Process each matching record
    auto name = db.get(recordId, "name");
    // ...
}
```

### TTL Operations

```cpp
// Set TTL (Time-To-Live) in seconds
db.setTTL("session_001", 3600);  // Expires in 1 hour

// Manual expiration check
int expiredCount = db.expireRecords();
std::cout << "Expired " << expiredCount << " records" << std::endl;
```

### Backup and Restore

```cpp
// Create backup
std::string backup = db.backup();

// Save backup to file or send over network...

// Restore from backup
bool success = db.restore(backup);
if (success) {
    std::cout << "Database restored successfully" << std::endl;
}
```

## Design Decisions

### Data Structure
- Uses `std::unordered_map` for O(1) average-case record and field lookups
- Records are stored as nested maps: `recordId -> (field -> value)`
- TTL information is stored separately to avoid overhead for non-TTL records

### Memory Management
- All data stored in memory (no persistence to disk by default)
- Automatic cleanup of expired records during access operations
- Manual cleanup available via `expireRecords()`

### Thread Safety
- **Not thread-safe**: This implementation is designed for single-threaded use
- For multi-threaded environments, external synchronization would be required

### Error Handling
- Uses `std::optional` for safe nullable returns
- Boolean return values for operations that can fail
- Graceful handling of expired records and missing data

## Testing

The test suite covers:
- ✅ All basic CRUD operations
- ✅ Record and field existence checks
- ✅ Filtering functionality
- ✅ TTL expiration behavior
- ✅ Backup and restore operations
- ✅ Edge cases and error conditions

Run `make test` to execute the full test suite.

## Performance Characteristics

- **Set**: O(1) average case
- **Get**: O(1) average case
- **Delete**: O(1) average case
- **Filter**: O(n) where n is the number of records
- **Expire**: O(k) where k is the number of records with TTL
- **Backup**: O(n) where n is the total number of field-value pairs
- **Restore**: O(n) where n is the size of backup data

## Requirements

- C++17 or later
- GCC/Clang with standard library support
- Unix-like system for shell scripts (optional)

## License

This implementation is provided as-is for educational and evaluation purposes.
