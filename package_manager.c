#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "package_manager.h"

Package *head = NULL;
Package *hashTable[TABLE_SIZE];

char installOrder[100][32];
int orderIndex = 0;
Package* findPackage(char name[]);
//hash function
unsigned int hash(char *name)
{
	unsigned int value = 0;

	while(*name)
	{
		value = value * 31 + *name;
		name++;
	}

	return value % TABLE_SIZE;
}
//add package function
void addPackage(char name[], int major, int minor, int patch)
{
	if(findPackage(name) != NULL)
	{
		printf("Package already exists\n");
		return;
	}
	Package *newPkg;
	Package *temp;
	newPkg = (Package *)malloc(sizeof(Package));
	if(newPkg == NULL)
	{
		printf("Memory allocation failed\n");
		return;
	}
	strcpy(newPkg->name, name);
	newPkg->version.major = major;      //version
	newPkg->version.minor = minor;
	newPkg->version.patch = patch;
 
	newPkg->deps = NULL;        //Initialize Dependency List
 
	newPkg->visit = WHITE;      //visit status Initialize
 
	newPkg->next = NULL;
	newPkg->hashNext = NULL; 
	newPkg->status = PENDING;       //status check
 
	unsigned int index;
 
	index = hash(name);
 
	newPkg->hashNext = hashTable[index];
	hashTable[index] = newPkg;
	if(head == NULL)
	{
		head = newPkg;
		return;
	}
	temp = head;
	while(temp->next != NULL)
	{
		temp = temp->next;
	}
	temp->next = newPkg;
}

//update Package
void updatePackage(char name[],int major,int minor,int patch)
{
	Package *pkg;
	pkg = findPackage(name);
	if(pkg == NULL)
	{
		printf("Package not found\n");
		return;
	}
	pkg->version.major = major;
	pkg->version.minor = minor;
	pkg->version.patch = patch;
	printf("Package updated successfully\n");
}

//search Package
Package* findPackage(char name[])
{
    unsigned int index;
    Package *temp;
 
    index = hash(name);
 
    temp = hashTable[index];
 
    while(temp != NULL)
    {
        if(strcmp(temp->name, name) == 0)
        {
            return temp;
        }
 
        temp = temp->hashNext;
    }
 
    return NULL;
}


//find dependency function
//search dependency
Dependency *findDependency(Package *pkg, char depName[])
{
	Dependency *temp = pkg->deps;

	while(temp != NULL)
	{
		if(strcmp(temp->name, depName) == 0)
		{
			return temp;
		}

		temp = temp->next;
	}

	return NULL;
}

//version comparison
int compareVersion(Version v1, Version v2)
{
	if(v1.major > v2.major)
		return 1;

	if(v1.major < v2.major)
		return -1;

	if(v1.minor > v2.minor)
		return 1;

	if(v1.minor < v2.minor)
		return -1;

	if(v1.patch > v2.patch)
		return 1;

	if(v1.patch < v2.patch)
		return -1;

	return 0;
}

//revisitis function
void resetVisits()
{
	Package *temp = head;

	while(temp != NULL)
	{
		temp->visit = WHITE;
		temp = temp->next;
	}
}
//DFS Function for cycle Dependency
int detectCycleDFS(Package *pkg)
{
	Dependency *dep;
	Package *nextPkg;

	pkg->visit = GRAY;

	dep = pkg->deps;

	while(dep != NULL)
	{
		nextPkg = findPackage(dep->name);

		if(nextPkg != NULL)
		{
			if(nextPkg->visit == GRAY)
			{
				return 1;
			}

			if(nextPkg->visit == WHITE)
			{
				if(detectCycleDFS(nextPkg))
				{
					return 1;
				}
			}
		}

		dep = dep->next;
	}

	pkg->visit = BLACK;

	return 0;
}
//For topological sorting - installation order
void topoDFS(Package *pkg)
{
	Dependency *dep;
	Package *nextPkg;
 
	pkg->visit = GRAY;
 
	dep = pkg->deps;
 
	while(dep != NULL)
	{
		nextPkg = findPackage(dep->name);
 
		if(nextPkg != NULL &&
		        nextPkg->visit == WHITE)
		{
			topoDFS(nextPkg);
		}
 
		dep = dep->next;
	}
 
	pkg->visit = BLACK;
	pkg->status = INSTALLED;
 
	if(orderIndex < 100)
	{
    	strcpy(installOrder[orderIndex],pkg->name);
    	orderIndex++;
	}
}
//intallation order cycle
void generateInstallOrder()
{
	Package *temp;

	/* Check Cycle First */

	resetVisits();

	temp = head;

	while(temp != NULL)
	{
		if(temp->visit == WHITE)
		{
			if(detectCycleDFS(temp))
			{
				printf("\nCYCLE DETECTED\n");
				printf("Installation Order Cannot Be Generated\n");

				return;
			}
		}

		temp = temp->next;
	}

	/* Generate Order */

	resetVisits();

	orderIndex = 0;

	temp = head;

	while(temp != NULL)
	{
		if(temp->visit == WHITE)
		{
			topoDFS(temp);
		}

		temp = temp->next;
	}

	printf("\nInstallation Order:\n");

	for(int i = 0; i < orderIndex; i++)
	{
		printf("%s\n", installOrder[i]);
	}
}
//main cycle checker
void detectCycles()
{
	Package *temp;

	resetVisits();

	temp = head;

	while(temp != NULL)
	{
		if(temp->visit == WHITE)
		{
			if(detectCycleDFS(temp))
			{
				printf("\nCIRCULAR DEPENDENCY DETECTED\n");
				return;
			}
		}

		temp = temp->next;
	}

	printf("\nNo Cycles Found\n");
}

