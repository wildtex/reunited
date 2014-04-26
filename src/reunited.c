/**
 * @version $Id$
 * @copyright 2014 wildtex @ github
 */
#include <stdio.h>     /* stdin, printf, and fgets */
#include <string.h>    /* string functions */
#include <stdlib.h>    /* malloc */
#include <time.h>      /* nanosleep */
#include <sys/stat.h>  /* stat */
#include <unistd.h>    /* getuid */
#include <pwd.h>       /* getpwnam */
#include <signal.h>    /* signal capture */
#include "config.h"    /* config file tools */

#define CONFIG_FILE "/etc/reunited.conf"

/**
 * @todo  ####  IMPROVEMENTS NEEDED ASAP   ####
 *
 * 1. recursive copying for files
 * 2. configurable external conf file
 *    a. build config file - COMPLETE
 *    b. build parsing library - COMPLETE
 *    c. cut over to parsing library - COMPLETE
 * 3. Implement sub-pattern ignoring feature
 * 4. hashing and logging of changes for package building
 * 5. add daemonization and /var/log/ output
 * 6. line arguments and help info
 * 7. replace change source file with inotify
 * 8. merge branchpush and do full sync on startup
 * 9. create init.d service start/stop scripts
 */


/**
 * Get last modified timestamp of a file
 * @param  path full path of file to stat
 * @return      modified unixtime
 */
int getModifiedUnixtime(const char *path) {
	struct stat attr;
	stat(path, &attr);
	return attr.st_mtime;
}

/**
 * Perform file copy
 * @param  sourcePath source file path
 * @param  destPath   destination file path
 * @return
 */
int copyFile(const char *sourcePath, const char *destPath)
{
	FILE  *sourceFile, *destFile;
	int a;

	if (!(sourceFile = fopen(sourcePath, "rb"))) {
		return -1;
	}

	if (!(destFile = fopen(destPath, "wb"))) {
		fclose(sourceFile);
		return -1;
	}

	while(1) {
		a  =  fgetc(sourceFile);
		if(!feof(sourceFile)) {
			fputc(a, destFile);
		}
		else {
			break;
		}
	}

	fclose(destFile);
	fclose(sourceFile);

	// change ownership to web user
 	struct passwd *webUser = getpwnam("www-data");
	chown(destPath, webUser->pw_uid, webUser->pw_gid);

	return  0;
}

/**
 * Replace remote root path with local root path
 * 
 * @param  rawSource full remote root file and path
 * @return string         localized root file and path
 */
char *getSourceAsLocal(const char *rawSource)
{
	char *sourcePath = 0;
	char *stripped;

	// @todo replace these as configurable or autodetecing properties
	char remoteWorkspace[21] = "/Volumes/vmWorkspace";
	char localWorkspace[25] = "/home/jsenator/Workspace";
	
	stripped = (char *)calloc((strlen(rawSource) - strlen(remoteWorkspace)) + 1, sizeof(char));
	sourcePath  = (char *)calloc((strlen(rawSource) + strlen(localWorkspace) + 1) - strlen(remoteWorkspace), sizeof(char));

	if (strstr(rawSource, remoteWorkspace) != NULL) {
		memmove(stripped, rawSource + strlen(remoteWorkspace), (strlen(rawSource) - strlen(remoteWorkspace)));
		strcpy(sourcePath, localWorkspace);
		strcat(sourcePath, stripped);
		sourcePath[strlen(stripped) + strlen(localWorkspace)] = '\0';
	}
	else {
		strcpy(sourcePath, rawSource);
		sourcePath[strlen(rawSource)] = '\0';
	}

	free(stripped);

	return sourcePath;
}

/**
 * Loop through log rows and perform processing actions
 * @param logContent    logs content read in from activity log
 * @param configuration reunited config structure
 */
void processLogItems(char *logContent, const struct reunitedConfig configuration)
{
	char *logToken = 0;
	int i = 0;
	int foundPath = 0;
	char *destPath = 0;

	logToken = (char *)malloc(strlen(logContent) + 1);
   
	for (logToken = strtok(logContent, "\n"); logToken != NULL; logToken = strtok(NULL, "\n")) {
		char *sourcePath = getSourceAsLocal(logToken);

		i = 0;
		foundPath = 0;
		for (i = 0; i < configuration.syncTargetCount; i++) {
			destPath = strstr(sourcePath, configuration.syncTargets[i].directory);

			if (destPath != NULL) {
				foundPath++;

				printf("Copying \"%s\" to \"%s\"\n", sourcePath, destPath);
				copyFile(sourcePath, destPath);

				break;
			}
		}

		if (!foundPath) {
			printf("\"%s\" is not eligible for autocopy!\n", sourcePath);
		}

		free(sourcePath);
	}
}

void sigHandler(int signo)
{
  if (signo == SIGINT)
    printf("Exiting reunited.\n");
	exit(0);
}

int main()
{
	if (geteuid() != 0) {
		printf("This utility must be run as root.\n");
		return -1;
	}

	// fetch configuration
	struct reunitedConfig configuration;
	configuration.syncTargetCount = buildConfig(CONFIG_FILE, &configuration);

	signal(SIGINT, sigHandler);

	const char *log = configuration.logfile;

	printf("Monitoring file: %s \n", log);

	long lastModified = (long)malloc(sizeof(long));
	long lastChecked = (long)malloc(sizeof(long));

	struct timespec t1;
	t1.tv_sec = 0;
	t1.tv_nsec = 500000000L;
	// or pass in like: 
	// <method>((struct timespec[]){{0, 500000000}});

	int done = 0;

	while (!done) {

		//getFileCreationTime(log);
		lastChecked = getModifiedUnixtime(log);

		if (lastChecked > lastModified) {
			lastModified = lastChecked;
			// open file
			FILE *autolog;
			if (!(autolog = fopen(log, "r+"))) {
				printf("Unable to open source log\n");
				return 0;
			}

			// go to end of file
			if((fseek(autolog, 0, SEEK_END))) {
				continue;
			}

			// capture file size
			size_t fsize = ftell(autolog);
			
			// reset cursor
			fseek(autolog,0,SEEK_SET);
			
			char *logContent = (char *)malloc(fsize + 1);

			if (fsize > 0) {
				// read file into buffer
				fread(logContent, fsize, 1, autolog);

				// terminate string properly
				logContent[fsize] = '\0';

				// reopen log and reset it
				if (autolog) {
					fclose(autolog);
				}
				autolog = fopen(log, "w");

				// print stats
				printf("Found content: file size: %d last modified: %d\n", (int)fsize, (int)lastModified);

				// process push actions for valid line items
				processLogItems(logContent, configuration);
				
			}

			// close file
			if (autolog) {
				fclose(autolog);
			}

			// flush buffer
			fflush(stdout);

			if (logContent) {
				free(logContent);
			}

		}

		// sleep for half a second
		nanosleep(&t1, NULL);
	}

	return 0;
}