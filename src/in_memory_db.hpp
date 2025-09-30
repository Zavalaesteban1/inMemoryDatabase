#ifndef IN_MEMORY_DB_HPP
#define IN_MEMORY_DB_HPP

#include <string>
#include <optional>
#include <map>
#include <vector>

/**
 * In-Memory Database Interface
 * 
 * Supports:
 * - Level 1: Basic record operations (set, delete, get)
 * - Level 2: Record filtering and display
 * - Level 3: TTL (Time-To-Live) functionality
 * - Level 4: Backup and restore operations
 */
class InMemoryDB {
public:
    /**
     * Set a field value for a given record
     * @param recordId Unique identifier for the record
     * @param field Field name (string)
     * @param value Field value (string)
     */
    virtual void set(const std::string& recordId, const std::string& field, const std::string& value) = 0;
    
    /**
     * Get a field value from a record
     * @param recordId Unique identifier for the record
     * @param field Field name to retrieve
     * @return Optional string containing the value, or nullopt if not found
     */
    virtual std::optional<std::string> get(const std::string& recordId, const std::string& field) const = 0;
    
    /**
     * Delete a field from a record
     * @param recordId Unique identifier for the record
     * @param field Field name to delete
     * @return true if field was deleted, false if field didn't exist
     */
    virtual bool deleteField(const std::string& recordId, const std::string& field) = 0;
    
    /**
     * Delete an entire record
     * @param recordId Unique identifier for the record
     * @return true if record was deleted, false if record didn't exist
     */
    virtual bool deleteRecord(const std::string& recordId) = 0;
    
    /**
     * Get all field names for a record
     * @param recordId Unique identifier for the record
     * @return Vector of field names, empty if record doesn't exist
     */
    virtual std::vector<std::string> getFields(const std::string& recordId) const = 0;
    
    /**
     * Check if a record exists
     * @param recordId Unique identifier for the record
     * @return true if record exists, false otherwise
     */
    virtual bool hasRecord(const std::string& recordId) const = 0;
    
    /**
     * Get all record IDs
     * @return Vector of all record IDs in the database
     */
    virtual std::vector<std::string> getAllRecordIds() const = 0;
    
    // Level 2: Filtering functionality
    /**
     * Get records that match a field-value filter
     * @param field Field name to filter on
     * @param value Field value to match
     * @return Vector of record IDs that have the specified field-value pair
     */
    virtual std::vector<std::string> getRecordsByFieldValue(const std::string& field, const std::string& value) const = 0;
    
    // Level 3: TTL functionality
    /**
     * Set TTL (Time-To-Live) for a record in seconds
     * @param recordId Unique identifier for the record
     * @param ttlSeconds TTL duration in seconds
     */
    virtual void setTTL(const std::string& recordId, int ttlSeconds) = 0;
    
    /**
     * Remove expired records based on TTL
     * @return Number of records that were expired and removed
     */
    virtual int expireRecords() = 0;
    
    // Level 4: Backup and restore
    /**
     * Create a backup of the entire database
     * @return String representation of the database state
     */
    virtual std::string backup() const = 0;
    
    /**
     * Restore database from a backup
     * @param backupData String representation of database state
     * @return true if restore was successful, false otherwise
     */
    virtual bool restore(const std::string& backupData) = 0;
    
    /**
     * Virtual destructor
     */
    virtual ~InMemoryDB() = default;
};

#endif // IN_MEMORY_DB_HPP
