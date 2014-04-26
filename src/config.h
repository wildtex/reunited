/**
 * @version $Id$
 * @copyright 2014 wildtex @ github
 */
#ifndef REUNITED_CONFIG
#define REUNITED_CONFIG

#include <stdio.h>    /* stdin, printf, and fgets */
#include <string.h>   /* string functions */
#include <stdlib.h>   /* malloc */

struct ignoreDir {
	char *directory;
};

struct syncTarget {
	char *directory;
	char *owner;
	char *group;
	char *permissions;
	int ignoreHidden;
	struct ignoreDir *ignoreDirs;
	int ignoreDirCount;
};

struct reunitedConfig {
	char *workspace;
	char *project;
	char *logfile;
	int loggingEnabled;
	int syncTargetCount;
	struct syncTarget *syncTargets;
};

char *cleanConfigValue(const char *configValue)
{

	char *cleanValue, *temp;
	unsigned long i;

	cleanValue = (char *)calloc(strlen(configValue), sizeof(char *));
	strcpy(cleanValue, configValue);

	if (strlen(configValue)) {
		// clean leading spaces
		for (i = 0; i < strlen(cleanValue); i++) {
			if (strncmp(" ", &cleanValue[i], 1) != 0) {
				temp = (char *)calloc(((strlen(cleanValue)) - i), sizeof(char *));
				memmove(temp, cleanValue + (i), (strlen(cleanValue)) - i);
				free(cleanValue);
				cleanValue = (char *)calloc((strlen(temp)), sizeof(char *));
				strcpy(cleanValue, temp);
				free(temp);
				break;
			}
		}

		// clean trailing spaces and new line
		for (i = strlen(cleanValue) - 1; i > 0 ; i--) {
			if (strncmp(" ", &cleanValue[i], 1) != 0
				&& strncmp("\n", &cleanValue[i], 1) != 0) 
			{
				temp = (char *)calloc(i + 1, sizeof(char *));
				memcpy(temp, cleanValue, i + 1);
				free(cleanValue);
				cleanValue = (char *)calloc(i + 1, sizeof(char *));
				strcpy(cleanValue, temp);
				free(temp);
				break;
			}
		}
	}

	return cleanValue;
}

char *getConfigValue(const char *configLine)
{
	char *configValue, *cleanValue;
	unsigned long i;

	// separate value from config line
	for (i = 0; i < strlen(configLine); i++) {
		if(strncmp("=", &configLine[i], 1) == 0) {
			configValue = (char *)calloc(((strlen(configLine)) - i) + 1, sizeof(char *));
			memmove(configValue, configLine + (i + 1), (strlen(configLine)) - i);
		}
	}

	cleanValue = cleanConfigValue(configValue);
	free(configValue);
	return cleanValue;
}

