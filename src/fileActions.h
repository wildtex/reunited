/**
 * @version $Id$
 * @copyright 2014 wildtex @ github
 */
#ifndef REUNITED_FILE_ACTIONS
#define REUNITED_FILE_ACTIONS
#include <stdio.h>     /* stdin, printf, and fgets */
#include <string.h>    /* string functions */
#include <stdlib.h>    /* malloc */
#include <sys/stat.h>  /* stat */
#include <unistd.h>    /* getuid, read, write */
#include <pwd.h>       /* getpwnam */
#include <signal.h>    /* signal capture */
#include <errno.h>     /* error defs */
#include <dirent.h>    /* dir tools */
#include <limits.h>    /* file sys size defs */
#include "config.h"    /* config file tools */

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
char *getSourceAsLocal(const char *rawSource, const struct reunitedConfig configuration)
{
	char *sourcePath = 0;
	char *stripped;

	// @todo replace these as configurable or autodetecing properties
	char remoteWorkspace[21] = "/Volumes/vmWorkspace";
	
	stripped = (char *)calloc(
		(strlen(rawSource) - strlen(remoteWorkspace)) + 1, sizeof(char));
	sourcePath  = (char *)calloc(
		(strlen(rawSource) + strlen(configuration.workspace) + 1) 
			- strlen(remoteWorkspace), sizeof(char));

	if (strstr(rawSource, remoteWorkspace) != NULL) {
		memmove(stripped, rawSource + strlen(remoteWorkspace), 
			(strlen(rawSource) - strlen(remoteWorkspace)));
		strcpy(sourcePath, configuration.workspace);
		strcat(sourcePath, stripped);
		sourcePath[strlen(stripped) + strlen(configuration.workspace)] = '\0';
	}
	else {
		strcpy(sourcePath, rawSource);
		sourcePath[strlen(rawSource)] = '\0';
	}

	free(stripped);

	return sourcePath;
}

#endif