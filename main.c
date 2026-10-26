#include <stdio.h>
#include "package_manager.h"
//main function
int main()
{
	int choice;
	//hash table Initialization
	for(int i = 0; i < TABLE_SIZE; i++)
	{
		hashTable[i] = NULL;
	}

	char name[32];
	int major;
	int minor;
	int patch;
	Package *p;
	do
	{
		printf("\n===== PACKAGE MANAGER =====\n");
		printf("1. Add Package\n");
		printf("2. Search Package\n");
		printf("3. Delete Package\n");
		printf("4. List Packages\n");
		printf("5. Update Package\n");

		//updated menu function-dependency
		printf("6. Add Dependency\n");
		printf("7. Remove Dependency\n");
		printf("8. Show Dependencies\n");

		//version conflict function
		printf("9. Check Version Conflicts\n");

		//for cycle detection
		printf("10. Detect Cycles\n");

		//installOrder
		printf("11. Generate Installation Order\n");

		//file options
		printf("12. Save Data\n");
		printf("13. Load Data\n");

		//package status
		printf("14. Show Package Status\n");

		printf("0. Exit\n");
		printf("Enter choice: ");
		scanf("%d", &choice);
		switch(choice)
		{
		case 1:
			printf("Enter package name: ");
			scanf("%31s", name);
			printf("Enter version: ");
			scanf("%d %d %d", &major,&minor,&patch);
			addPackage(name, major,minor,patch);
			printf("Package added successfully.\n");
			break;
		case 2:
			printf("Enter package name to search: ");
			scanf("%31s", name);
			p = findPackage(name);
			if(p != NULL)
			{
				printf("Package Found!\n");
				printf("Name : %s\n", p->name);
				printf("Version : %d.%d.%d\n",p->version.major,p->version.minor,p->version.patch);
			}
			else
			{
				printf("Package not found.\n");
			}
			break;
		case 3:
			printf("Enter package name to delete: ");
			scanf("%31s", name);
			deletePackage(name);
			printf("Delete operation completed.\n");
			break;
		case 4:
			printf("\nPackage List:\n");
			listPackages();
			break;
		case 5:
		{
			char pkgName[32];
			int major;
			int minor;
			int patch;
			printf("Package Name: ");
			scanf("%31s", pkgName);
			printf("New Version (major minor patch): ");
			scanf("%d %d %d",&major,&minor,&patch);
			updatePackage(pkgName,major,minor,patch);
			break;
		}
		
		//for Dependency
		case 6:
		{
			char pkg[32];
			char dep[32];

			int major;
			int minor;
			int patch;

			printf("Package Name: ");
			scanf("%31s", pkg);
			printf("Dependency Name: ");
			scanf("%31s", dep);

			printf("Minimum Version (major minor patch): ");
			scanf("%d %d %d",&major, &minor, &patch);

			addDependency(pkg, dep, major, minor, patch);
			break;
		}
		case 7:
		{
			char pkg[32];
			char dep[32];
			printf("Package Name: ");
			scanf("%31s", pkg);
			printf("Dependency Name: ");
			scanf("%31s", dep);
			removeDependency(pkg, dep);
			break;
		}
		case 8:
		{
			char pkg[32];
			printf("Package Name: ");
			scanf("%31s", pkg);
			showDependencies(pkg);
			break;
		}
		case 9:
		{
			checkVersionConflicts();
			break;
		}
		case 10:
		{
			detectCycles();
			break;
		}
		case 11:
		{
			generateInstallOrder();
			break;
		}
		case 12:
		{
			saveToFile();
			break;
		}
		case 13:
		{
			loadFromFile();
			break;
		}
		case 14:
		{
			showPackageStatus();
			break;
		}
		case 0:
			printf("Exiting program...\n");
			break;
		default:
			printf("Invalid choice!\n");
		}
	} while(choice != 0);
	freePackages();
	return 0;
}