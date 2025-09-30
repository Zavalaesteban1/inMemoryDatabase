#ifndef IN_MEMORY_DB_IMP_HPP
#define IN_MEMORY_DB_IMP_HPP

#include "in_memory_db.hpp"
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <iostream>

/**
 * Concrete implementation of the InMemoryDB interface
 */
class InMemoryDBImpl : public InMemoryDB {
private:
    // Record structure: recordId -> (field -> value)
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> records_;
    
    // TTL structure: recordId -> expiration timestamp
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> ttlMap_;
    
    /**
     * Helper function to check if a record has expired
     * @param recordId Unique identifier for the record
     * @return true if record has expired, false otherwise
     */
    bool isRecordExpired(const std::string& recordId) const;
    
    /**
     * Helper function to clean up expired record
     * @param recordId Unique identifier for the record
     */
    void cleanupExpiredRecord(const std::string& recordId);

public:
    /**
     * Constructor
     */
    InMemoryDBImpl();
    
    /**
     * Destructor
     */
    ~InMemoryDBImpl() override = default;
    
    // Level 1: Basic operations
    void set(const std::string& recordId, const std::string& field, const std::string& value) override;
    std::optional<std::string> get(const std::string& recordId, const std::string& field) const override;
    bool deleteField(const std::string& recordId, const std::string& field) override;
    bool deleteRecord(const std::string& recordId) override;
    std::vector<std::string> getFields(const std::string& recordId) const override;
    bool hasRecord(const std::string& recordId) const override;
    std::vector<std::string> getAllRecordIds() const override;
    
    // Level 2: Filtering functionality
    std::vector<std::string> getRecordsByFieldValue(const std::string& field, const std::string& value) const override;
    
    // Level 3: TTL functionality
    void setTTL(const std::string& recordId, int ttlSeconds) override;
    int expireRecords() override;
    
    // Level 4: Backup and restore
    std::string backup() const override;
    bool restore(const std::string& backupData) override;
    
    // Utility functions for debugging/testing
    void printAllRecords() const;
    size_t getRecordCount() const;
};

#endif // IN_MEMORY_DB_IMP_HPP
