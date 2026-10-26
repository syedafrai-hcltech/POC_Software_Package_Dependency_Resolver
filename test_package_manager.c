
#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>

extern "C" {
#include "package_manager.h"

int compareVersion(Version v1, Version v2);
extern char installOrder[100][32];
extern int orderIndex;
}

class PackageManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        freePackages();
        std::remove("packages.txt");
    }

    void TearDown() override {
        freePackages();
        std::remove("packages.txt");
    }

    void createPackage(const char* name, int major = 1,
                       int minor = 0, int patch = 0) {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%s", name);
        addPackage(buffer, major, minor, patch);
    }

    void createDependency(const char* package, const char* dependency,
                          int major = 1, int minor = 0, int patch = 0) {
        char pkg[32], dep[32];
        std::snprintf(pkg, sizeof(pkg), "%s", package);
        std::snprintf(dep, sizeof(dep), "%s", dependency);
        addDependency(pkg, dep, major, minor, patch);
    }
};

// -------------------- Package management --------------------

TEST_F(PackageManagerTest, AddPackage) {
    createPackage("A", 1, 2, 3);

    Package* p = findPackage(const_cast<char*>("A"));
    ASSERT_NE(nullptr, p);
    EXPECT_EQ(1, p->version.major);
    EXPECT_EQ(2, p->version.minor);
    EXPECT_EQ(3, p->version.patch);
    EXPECT_EQ(PENDING, p->status);
    EXPECT_EQ(WHITE, p->visit);
    EXPECT_EQ(nullptr, p->deps);
}

TEST_F(PackageManagerTest, AddMultiplePackages) {
    createPackage("A");
    createPackage("B");
    createPackage("C");

    ASSERT_NE(nullptr, head);
    EXPECT_STREQ("A", head->name);
    ASSERT_NE(nullptr, head->next);
    EXPECT_STREQ("B", head->next->name);
    ASSERT_NE(nullptr, head->next->next);
    EXPECT_STREQ("C", head->next->next->name);
    EXPECT_EQ(nullptr, head->next->next->next);
}

TEST_F(PackageManagerTest, DuplicatePackageIsRejected) {
    createPackage("A", 1, 0, 0);
    createPackage("A", 9, 9, 9);

    Package* p = findPackage(const_cast<char*>("A"));
    ASSERT_NE(nullptr, p);
    EXPECT_EQ(1, p->version.major);
    EXPECT_EQ(0, p->version.minor);
    EXPECT_EQ(0, p->version.patch);
    EXPECT_EQ(1, head == nullptr ? 0 : (head->next == nullptr ? 1 : 2));
}

TEST_F(PackageManagerTest, FindExistingPackage) {
    createPackage("ALPHA");
    EXPECT_NE(nullptr, findPackage(const_cast<char*>("ALPHA")));
}

TEST_F(PackageManagerTest, FindMissingPackage) {
    EXPECT_EQ(nullptr, findPackage(const_cast<char*>("UNKNOWN")));
}

TEST_F(PackageManagerTest, UpdateExistingPackage) {
    createPackage("APP");
    updatePackage(const_cast<char*>("APP"), 2, 3, 4);

    Package* p = findPackage(const_cast<char*>("APP"));
    ASSERT_NE(nullptr, p);
    EXPECT_EQ(2, p->version.major);
    EXPECT_EQ(3, p->version.minor);
    EXPECT_EQ(4, p->version.patch);
}

TEST_F(PackageManagerTest, UpdateMissingPackage) {
    updatePackage(const_cast<char*>("UNKNOWN"), 2, 0, 0);
    EXPECT_EQ(nullptr, findPackage(const_cast<char*>("UNKNOWN")));
}

TEST_F(PackageManagerTest, ListPackagesWhenEmpty) {
    EXPECT_NO_THROW(listPackages());
}

TEST_F(PackageManagerTest, ListPackagesWhenNonEmpty) {
    createPackage("A");
    createPackage("B");
    EXPECT_NO_THROW(listPackages());
}

