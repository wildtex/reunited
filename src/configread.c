#include <stdio.h>          /* stdin, printf, and fgets */
#include <string.h>         /* string functions */
#include <stdlib.h>         /* malloc */
#include <sys/stat.h>       /* stat */
#include "../include/config.h" /* config file tools */

int main()
{

	char configFile[49] = "/home/jsenator/Workspace/reunited/reunited.conf";
	struct reunitedConfig configuration;

	// fetch configuration
	configuration.syncTargetCount = buildConfig(configFile, &configuration);

	int i, j;
	for (i = 0; i < configuration.syncTargetCount; i++) {
		printf("directory: %s  owner: %s\n", 
			configuration.syncTargets[i].directory, 
			configuration.syncTargets[i].owner);
		for (j = 0; j < configuration.syncTargets[i].ignoreDirCount; j++) {
			printf("   Ignore subdirectory: %s\n", configuration.syncTargets[i].ignoreDirs[j].directory);
		}
	}

	// free config memory
	freeConfig(&configuration);

	return 0;
}	