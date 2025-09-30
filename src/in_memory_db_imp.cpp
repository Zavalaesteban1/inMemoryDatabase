#include "in_memory_db_imp.hpp"
#include <algorithm>
#include <iomanip>

InMemoryDBImpl::InMemoryDBImpl() {
    // Initialize empty database
}

// Helper functions
bool InMemoryDBImpl::isRecordExpired(const std::string& recordId) const {
    auto it = ttlMap_.find(recordId);
    if (it == ttlMap_.end()) {
        return false; // No TTL set, record doesn't expire
    }
    
    auto now = std::chrono::steady_clock::now();
    return now >= it->second;
}

void InMemoryDBImpl::cleanupExpiredRecord(const std::string& recordId) {
    records_.erase(recordId);
    ttlMap_.erase(recordId);
}

// Level 1: Basic operations
void InMemoryDBImpl::set(const std::string& recordId, const std::string& field, const std::string& value) {
    // Check if record is expired before setting
    if (isRecordExpired(recordId)) {
        cleanupExpiredRecord(recordId);
    }
    
    records_[recordId][field] = value;
}

std::optional<std::string> InMemoryDBImpl::get(const std::string& recordId, const std::string& field) const {
    // Check if record is expired
    if (isRecordExpired(recordId)) {
        return std::nullopt;
    }
    
    auto recordIt = records_.find(recordId);
    if (recordIt == records_.end()) {
        return std::nullopt; // Record doesn't exist
    }
    
    auto fieldIt = recordIt->second.find(field);
    if (fieldIt == recordIt->second.end()) {
        return std::nullopt; // Field doesn't exist
    }
    
    return fieldIt->second;
}

bool InMemoryDBImpl::deleteField(const std::string& recordId, const std::string& field) {
    // Check if record is expired
    if (isRecordExpired(recordId)) {
        cleanupExpiredRecord(recordId);
        return false;
    }
    
    auto recordIt = records_.find(recordId);
    if (recordIt == records_.end()) {
        return false; // Record doesn't exist
    }
    
    auto fieldIt = recordIt->second.find(field);
    if (fieldIt == recordIt->second.end()) {
        return false; // Field doesn't exist
    }
    
    recordIt->second.erase(fieldIt);
    
    // If record becomes empty, remove it entirely
    if (recordIt->second.empty()) {
        records_.erase(recordIt);
        ttlMap_.erase(recordId);
    }
    
    return true;
}

bool InMemoryDBImpl::deleteRecord(const std::string& recordId) {
    auto recordIt = records_.find(recordId);
    if (recordIt == records_.end()) {
        return false; // Record doesn't exist
    }
    
    records_.erase(recordIt);
    ttlMap_.erase(recordId);
    return true;
}

std::vector<std::string> InMemoryDBImpl::getFields(const std::string& recordId) const {
    // Check if record is expired
    if (isRecordExpired(recordId)) {
        return {}; // Return empty vector for expired records
    }
    
    auto recordIt = records_.find(recordId);
    if (recordIt == records_.end()) {
        return {}; // Record doesn't exist
    }
    
    std::vector<std::string> fields;
    fields.reserve(recordIt->second.size());
    
    for (const auto& pair : recordIt->second) {
        fields.push_back(pair.first);
    }
    
    std::sort(fields.begin(), fields.end()); // Sort for consistent ordering
    return fields;
}

bool InMemoryDBImpl::hasRecord(const std::string& recordId) const {
    // Check if record is expired
    if (isRecordExpired(recordId)) {
        return false;
    }
    
    return records_.find(recordId) != records_.end();
}

std::vector<std::string> InMemoryDBImpl::getAllRecordIds() const {
    std::vector<std::string> recordIds;
    
    for (const auto& pair : records_) {
        // Only include non-expired records
        if (!isRecordExpired(pair.first)) {
            recordIds.push_back(pair.first);
        }
    }
    
    std::sort(recordIds.begin(), recordIds.end()); // Sort for consistent ordering
    return recordIds;
}

// Level 2: Filtering functionality
std::vector<std::string> InMemoryDBImpl::getRecordsByFieldValue(const std::string& field, const std::string& value) const {
    std::vector<std::string> matchingRecords;
    
    for (const auto& recordPair : records_) {
        const std::string& recordId = recordPair.first;
        const auto& fields = recordPair.second;
        
        // Skip expired records
        if (isRecordExpired(recordId)) {
            continue;
        }
        
        auto fieldIt = fields.find(field);
        if (fieldIt != fields.end() && fieldIt->second == value) {
            matchingRecords.push_back(recordId);
        }
    }
    
    std::sort(matchingRecords.begin(), matchingRecords.end()); // Sort for consistent ordering
    return matchingRecords;
}

// Level 3: TTL functionality
void InMemoryDBImpl::setTTL(const std::string& recordId, int ttlSeconds) {
    // Only set TTL if record exists
    if (records_.find(recordId) == records_.end()) {
        return; // Record doesn't exist
    }
    
    auto expirationTime = std::chrono::steady_clock::now() + std::chrono::seconds(ttlSeconds);
    ttlMap_[recordId] = expirationTime;
}