TEST_F(PackageManagerTest, DeleteMissingPackage) {
    createPackage("A");
    deletePackage(const_cast<char*>("UNKNOWN"));

    EXPECT_NE(nullptr, findPackage(const_cast<char*>("A")));
}

TEST_F(PackageManagerTest, DeleteOnlyPackage) {
    createPackage("A");
    deletePackage(const_cast<char*>("A"));

    EXPECT_EQ(nullptr, head);
    EXPECT_EQ(nullptr, findPackage(const_cast<char*>("A")));
}

TEST_F(PackageManagerTest, DeleteHeadPackage) {
    createPackage("A");
    createPackage("B");

    deletePackage(const_cast<char*>("A"));

    ASSERT_NE(nullptr, head);
    EXPECT_STREQ("B", head->name);
    EXPECT_EQ(nullptr, findPackage(const_cast<char*>("A")));
}

TEST_F(PackageManagerTest, DeleteTailPackage) {
    createPackage("A");
    createPackage("B");

    deletePackage(const_cast<char*>("B"));

    ASSERT_NE(nullptr, head);
    EXPECT_STREQ("A", head->name);
    EXPECT_EQ(nullptr, head->next);
}

TEST_F(PackageManagerTest, DeleteMiddlePackage) {
    createPackage("A");
    createPackage("B");
    createPackage("C");

    deletePackage(const_cast<char*>("B"));

    ASSERT_NE(nullptr, head);
    ASSERT_NE(nullptr, head->next);
    EXPECT_STREQ("A", head->name);
    EXPECT_STREQ("C", head->next->name);
    EXPECT_EQ(nullptr, findPackage(const_cast<char*>("B")));
}

// -------------------- Version comparison --------------------

TEST_F(PackageManagerTest, CompareHigherMajorVersion) {
    EXPECT_EQ(1, compareVersion({2, 0, 0}, {1, 9, 9}));
}

TEST_F(PackageManagerTest, CompareLowerMajorVersion) {
    EXPECT_EQ(-1, compareVersion({1, 9, 9}, {2, 0, 0}));
}

TEST_F(PackageManagerTest, CompareHigherMinorVersion) {
    EXPECT_EQ(1, compareVersion({1, 2, 0}, {1, 1, 9}));
}

TEST_F(PackageManagerTest, CompareLowerMinorVersion) {
    EXPECT_EQ(-1, compareVersion({1, 1, 9}, {1, 2, 0}));
}

TEST_F(PackageManagerTest, CompareHigherPatchVersion) {
    EXPECT_EQ(1, compareVersion({1, 2, 3}, {1, 2, 2}));
}

TEST_F(PackageManagerTest, CompareLowerPatchVersion) {
    EXPECT_EQ(-1, compareVersion({1, 2, 2}, {1, 2, 3}));
}

TEST_F(PackageManagerTest, CompareEqualVersions) {
    EXPECT_EQ(0, compareVersion({1, 2, 3}, {1, 2, 3}));
}

// -------------------- Dependencies --------------------

TEST_F(PackageManagerTest, AddDependencySuccessfully) {
    createPackage("APP");
    createPackage("LIB");
    createDependency("APP", "LIB", 1, 2, 3);

    Package* app = findPackage(const_cast<char*>("APP"));
    ASSERT_NE(nullptr, app);
    ASSERT_NE(nullptr, app->deps);
    EXPECT_STREQ("LIB", app->deps->name);
    EXPECT_EQ(1, app->deps->minVersion.major);
    EXPECT_EQ(2, app->deps->minVersion.minor);
    EXPECT_EQ(3, app->deps->minVersion.patch);
}

TEST_F(PackageManagerTest, AddDependencyToMissingPackage) {
    createPackage("LIB");
    createDependency("UNKNOWN", "LIB");

    EXPECT_EQ(nullptr, findPackage(const_cast<char*>("UNKNOWN")));
}

