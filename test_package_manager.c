#include <gtest/gtest.h>

extern "C"
{
#include "package_manager.h"
}

/* External symbols from package_manager.c */
extern int compareVersion(Version v1, Version v2);

class PackageManagerTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        freePackages();
    }

    void TearDown() override
    {
        freePackages();
    }
};

/* -------------------
   Add Package
-------------------- */

TEST_F(PackageManagerTest, AddPackage)
{
    char name[] = "A";

    addPackage(name,1,0,0);

    EXPECT_NE(nullptr,
              findPackage(name));
}

/* -------------------
   Search Package
-------------------- */

TEST_F(PackageManagerTest, SearchPackage)
{
    char name[] = "B";

    addPackage(name,1,0,0);

    EXPECT_NE(nullptr,
              findPackage(name));
}

/* -------------------
   Update Package
-------------------- */

TEST_F(PackageManagerTest, UpdatePackage)
{
    char name[] = "APP";

    addPackage(name,1,0,0);

    updatePackage(name,2,1,5);

    Package *p = findPackage(name);

    ASSERT_NE(nullptr,p);

    EXPECT_EQ(2,p->version.major);
    EXPECT_EQ(1,p->version.minor);
    EXPECT_EQ(5,p->version.patch);
}

/* -------------------
   Delete Package
-------------------- */

TEST_F(PackageManagerTest, DeletePackage)
{
    char name[] = "DEL";

    addPackage(name,1,0,0);

    deletePackage(name);

    EXPECT_EQ(nullptr,
              findPackage(name));
}

/* -------------------
   Add Dependency
-------------------- */

TEST_F(PackageManagerTest, AddDependency)
{
    char app[] = "APP";
    char lib[] = "LIB";

    addPackage(app,1,0,0);
    addPackage(lib,1,0,0);

    addDependency(app,
                  lib,
                  1,0,0);

    Package *pkg =
        findPackage(app);

    ASSERT_NE(nullptr,
              pkg->deps);

    EXPECT_STREQ("LIB",
                 pkg->deps->name);
}

/* -------------------
   Remove Dependency
-------------------- */

TEST_F(PackageManagerTest, RemoveDependency)
{
    char app[] = "APP";
    char lib[] = "LIB";

    addPackage(app,1,0,0);
    addPackage(lib,1,0,0);

    addDependency(app,
                  lib,
                  1,0,0);

    removeDependency(app,
                     lib);

    EXPECT_EQ(nullptr,
              findPackage(app)->deps);
}

/* -------------------
   Missing Dependency
-------------------- */

TEST_F(PackageManagerTest, MissingDependency)
{
    char app[] = "APP";
    char missing[] = "MISSING";

    addPackage(app,1,0,0);

    addDependency(app,
                  missing,
                  1,0,0);

    EXPECT_EQ(MISSING,
              findPackage(app)->status);
}

/* -------------------
   Version Comparison
-------------------- */

TEST_F(PackageManagerTest,
       HigherVersion)
{
    Version v1 = {2,0,0};
    Version v2 = {1,0,0};

    EXPECT_EQ(1,
              compareVersion(v1,v2));
}

TEST_F(PackageManagerTest,
       EqualVersion)
{
    Version v1 = {1,0,0};
    Version v2 = {1,0,0};

    EXPECT_EQ(0,
              compareVersion(v1,v2));
}

/* -------------------
   Version Conflict
-------------------- */

TEST_F(PackageManagerTest,
       VersionConflict)
{
    char app[] = "APP";
    char lib[] = "LIB";

    addPackage(app,1,0,0);
    addPackage(lib,1,0,0);

    addDependency(app,
                  lib,
                  5,0,0);

    checkVersionConflicts();

    EXPECT_EQ(CONFLICTING,
              findPackage(app)->status);
}

/* -------------------
   Save File
-------------------- */

TEST_F(PackageManagerTest,
       SaveToFile)
{
    char pkg[] = "PKG";

    addPackage(pkg,1,0,0);

    EXPECT_NO_THROW(
        saveToFile()
    );
}

/* -------------------
   Load File
-------------------- */

TEST_F(PackageManagerTest,
       LoadFromFile)
{
    char pkg[] = "LOAD";

    addPackage(pkg,1,0,0);

    saveToFile();

    freePackages();

    loadFromFile();

    EXPECT_NE(nullptr,
              findPackage(pkg));
}