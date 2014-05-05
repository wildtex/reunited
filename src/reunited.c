/**
 * @version $Id$
 * @copyright 2014 wildtex @ github
 */
#include <stdio.h>       /* stdin, printf, and fgets */
#include <string.h>      /* string functions */
#include <stdlib.h>      /* malloc */
#include <time.h>        /* nanosleep */
#include <sys/stat.h>    /* stat */
#include <unistd.h>      /* getuid, read, write */
#include <pwd.h>         /* getpwnam */
#include <signal.h>      /* signal capture */
#include <errno.h>       /* error defs */
#include <dirent.h>      /* dir tools */
#include <limits.h>      /* file sys size defs */
#include "config.h"      /* config file tools */
#include "fileActions.h" /* file manipulation */

#define CONFIG_FILE "/etc/reunited.conf"

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
		char *sourcePath = getSourceAsLocal(logToken, configuration);

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

	struct timespec sleepTime;
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = 500000000L;
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
			if (!(autolog = fopen(log, "r"))) {
				fprintf(stderr, "Unable to open source log '%s': %s\n",
					log, strerror(errno));
				exit (EXIT_FAILURE);
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

				// close log
				if (autolog) {
					fclose(autolog);
				}

				// empty the log
				autolog = fopen(log, "w");

				// free log for new entries
				if (autolog) {
					fclose(autolog);
				}

				// print stats
				printf("Found content: file size: %d last modified: %d\n", (int)fsize, (int)lastModified);

				// process push actions for valid line items
				processLogItems(logContent, configuration);
				
			}
			else {
				// close file since no content
				if (autolog) {
					fclose(autolog);
				}
			}

			// flush buffer
			fflush(stdout);

			if (logContent) {
				free(logContent);
			}

		}

		// sleep for half a second
		nanosleep(&sleepTime, NULL);
	}

	return 0;
}