TEST_F(PackageManagerTest, AddMissingDependencyMarksPackageMissing) {
    createPackage("APP");
    createDependency("APP", "UNKNOWN");

    Package* app = findPackage(const_cast<char*>("APP"));
    ASSERT_NE(nullptr, app);
    EXPECT_EQ(MISSING, app->status);
    EXPECT_EQ(nullptr, app->deps);
}

TEST_F(PackageManagerTest, SelfDependencyIsRejected) {
    createPackage("APP");
    createDependency("APP", "APP");

    Package* app = findPackage(const_cast<char*>("APP"));
    ASSERT_NE(nullptr, app);
    EXPECT_EQ(nullptr, app->deps);
}

TEST_F(PackageManagerTest, DuplicateDependencyIsRejected) {
    createPackage("APP");
    createPackage("LIB");

    createDependency("APP", "LIB");
    createDependency("APP", "LIB");

    Package* app = findPackage(const_cast<char*>("APP"));
    ASSERT_NE(nullptr, app);
    ASSERT_NE(nullptr, app->deps);
    EXPECT_EQ(nullptr, app->deps->next);
}

TEST_F(PackageManagerTest, AddMultipleDependencies) {
    createPackage("APP");
    createPackage("LIB1");
    createPackage("LIB2");

    createDependency("APP", "LIB1");
    createDependency("APP", "LIB2");

    Package* app = findPackage(const_cast<char*>("APP"));
    ASSERT_NE(nullptr, app);
    ASSERT_NE(nullptr, app->deps);
    ASSERT_NE(nullptr, app->deps->next);
    EXPECT_STREQ("LIB1", app->deps->name);
    EXPECT_STREQ("LIB2", app->deps->next->name);
}

TEST_F(PackageManagerTest, ShowDependenciesForMissingPackage) {
    EXPECT_NO_THROW(showDependencies(const_cast<char*>("UNKNOWN")));
}

TEST_F(PackageManagerTest, ShowDependenciesForPackageWithoutDependencies) {
    createPackage("APP");
    EXPECT_NO_THROW(showDependencies(const_cast<char*>("APP")));
}

TEST_F(PackageManagerTest, ShowDependenciesForPackageWithDependencies) {
    createPackage("APP");
    createPackage("LIB");
    createDependency("APP", "LIB");

    EXPECT_NO_THROW(showDependencies(const_cast<char*>("APP")));
}

TEST_F(PackageManagerTest, RemoveDependencyFromMissingPackage) {
    removeDependency(const_cast<char*>("UNKNOWN"),
                     const_cast<char*>("LIB"));
}

TEST_F(PackageManagerTest, RemoveMissingDependency) {
    createPackage("APP");
    createPackage("LIB");

    removeDependency(const_cast<char*>("APP"),
                     const_cast<char*>("LIB"));

    EXPECT_EQ(nullptr, findPackage(const_cast<char*>("APP"))->deps);
}

TEST_F(PackageManagerTest, RemoveFirstDependency) {
    createPackage("APP");
    createPackage("LIB1");
    createPackage("LIB2");

    createDependency("APP", "LIB1");
    createDependency("APP", "LIB2");

    removeDependency(const_cast<char*>("APP"),
                     const_cast<char*>("LIB1"));

    Package* app = findPackage(const_cast<char*>("APP"));
    ASSERT_NE(nullptr, app);
    ASSERT_NE(nullptr, app->deps);
    EXPECT_STREQ("LIB2", app->deps->name);
    EXPECT_EQ(nullptr, app->deps->next);
}

TEST_F(PackageManagerTest, RemoveLastDependency) {
    createPackage("APP");
    createPackage("LIB1");
    createPackage("LIB2");

    createDependency("APP", "LIB1");
    createDependency("APP", "LIB2");

    removeDependency(const_cast<char*>("APP"),
                     const_cast<char*>("LIB2"));

    Package* app = findPackage(const_cast<char*>("APP"));
    ASSERT_NE(nullptr, app);
    ASSERT_NE(nullptr, app->deps);
    EXPECT_STREQ("LIB1", app->deps->name);
    EXPECT_EQ(nullptr, app->deps->next);
}

