#include "src/in_memory_db_imp.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

class DatabaseTester {
private:
    InMemoryDBImpl db;
    int testCount = 0;
    int passedTests = 0;
    
    void assert_test(bool condition, const std::string& testName) {
        testCount++;
        if (condition) {
            std::cout << "✓ " << testName << " PASSED" << std::endl;
            passedTests++;
        } else {
            std::cout << "✗ " << testName << " FAILED" << std::endl;
        }
    }
    
public:
    void runAllTests() {
        std::cout << "Starting In-Memory Database Tests..." << std::endl << std::endl;
        
        testLevel1();
        testLevel2();
        testLevel3();
        testLevel4();
        
        std::cout << std::endl << "Test Summary: " << passedTests << "/" << testCount << " tests passed" << std::endl;
        
        if (passedTests == testCount) {
            std::cout << "🎉 All tests passed!" << std::endl;
        } else {
            std::cout << "❌ Some tests failed. Please review the implementation." << std::endl;
        }
    }
    
    void testLevel1() {
        std::cout << "=== Level 1: Basic Operations ===" << std::endl;
        
        // Test 1: Set and Get
        db.set("user1", "name", "Alice");
        db.set("user1", "age", "25");
        auto name = db.get("user1", "name");
        auto age = db.get("user1", "age");
        
        assert_test(name.has_value() && name.value() == "Alice", "Set/Get name field");
        assert_test(age.has_value() && age.value() == "25", "Set/Get age field");
        
        // Test 2: Get non-existent field/record
        auto nonExistent = db.get("user1", "email");
        auto nonExistentRecord = db.get("user999", "name");
        
        assert_test(!nonExistent.has_value(), "Get non-existent field returns nullopt");
        assert_test(!nonExistentRecord.has_value(), "Get from non-existent record returns nullopt");
        
        // Test 3: Has record
        assert_test(db.hasRecord("user1"), "hasRecord returns true for existing record");
        assert_test(!db.hasRecord("user999"), "hasRecord returns false for non-existent record");
        
        // Test 4: Get fields
        auto fields = db.getFields("user1");
        assert_test(fields.size() == 2, "getFields returns correct count");
        assert_test(std::find(fields.begin(), fields.end(), "name") != fields.end(), "getFields contains name");
        assert_test(std::find(fields.begin(), fields.end(), "age") != fields.end(), "getFields contains age");
        
        // Test 5: Delete field
        bool deleted = db.deleteField("user1", "age");
        auto ageAfterDelete = db.get("user1", "age");
        
        assert_test(deleted, "deleteField returns true for existing field");
        assert_test(!ageAfterDelete.has_value(), "Field is deleted correctly");
        assert_test(db.hasRecord("user1"), "Record still exists after field deletion");
        
        // Test 6: Delete record
        bool recordDeleted = db.deleteRecord("user1");
        assert_test(recordDeleted, "deleteRecord returns true for existing record");
        assert_test(!db.hasRecord("user1"), "Record is deleted correctly");
        
        // Test 7: Multiple records
        db.set("user1", "name", "Alice");
        db.set("user2", "name", "Bob");
        db.set("user3", "name", "Charlie");
        
        auto allRecords = db.getAllRecordIds();
        assert_test(allRecords.size() == 3, "getAllRecordIds returns correct count");
        
        std::cout << std::endl;
    }
    
    void testLevel2() {
        std::cout << "=== Level 2: Filtering ===" << std::endl;
        
        // Setup data for filtering tests
        db.deleteRecord("user1");
        db.deleteRecord("user2");
        db.deleteRecord("user3");
        
        db.set("user1", "department", "engineering");
        db.set("user1", "role", "developer");
        db.set("user2", "department", "engineering");
        db.set("user2", "role", "manager");
        db.set("user3", "department", "marketing");
        db.set("user3", "role", "analyst");
        db.set("user4", "department", "engineering");
        db.set("user4", "role", "developer");
        
        // Test filtering by department
        auto engineeringUsers = db.getRecordsByFieldValue("department", "engineering");
        assert_test(engineeringUsers.size() == 3, "Filter by department returns correct count");
        
        // Test filtering by role
        auto developers = db.getRecordsByFieldValue("role", "developer");
        assert_test(developers.size() == 2, "Filter by role returns correct count");
        
        // Test filtering with no matches
        auto hrUsers = db.getRecordsByFieldValue("department", "hr");
        assert_test(hrUsers.empty(), "Filter with no matches returns empty vector");
        
        std::cout << std::endl;
    }
    
