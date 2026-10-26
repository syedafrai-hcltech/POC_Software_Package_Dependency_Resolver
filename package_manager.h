#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#define TABLE_SIZE 100

typedef struct
{
	int major;
	int minor;
	int patch;
} Version;

typedef enum
{
	WHITE,
	GRAY,
	BLACK
} VisitStatus;

typedef enum
{
	PENDING,
	INSTALLED,
	MISSING,
	CONFLICTING
} PackageStatus;

typedef struct Dependency
{
	char name[32];
	Version minVersion;
	struct Dependency *next;
} Dependency;

typedef struct Package
{
	char name[32];
	Version version;

	Dependency *deps;

	VisitStatus visit;

	PackageStatus status;

	struct Package *next;
	struct Package *hashNext;  // Hash chain

} Package;

extern Package *head;
extern Package *hashTable[TABLE_SIZE];

void addPackage(char name[],int major,int minor,int patch);
void updatePackage(char name[],int major,int minor,int patch);
Package* findPackage(char name[]);
void deletePackage(char name[]);
void listPackages();

void addDependency(char packageName[],
                   char dependencyName[],
                   int major,
                   int minor,
                   int patch);

void removeDependency(char packageName[],
                      char dependencyName[]);

void showDependencies(char packageName[]);

void checkVersionConflicts();
void detectCycles();
void generateInstallOrder();

void saveToFile();
void loadFromFile();

void showPackageStatus();
void freePackages();

#endif