// -------------------- Version conflicts --------------------

TEST_F(PackageManagerTest, VersionConflictIsDetected) {
    createPackage("APP");
    createPackage("LIB", 1, 0, 0);
    createDependency("APP", "LIB", 2, 0, 0);

    checkVersionConflicts();

    EXPECT_EQ(CONFLICTING,
              findPackage(const_cast<char*>("APP"))->status);
}

TEST_F(PackageManagerTest, SufficientDependencyVersionHasNoConflict) {
    createPackage("APP");
    createPackage("LIB", 2, 0, 0);
    createDependency("APP", "LIB", 1, 0, 0);

    checkVersionConflicts();

    EXPECT_EQ(PENDING,
              findPackage(const_cast<char*>("APP"))->status);
}

TEST_F(PackageManagerTest, ConflictComparisonUsesMinorVersion) {
    createPackage("APP");
    createPackage("LIB", 1, 1, 0);
    createDependency("APP", "LIB", 1, 2, 0);

    checkVersionConflicts();

    EXPECT_EQ(CONFLICTING,
              findPackage(const_cast<char*>("APP"))->status);
}

TEST_F(PackageManagerTest, ConflictComparisonUsesPatchVersion) {
    createPackage("APP");
    createPackage("LIB", 1, 2, 2);
    createDependency("APP", "LIB", 1, 2, 3);

    checkVersionConflicts();

    EXPECT_EQ(CONFLICTING,
              findPackage(const_cast<char*>("APP"))->status);
}

// -------------------- Cycle detection --------------------

TEST_F(PackageManagerTest, DetectCyclesWithNoPackages) {
    EXPECT_NO_THROW(detectCycles());
}

TEST_F(PackageManagerTest, DetectCyclesInAcyclicGraph) {
    createPackage("APP");
    createPackage("LIB");
    createDependency("APP", "LIB");

    EXPECT_NO_THROW(detectCycles());

    EXPECT_EQ(BLACK, findPackage(const_cast<char*>("APP"))->visit);
    EXPECT_EQ(BLACK, findPackage(const_cast<char*>("LIB"))->visit);
}

TEST_F(PackageManagerTest, DetectDirectCycle) {
    createPackage("A");
    createPackage("B");
    createDependency("A", "B");
    createDependency("B", "A");

    EXPECT_NO_THROW(detectCycles());
}

TEST_F(PackageManagerTest, DetectLongerCycle) {
    createPackage("A");
    createPackage("B");
    createPackage("C");

    createDependency("A", "B");
    createDependency("B", "C");
    createDependency("C", "A");

    EXPECT_NO_THROW(detectCycles());
}

TEST_F(PackageManagerTest, GenerateInstallOrderWithNoPackages) {
    EXPECT_NO_THROW(generateInstallOrder());
    EXPECT_EQ(0, orderIndex);
}

TEST_F(PackageManagerTest, GenerateInstallOrderForDependencies) {
    createPackage("APP");
    createPackage("LIB");

    createDependency("APP", "LIB");

    generateInstallOrder();

    EXPECT_EQ(INSTALLED, findPackage(const_cast<char*>("APP"))->status);
    EXPECT_EQ(INSTALLED, findPackage(const_cast<char*>("LIB"))->status);
    EXPECT_EQ(2, orderIndex);

    // Dependencies should be installed before dependents.
    EXPECT_STREQ("LIB", installOrder[0]);
    EXPECT_STREQ("APP", installOrder[1]);
}