//version conflict checker
void checkVersionConflicts()
{
	Package *pkg;
	Dependency *dep;
	Package *requiredPkg;

	pkg = head;

	while(pkg != NULL)
	{
		dep = pkg->deps;

		while(dep != NULL)
		{
			requiredPkg = findPackage(dep->name);

			if(requiredPkg != NULL)
			{
				if(compareVersion(requiredPkg->version,
				                  dep->minVersion) < 0)
				{
					pkg->status = CONFLICTING;
					printf("\nVERSION CONFLICT\n");
					printf("%s requires %s >= %d.%d.%d\n",pkg->name,dep->name,
					       dep->minVersion.major,
					       dep->minVersion.minor,
					       dep->minVersion.patch);
					printf("Current Version : %d.%d.%d\n",
					       requiredPkg->version.major,
					       requiredPkg->version.minor,
					       requiredPkg->version.patch);
				}
			}
			dep = dep->next;
		}

		pkg = pkg->next;
	}
}

//list all Package
void listPackages()
{
	Package *temp = head;
	if(temp == NULL)
	{
		printf("No packages found\n");
		return;
	}
	while(temp != NULL)
	{
		printf("Name: %s Version: %d.%d.%d\n",temp->name,temp->version.major,temp->version.minor,temp->version.patch);
		temp = temp->next;
	}
}
//delete packages
void deletePackage(char name[])
{
    Package *current;
    Package *previous;
 
    Package *hashCurrent;
    Package *hashPrevious;
 
    Dependency *dep;
 
    unsigned int index;
 
    current = head;
    previous = NULL;
 
    while(current != NULL &&
          strcmp(current->name, name) != 0)
    {
        previous = current;
        current = current->next;
    }
 
    if(current == NULL)
    {
        printf("Package not found\n");
        return;
    }
 
    /* Remove from linked list */
 
    if(previous == NULL)
    {
        head = current->next;
    }
    else
    {
        previous->next = current->next;
    }
 
    /* Remove from hash table */
 
    index = hash(name);
 
    hashCurrent = hashTable[index];
    hashPrevious = NULL;
 
    while(hashCurrent != NULL)
    {
        if(strcmp(hashCurrent->name, name) == 0)
        {
            break;
        }
 
        hashPrevious = hashCurrent;
        hashCurrent = hashCurrent->hashNext;
    }
 
    if(hashCurrent != NULL)
    {
        if(hashPrevious == NULL)
        {
            hashTable[index] = hashCurrent->hashNext;
        }
        else
        {
            hashPrevious->hashNext =
                hashCurrent->hashNext;
        }
    }
 
    /* Free dependency list */
 
    while(current->deps != NULL)
    {
        dep = current->deps;
        current->deps = current->deps->next;
        free(dep);
    }
 
    free(current);
 
    printf("Package deleted successfully\n");
}
//add dependency function
void addDependency(char packageName[], char dependencyName[], int  major, int  minor, int patch)
{
	Package *pkg;
	Dependency *newDep;
	Dependency *temp;

	pkg = findPackage(packageName);

	if(pkg == NULL)
	{
		printf("Package not found\n");
		return;
	}

	//updated add Dependency
	/* Check dependency package exists */
	if(findPackage(dependencyName) == NULL)
	{
		pkg->status = MISSING;
		printf("Dependency package does not exist\n");
		return;
	}

	/* Check self dependency */
	if(strcmp(packageName, dependencyName) == 0)
	{
		printf("Self dependency not allowed\n");
		return;
	}

	/* Check duplicate dependency */
	if(findDependency(pkg, dependencyName) != NULL)
	{
		printf("Dependency already exists\n");
		return;
	}

	newDep = (Dependency *)malloc(sizeof(Dependency));

	if(newDep == NULL)
	{
		printf("Memory allocation failed\n");
		return;
	}

	strcpy(newDep->name, dependencyName);

	newDep->minVersion.major = major;
	newDep->minVersion.minor = minor;
	newDep->minVersion.patch = patch;

	newDep->next = NULL;

	if(pkg->deps == NULL)
	{
		pkg->deps = newDep;
	}
	else
	{
		temp = pkg->deps;

		while(temp->next != NULL)
		{
			temp = temp->next;
		}

		temp->next = newDep;
	}

	printf("Dependency added successfully\n");
}