int InMemoryDBImpl::expireRecords() {
    int expiredCount = 0;
    auto now = std::chrono::steady_clock::now();
    
    // Collect expired record IDs to avoid iterator invalidation
    std::vector<std::string> expiredRecords;
    
    for (const auto& ttlPair : ttlMap_) {
        if (now >= ttlPair.second) {
            expiredRecords.push_back(ttlPair.first);
        }
    }
    
    // Remove expired records
    for (const std::string& recordId : expiredRecords) {
        cleanupExpiredRecord(recordId);
        expiredCount++;
    }
    
    return expiredCount;
}

// Level 4: Backup and restore
std::string InMemoryDBImpl::backup() const {
    std::ostringstream backup;
    
    // Format: RECORD_COUNT\n
    // For each record: RECORD_ID\nFIELD_COUNT\nFIELD1\nVALUE1\nFIELD2\nVALUE2\n...
    // TTL_COUNT\n
    // For each TTL: RECORD_ID\nTTL_SECONDS_REMAINING\n
    
    std::vector<std::string> validRecordIds;
    for (const auto& recordPair : records_) {
        if (!isRecordExpired(recordPair.first)) {
            validRecordIds.push_back(recordPair.first);
        }
    }
    
    backup << validRecordIds.size() << "\n";
    
    for (const std::string& recordId : validRecordIds) {
        const auto& fields = records_.at(recordId);
        backup << recordId << "\n";
        backup << fields.size() << "\n";
        
        for (const auto& fieldPair : fields) {
            backup << fieldPair.first << "\n";
            backup << fieldPair.second << "\n";
        }
    }
    
    // Backup TTL information
    auto now = std::chrono::steady_clock::now();
    std::vector<std::pair<std::string, int>> validTTLs;
    
    for (const std::string& recordId : validRecordIds) {
        auto ttlIt = ttlMap_.find(recordId);
        if (ttlIt != ttlMap_.end()) {
            auto remainingTime = std::chrono::duration_cast<std::chrono::seconds>(ttlIt->second - now);
            if (remainingTime.count() > 0) {
                validTTLs.push_back({recordId, static_cast<int>(remainingTime.count())});
            }
        }
    }
    
    backup << validTTLs.size() << "\n";
    for (const auto& ttlPair : validTTLs) {
        backup << ttlPair.first << "\n";
        backup << ttlPair.second << "\n";
    }
    
    return backup.str();
}

bool InMemoryDBImpl::restore(const std::string& backupData) {
    try {
        std::istringstream stream(backupData);
        std::string line;
        
        // Clear current database
        records_.clear();
        ttlMap_.clear();
        
        // Read record count
        if (!std::getline(stream, line)) return false;
        int recordCount = std::stoi(line);
        
        // Read records
        for (int i = 0; i < recordCount; i++) {
            if (!std::getline(stream, line)) return false;
            std::string recordId = line;
            
            if (!std::getline(stream, line)) return false;
            int fieldCount = std::stoi(line);
            
            for (int j = 0; j < fieldCount; j++) {
                if (!std::getline(stream, line)) return false;
                std::string field = line;
                
                if (!std::getline(stream, line)) return false;
                std::string value = line;
                
                records_[recordId][field] = value;
            }
        }
        
        // Read TTL count
        if (!std::getline(stream, line)) return false;
        int ttlCount = std::stoi(line);
        
        // Read TTLs
        auto now = std::chrono::steady_clock::now();
        for (int i = 0; i < ttlCount; i++) {
            if (!std::getline(stream, line)) return false;
            std::string recordId = line;
            
            if (!std::getline(stream, line)) return false;
            int ttlSeconds = std::stoi(line);
            
            ttlMap_[recordId] = now + std::chrono::seconds(ttlSeconds);
        }
        
        return true;
    } catch (const std::exception&) {
        // Clear database on restore failure
        records_.clear();
        ttlMap_.clear();
        return false;
    }
}

// Utility functions
void InMemoryDBImpl::printAllRecords() const {
    std::cout << "=== Database Contents ===" << std::endl;
    std::vector<std::string> recordIds = getAllRecordIds();
    
    if (recordIds.empty()) {
        std::cout << "Database is empty." << std::endl;
        return;
    }
    
    for (const std::string& recordId : recordIds) {
        std::cout << "Record: " << recordId << std::endl;
        std::vector<std::string> fields = getFields(recordId);
        
        for (const std::string& field : fields) {
            auto value = get(recordId, field);
            if (value.has_value()) {
                std::cout << "  " << field << " = " << value.value() << std::endl;
            }
        }
        
        // Show TTL if set
        auto ttlIt = ttlMap_.find(recordId);
        if (ttlIt != ttlMap_.end()) {
            auto now = std::chrono::steady_clock::now();
            auto remainingTime = std::chrono::duration_cast<std::chrono::seconds>(ttlIt->second - now);
            std::cout << "  [TTL: " << remainingTime.count() << " seconds remaining]" << std::endl;
        }
        
        std::cout << std::endl;
    }
}

size_t InMemoryDBImpl::getRecordCount() const {
    return getAllRecordIds().size();
}