TEST_F(PackageManagerTest, GenerateInstallOrderWithCycle) {
    createPackage("A");
    createPackage("B");
    createDependency("A", "B");
    createDependency("B", "A");

    EXPECT_NO_THROW(generateInstallOrder());
    EXPECT_NE(INSTALLED, findPackage(const_cast<char*>("A"))->status);
}

// -------------------- Save and load --------------------

TEST_F(PackageManagerTest, SaveEmptyPackageList) {
    EXPECT_NO_THROW(saveToFile());
}

TEST_F(PackageManagerTest, SavePackageAndDependency) {
    createPackage("APP");
    createPackage("LIB");
    createDependency("APP", "LIB", 1, 2, 3);

    saveToFile();

    FILE* fp = std::fopen("packages.txt", "r");
    ASSERT_NE(nullptr, fp);
    std::fclose(fp);
}

TEST_F(PackageManagerTest, LoadWhenFileDoesNotExist) {
    std::remove("packages.txt");

    EXPECT_NO_THROW(loadFromFile());
}

TEST_F(PackageManagerTest, LoadPackagesFromFile) {
    FILE* fp = std::fopen("packages.txt", "w");
    ASSERT_NE(nullptr, fp);

    std::fputs("P APP 1 2 3\n", fp);
    std::fputs("P LIB 2 0 0\n", fp);
    std::fputs("D APP LIB 1 0 0\n", fp);
    std::fclose(fp);

    loadFromFile();

    Package* app = findPackage(const_cast<char*>("APP"));
    Package* lib = findPackage(const_cast<char*>("LIB"));

    ASSERT_NE(nullptr, app);
    ASSERT_NE(nullptr, lib);
    EXPECT_EQ(1, app->version.major);
    EXPECT_EQ(2, app->version.minor);
    EXPECT_EQ(3, app->version.patch);
    ASSERT_NE(nullptr, app->deps);
    EXPECT_STREQ("LIB", app->deps->name);
}

TEST_F(PackageManagerTest, LoadIgnoresUnknownRecordType) {
    FILE* fp = std::fopen("packages.txt", "w");
    ASSERT_NE(nullptr, fp);

    std::fputs("X ignored record\n", fp);
    std::fputs("P APP 1 0 0\n", fp);
    std::fclose(fp);

    EXPECT_NO_THROW(loadFromFile());
    EXPECT_NE(nullptr, findPackage(const_cast<char*>("APP")));
}

// -------------------- Status and cleanup --------------------

TEST_F(PackageManagerTest, ShowStatusForEmptyList) {
    EXPECT_NO_THROW(showPackageStatus());
}

TEST_F(PackageManagerTest, ShowStatusForPendingPackage) {
    createPackage("APP");
    EXPECT_NO_THROW(showPackageStatus());
}

TEST_F(PackageManagerTest, ShowStatusForInstalledPackage) {
    createPackage("APP");
    findPackage(const_cast<char*>("APP"))->status = INSTALLED;

    EXPECT_NO_THROW(showPackageStatus());
}

TEST_F(PackageManagerTest, ShowStatusForMissingPackage) {
    createPackage("APP");
    findPackage(const_cast<char*>("APP"))->status = MISSING;

    EXPECT_NO_THROW(showPackageStatus());
}

TEST_F(PackageManagerTest, ShowStatusForConflictingPackage) {
    createPackage("APP");
    findPackage(const_cast<char*>("APP"))->status = CONFLICTING;

    EXPECT_NO_THROW(showPackageStatus());
}

TEST_F(PackageManagerTest, FreePackagesClearsAllPackages) {
    createPackage("APP");
    createPackage("LIB");
    createDependency("APP", "LIB");

    freePackages();

    EXPECT_EQ(nullptr, head);
    EXPECT_EQ(nullptr, findPackage(const_cast<char*>("APP")));
    EXPECT_EQ(nullptr, findPackage(const_cast<char*>("LIB")));

    for (int i = 0; i < TABLE_SIZE; ++i) {
        EXPECT_EQ(nullptr, hashTable[i]);
    }
}