//display dependencies
void showDependencies(char packageName[])
{
	Package *pkg;
	Dependency *temp;

	pkg = findPackage(packageName);

	if(pkg == NULL)
	{
		printf("Package not found\n");
		return;
	}

	printf("Dependencies of %s:\n", packageName);

	temp = pkg->deps;

	if(temp == NULL)
	{
		printf("No dependencies\n");
		return;
	}

	while(temp != NULL)
	{
		printf("%s >= %d.%d.%d\n",temp->name,temp->minVersion.major,temp->minVersion.minor,temp->minVersion.patch);
		temp = temp->next;
	}
}

//remove dependency function
void removeDependency(char packageName[], char dependencyName[])
{
	Package *pkg;
	Dependency *current;
	Dependency *previous;

	pkg = findPackage(packageName);

	if(pkg == NULL)
	{
		printf("Package not found\n");
		return;
	}

	current = pkg->deps;
	previous = NULL;

	while(current != NULL &&
	        strcmp(current->name, dependencyName) != 0)
	{
		previous = current;
		current = current->next;
	}

	if(current == NULL)
	{
		printf("Dependency not found\n");
		return;
	}

	if(previous == NULL)
	{
		pkg->deps = current->next;
	}
	else
	{
		previous->next = current->next;
	}

	free(current);

	printf("Dependency removed successfully\n");
}

//updated free function free all memory
void freePackages()
{
    Package *tempPkg;
    Dependency *tempDep;
 
    while(head != NULL)
    {
        tempPkg = head;
 
        while(tempPkg->deps != NULL)
        {
            tempDep = tempPkg->deps;
            tempPkg->deps = tempPkg->deps->next;
            free(tempDep);
        }
 
        head = head->next;
        free(tempPkg);
    }
 
    for(int i = 0; i < TABLE_SIZE; i++)
    {
        hashTable[i] = NULL;
    }
}
//savetofile
void saveToFile()
{
	FILE *fp;
	Package *pkg;
	Dependency *dep;

	fp = fopen("packages.txt", "w");

	if(fp == NULL)
	{
		printf("File cannot be opened\n");
		return;
	}

	pkg = head;

	while(pkg != NULL)
	{
		fprintf(fp,
		        "P %s %d %d %d\n",
		        pkg->name,
		        pkg->version.major,
		        pkg->version.minor,
		        pkg->version.patch);

		dep = pkg->deps;

		while(dep != NULL)
		{
			fprintf(fp,
			        "D %s %s %d %d %d\n",
			        pkg->name,
			        dep->name,
			        dep->minVersion.major,
			        dep->minVersion.minor,
			        dep->minVersion.patch);

			dep = dep->next;
		}

		pkg = pkg->next;
	}

	fclose(fp);

	printf("Data saved successfully\n");
}
//loadfrom File
void loadFromFile()
{
	FILE *fp;
	char type;

	fp = fopen("packages.txt", "r");

	if(fp == NULL)
	{
		printf("File not found\n");
		return;
	}

	while(fscanf(fp, " %c", &type) == 1)
	{
		if(type == 'P')
		{
			char name[32];
			int maj;
			int min;
			int pat;

			fscanf(fp,
			       "%s %d %d %d",
			       name,
			       &maj,
			       &min,
			       &pat);

			addPackage(name,
			           maj,
			           min,
			           pat);
		}
		else if(type == 'D')
		{
			char pkg[32];
			char dep[32];

			int maj;
			int min;
			int pat;

			fscanf(fp,
			       "%s %s %d %d %d",
			       pkg,
			       dep,
			       &maj,
			       &min,
			       &pat);

			addDependency(pkg,
			              dep,
			              maj,
			              min,
			              pat);
		}
	}

	fclose(fp);

	printf("Data loaded successfully\n");
}
//Dependencystatus
void showPackageStatus()
{
	Package *temp = head;

	while(temp != NULL)
	{
		printf("%s : ", temp->name);

		switch(temp->status)
		{
		case PENDING:
			printf("PENDING");
			break;

		case INSTALLED:
			printf("INSTALLED");
			break;

		case MISSING:
			printf("MISSING");
			break;

		case CONFLICTING:
			printf("CONFLICTING");
			break;
		}

		printf("\n");

		temp = temp->next;
	}
}