int buildConfig(const char *configFileName, struct reunitedConfig *configuration) 
{
	// open config file
	FILE  *configFile;
	char *configLine;
	const int lineSize = 256;
	int targetCount = 0;

	if (!(configFile = fopen(configFileName, "r"))) {
		return -1;
	}

	printf("reading: %s\n", configFileName);

	// first pass - fill globals and count targets
	configLine = (char *)malloc(lineSize * sizeof(char *));
	while (fgets(configLine, lineSize, configFile) != NULL ) {

		// incremement target count 
		if (strncmp("[", configLine, 1) == 0) {
			targetCount++;
		}

		// look for logfile
		if (strncmp("logfile", configLine, 7) == 0) {
			configuration->logfile = getConfigValue(configLine);
		}

		// look for logging-enabled
		if (strncmp("logging-enabled", configLine, 15) == 0) {
			char *loggingEnabled;
			loggingEnabled = getConfigValue(configLine);
			if (strcmp("true", loggingEnabled) == 0) {
				configuration->loggingEnabled = 1;	
			}
			else {
				configuration->loggingEnabled = 0;
			}
			free(loggingEnabled);
		}

		// look for workspace
		if (strncmp("workspace", configLine, 9) == 0) {
			configuration->workspace = getConfigValue(configLine);
		}

		// look for project
		if (strncmp("project", configLine, 7) == 0) {
			configuration->project = getConfigValue(configLine);
		}		
	}

	configuration->syncTargetCount = targetCount;

	printf("Log File: %s\n", configuration->logfile);
	printf("Workspace: %s\n", configuration->workspace);
	printf("Project: %s\n", configuration->project);
	printf("Target Count: %d\n", targetCount);

	rewind(configFile);

	configuration->syncTargets = (struct syncTarget*)calloc(targetCount, sizeof(struct syncTarget));
	int currentTarget;
	currentTarget = -1;
	while (fgets(configLine, lineSize, configFile) != NULL ) {
		// extract target name
		if (strncmp("[", configLine, 1) == 0) {
			// increment the current target
			currentTarget++;

			// parse out target
			int i;
			for (i = 1; i < strlen(configLine); i++) {
				if (strncmp("]", &configLine[i], 1) == 0) {
					char *rawDirectory;
					rawDirectory = (char *)calloc(i, sizeof(char *));
					memcpy(rawDirectory, configLine + 1, i - 1);
					configuration->syncTargets[currentTarget].directory = cleanConfigValue(rawDirectory);
					free(rawDirectory);
					break;
				}
			}
		}

		// extract target properties
		if (currentTarget > -1) {

			// look for owner
			if (strncmp("owner", configLine, 5) == 0) {
				configuration->syncTargets[currentTarget].owner = getConfigValue(configLine);
			}	

			// look for group
			if (strncmp("group", configLine, 5) == 0) {
				configuration->syncTargets[currentTarget].group = getConfigValue(configLine);
			}

			// look for permissions
			if (strncmp("permissions", configLine, 11) == 0) {
				configuration->syncTargets[currentTarget].permissions = getConfigValue(configLine);
			}

			// look for ignoreHidden
			if (strncmp("ignore-hidden", configLine, 12) == 0) {
				char *ignoreHidden;
				ignoreHidden = getConfigValue(configLine);
				if (strcmp("true", ignoreHidden) == 0) {
					configuration->syncTargets[currentTarget].ignoreHidden = 1;	
				}
				else {
					configuration->syncTargets[currentTarget].ignoreHidden = 0;
				}
				free(ignoreHidden);
			}

			// look for ignore-directories
			if (strncmp("ignore-directories", configLine, 18) == 0) {
				char *ignoreDirs;
				int i;
				configuration->syncTargets[currentTarget].ignoreDirCount = 1;
				ignoreDirs = getConfigValue(configLine);
				if (strlen(ignoreDirs)) {
					for(i=0 ; ignoreDirs[i]!='\0' ; i++) {
						if(strncmp(",", &ignoreDirs[i], 1) == 0) {
							configuration->syncTargets[currentTarget].ignoreDirCount++;
						}
					}
					configuration->syncTargets[currentTarget].ignoreDirs 
						= (struct ignoreDir*)calloc(configuration->syncTargets[currentTarget].ignoreDirCount, sizeof(struct ignoreDir));
					int j = 0;
					if (configuration->syncTargets[currentTarget].ignoreDirCount > 1) {

						char *ignoreDir;
						int start = 0;
						for(i=0 ; i <= strlen(ignoreDirs) ; i++) {
							if(strncmp(",", &ignoreDirs[i], 1) == 0 || i == strlen(ignoreDirs)) {
								ignoreDir = (char *)calloc(i - start, sizeof(char *));
								memcpy(ignoreDir, ignoreDirs + start, (i - start));
								configuration->syncTargets[currentTarget].ignoreDirs[j].directory 
									= cleanConfigValue(ignoreDir);
								free(ignoreDir);
								start = i + 1;
								j++;
							}
						}

					}
					else {

						configuration->syncTargets[currentTarget].ignoreDirs[j].directory 
							= (char *)calloc(strlen(ignoreDirs), sizeof(char *));
						strcpy(configuration->syncTargets[currentTarget].ignoreDirs[j].directory, ignoreDirs);

					}
				}
				free(ignoreDirs);
			}			
		}		
	}

	fclose(configFile);

	free(configLine);

	return targetCount;
}

void freeConfig(struct reunitedConfig *configuration)
{
	int i, j;
	for (i = 0; i < configuration->syncTargetCount; i++) {
		free(configuration->syncTargets[i].directory);
		free(configuration->syncTargets[i].owner);
		free(configuration->syncTargets[i].group);
		free(configuration->syncTargets[i].permissions);
		for(j = 0; j < configuration->syncTargets[i].ignoreDirCount; j++) {
			free(configuration->syncTargets[i].ignoreDirs[j].directory);
		}
		free(configuration->syncTargets[i].ignoreDirs);
	}

	free(configuration->syncTargets);
	free(configuration->logfile);
	free(configuration->workspace);
	free(configuration->project);
}

#endif