    void testLevel3() {
        std::cout << "=== Level 3: TTL (Time-To-Live) ===" << std::endl;
        
        // Clear database
        for (const auto& recordId : db.getAllRecordIds()) {
            db.deleteRecord(recordId);
        }
        
        // Test TTL setting and expiration
        db.set("temp1", "data", "temporary");
        db.set("temp2", "data", "also_temporary");
        db.set("permanent", "data", "permanent");
        
        // Set TTL for temp records (1 second)
        db.setTTL("temp1", 1);
        db.setTTL("temp2", 1);
        // No TTL for permanent record
        
        assert_test(db.hasRecord("temp1"), "Record exists before TTL expiration");
        assert_test(db.hasRecord("temp2"), "Record exists before TTL expiration");
        assert_test(db.hasRecord("permanent"), "Permanent record exists");
        
        // Wait for expiration
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        
        // Manually trigger expiration check
        int expiredCount = db.expireRecords();
        assert_test(expiredCount == 2, "expireRecords returns correct count");
        
        assert_test(!db.hasRecord("temp1"), "Expired record is removed");
        assert_test(!db.hasRecord("temp2"), "Expired record is removed");
        assert_test(db.hasRecord("permanent"), "Permanent record still exists");
        
        // Test accessing expired record
        auto expiredData = db.get("temp1", "data");
        assert_test(!expiredData.has_value(), "Get from expired record returns nullopt");
        
        std::cout << std::endl;
    }
    
    void testLevel4() {
        std::cout << "=== Level 4: Backup and Restore ===" << std::endl;
        
        // Clear and setup test data
        for (const auto& recordId : db.getAllRecordIds()) {
            db.deleteRecord(recordId);
        }
        
        db.set("backup_test1", "name", "Test User 1");
        db.set("backup_test1", "email", "test1@example.com");
        db.set("backup_test2", "name", "Test User 2");
        db.set("backup_test2", "email", "test2@example.com");
        db.set("backup_test3", "status", "active");
        
        // Set TTL for one record
        db.setTTL("backup_test3", 3600); // 1 hour
        
        // Create backup
        std::string backupData = db.backup();
        assert_test(!backupData.empty(), "Backup creates non-empty data");
        
        // Clear database
        for (const auto& recordId : db.getAllRecordIds()) {
            db.deleteRecord(recordId);
        }
        assert_test(db.getAllRecordIds().empty(), "Database is cleared before restore");
        
        // Restore from backup
        bool restored = db.restore(backupData);
        assert_test(restored, "Restore operation succeeds");
        
        // Verify restored data
        auto restoredRecords = db.getAllRecordIds();
        assert_test(restoredRecords.size() == 3, "All records restored correctly");
        
        auto name1 = db.get("backup_test1", "name");
        auto email1 = db.get("backup_test1", "email");
        auto name2 = db.get("backup_test2", "name");
        auto status3 = db.get("backup_test3", "status");
        
        assert_test(name1.has_value() && name1.value() == "Test User 1", "Restored record 1 name correct");
        assert_test(email1.has_value() && email1.value() == "test1@example.com", "Restored record 1 email correct");
        assert_test(name2.has_value() && name2.value() == "Test User 2", "Restored record 2 name correct");
        assert_test(status3.has_value() && status3.value() == "active", "Restored record 3 status correct");
        
        // Test restore with invalid data
        bool invalidRestore = db.restore("invalid backup data");
        assert_test(!invalidRestore, "Restore with invalid data fails gracefully");
        
        std::cout << std::endl;
    }
};

int main() {
    DatabaseTester tester;
    tester.runAllTests();
    return 0;
}
