#include "src/in_memory_db_imp.hpp"
#include <iostream>
#include <thread>
#include <chrono>

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << " " << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

int main() {
    InMemoryDBImpl db;
    
    printSeparator("In-Memory Database Demo");
    
    // Level 1: Basic Operations Demo
    printSeparator("Level 1: Basic Operations");
    
    std::cout << "Creating user records..." << std::endl;
    db.set("user_001", "name", "Alice Johnson");
    db.set("user_001", "email", "alice@company.com");
    db.set("user_001", "department", "Engineering");
    db.set("user_001", "role", "Senior Developer");
    
    db.set("user_002", "name", "Bob Smith");
    db.set("user_002", "email", "bob@company.com");
    db.set("user_002", "department", "Engineering");
    db.set("user_002", "role", "Team Lead");
    
    db.set("user_003", "name", "Carol Wilson");
    db.set("user_003", "email", "carol@company.com");
    db.set("user_003", "department", "Marketing");
    db.set("user_003", "role", "Marketing Manager");
    
    std::cout << "\nDatabase contents:" << std::endl;
    db.printAllRecords();
    
    // Demonstrate get operations
    std::cout << "Getting specific fields:" << std::endl;
    auto name = db.get("user_001", "name");
    auto email = db.get("user_001", "email");
    std::cout << "user_001 name: " << (name ? name.value() : "NOT FOUND") << std::endl;
    std::cout << "user_001 email: " << (email ? email.value() : "NOT FOUND") << std::endl;
    
    // Demonstrate field deletion
    std::cout << "\nDeleting email field from user_001..." << std::endl;
    db.deleteField("user_001", "email");
    auto emailAfterDelete = db.get("user_001", "email");
    std::cout << "user_001 email after deletion: " << (emailAfterDelete ? emailAfterDelete.value() : "NOT FOUND") << std::endl;
    
    // Level 2: Filtering Demo
    printSeparator("Level 2: Filtering Operations");
    
    std::cout << "Finding all Engineering department employees:" << std::endl;
    auto engineeringEmployees = db.getRecordsByFieldValue("department", "Engineering");
    for (const auto& recordId : engineeringEmployees) {
        auto name = db.get(recordId, "name");
        auto role = db.get(recordId, "role");
        std::cout << "- " << recordId << ": " << (name ? name.value() : "N/A") << " (" << (role ? role.value() : "N/A") << ")" << std::endl;
    }
    
    std::cout << "\nFinding all Team Leads:" << std::endl;
    auto teamLeads = db.getRecordsByFieldValue("role", "Team Lead");
    for (const auto& recordId : teamLeads) {
        auto name = db.get(recordId, "name");
        std::cout << "- " << recordId << ": " << (name ? name.value() : "N/A") << std::endl;
    }
    
    // Level 3: TTL Demo
    printSeparator("Level 3: TTL (Time-To-Live) Operations");
    
    std::cout << "Creating temporary sessions..." << std::endl;
    db.set("session_001", "user_id", "user_001");
    db.set("session_001", "login_time", "2024-01-01 09:00:00");
    db.set("session_001", "status", "active");
    
    db.set("session_002", "user_id", "user_002");
    db.set("session_002", "login_time", "2024-01-01 09:30:00");
    db.set("session_002", "status", "active");
    
    std::cout << "Setting TTL of 2 seconds for sessions..." << std::endl;
    db.setTTL("session_001", 2);
    db.setTTL("session_002", 2);
    
    std::cout << "\nCurrent sessions (before expiration):" << std::endl;
    auto allSessions = db.getRecordsByFieldValue("status", "active");
    std::cout << "Active sessions: " << allSessions.size() << std::endl;
    for (const auto& sessionId : allSessions) {
        auto userId = db.get(sessionId, "user_id");
        std::cout << "- " << sessionId << " (user: " << (userId ? userId.value() : "N/A") << ")" << std::endl;
    }
    
    std::cout << "\nWaiting for sessions to expire..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    int expiredCount = db.expireRecords();
    std::cout << "Expired " << expiredCount << " records" << std::endl;
    
    auto sessionsAfterExpiry = db.getRecordsByFieldValue("status", "active");
    std::cout << "Active sessions after expiry: " << sessionsAfterExpiry.size() << std::endl;
    
    // Level 4: Backup and Restore Demo
    printSeparator("Level 4: Backup and Restore Operations");
    
    std::cout << "Current database state:" << std::endl;
    auto currentRecords = db.getAllRecordIds();
    std::cout << "Records in database: " << currentRecords.size() << std::endl;
    for (const auto& recordId : currentRecords) {
        std::cout << "- " << recordId << std::endl;
    }
    
    std::cout << "\nCreating backup..." << std::endl;
    std::string backup = db.backup();
    std::cout << "Backup created (size: " << backup.length() << " characters)" << std::endl;
    
    std::cout << "\nAdding more data and then clearing database..." << std::endl;
    db.set("temp_record", "temp_field", "temp_value");
    
    // Clear all records
    auto recordsToDelete = db.getAllRecordIds();
    for (const auto& recordId : recordsToDelete) {
        db.deleteRecord(recordId);
    }
    
    std::cout << "Records after clearing: " << db.getAllRecordIds().size() << std::endl;
    
    std::cout << "\nRestoring from backup..." << std::endl;
    bool restored = db.restore(backup);
    std::cout << "Restore " << (restored ? "successful" : "failed") << std::endl;
    
    auto restoredRecords = db.getAllRecordIds();
    std::cout << "Records after restore: " << restoredRecords.size() << std::endl;
    for (const auto& recordId : restoredRecords) {
        std::cout << "- " << recordId << std::endl;
    }
    
    printSeparator("Final Database State");
    db.printAllRecords();
    
    printSeparator("Demo Complete");
    std::cout << "All levels of the in-memory database have been demonstrated!" << std::endl;
    
    return 0;
}
