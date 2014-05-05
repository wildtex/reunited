/**
 * @version $Id$
 * @copyright 2014 wildtex @ github
 */
#ifndef REUNITED_WATCH_MAPPING
#define REUNITED_WATCH_MAPPING

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h> /* directory functions */
#include <limits.h> /* limits.h defines "PATH_MAX". */
#include <unistd.h> /* for read/write functions */
#include <sys/inotify.h> /* inotify library */

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN  ( 1024 * ( EVENT_SIZE + 16 ) )
#define IN_EVENTS_OF_INTEREST ( \
 	IN_MODIFY | IN_ATTRIB | IN_MOVED_FROM | IN_MOVED_TO | \
 	IN_DELETE | IN_CREATE | IN_DELETE_SELF | IN_MOVE_SELF)

struct watchedDir {
    int targetIndex;
    char *path;
};

struct watchedMap {
    int watchCount;
    struct watchedDir *watchedDirs;
};

extern struct watchedMap watched;

/**
 * Add directory to watch list
 * @param directoryName Name of directory to watch
 * @param targetIndex   Index of master target path
 */
void addWatch(const char * directoryName, const int targetIndex, const int iNotifyInstance)
{
	int watchedDirIndex;
	watchedDirIndex = 
		inotify_add_watch(
			iNotifyInstance, 
			directoryName, 
			IN_EVENTS_OF_INTEREST | IN_DONT_FOLLOW);

	int largestIndex;
	if (watchedDirIndex > watched.watchCount) {
		largestIndex = watchedDirIndex + 1;
	}
	else {
		largestIndex = watched.watchCount + 1;
	}

	// DEBUG ONLY
	// printf("watchedIndex = %d  watchedCount = %d largestIndex = %d dirname = %s\n", 
	// watchedDirIndex, watched.watchCount, largestIndex, directoryName);

    watched.watchedDirs = 
    	(struct watchedDir *)realloc(
    		watched.watchedDirs, 
    		largestIndex * sizeof(struct watchedDir));

    // initialize descriptor for now
    watched.watchedDirs[watchedDirIndex].targetIndex = targetIndex; 

    watched.watchedDirs[watchedDirIndex].path = 
    	(char *)calloc((strlen(directoryName) + 1), sizeof(char *));

    memcpy(
    	watched.watchedDirs[watchedDirIndex].path, 
    	directoryName, strlen(directoryName) + 1);  

    watched.watchCount = largestIndex;
}

/**
 * Recursively find all subdirectories
 * @param directoryName Directory to traverse
 * @param targetIndex   Index of master target path
 */
void mapDirectory (const char * directoryName, const int targetIndex, const int iNotifyInstance)
{
    addWatch(directoryName, targetIndex, iNotifyInstance);

    DIR * directory;

    directory = opendir (directoryName);

    if (! directory) {
        fprintf (stderr, "Cannot open directory '%s': %s\n",
                 directoryName, strerror (errno));
    }
    else {
	    while (1) {
	        struct dirent * entry;
	        const char * dName;

	        entry = readdir (directory);

	        if (! entry) {
	            break;
	        }

	        dName = entry->d_name;

	        if (entry->d_type & DT_DIR) {

	            if (strcmp (dName, "..") != 0
	            	&& strcmp (dName, ".") != 0
	            	&& strcmp (dName, ".svn") != 0 
	            	&& strcmp (dName, "DEBIAN") != 0) 
	            {
	                int pathLength;
	                char path[PATH_MAX];
	 
	                pathLength = snprintf (path, PATH_MAX,
                        "%s/%s", directoryName, dName);

	                if (pathLength >= PATH_MAX) {
	                    fprintf (stderr, "Path length has got too long.\n");
	                    exit (EXIT_FAILURE);
	                }
	                mapDirectory (path, targetIndex, iNotifyInstance);
	            }
	        }
	    }

	    if (closedir (directory)) {
			fprintf (stderr, "Could not close '%s': %s\n",
			directoryName, strerror (errno));
	    }

	}
}

/**
 * Initialize watched map
 */
void initializeWatchedMap() {
    watched.watchedDirs = 
    	(struct watchedDir *)malloc(sizeof(struct watchedDir));
    watched.watchCount = 0;
}

/**
 * Free memory from watched map
 */
void freeWatchedMap() {
    int i;
    for (i = 0; i < watched.watchCount; i++) {
        free(watched.watchedDirs[i].path);
    }
    free(watched.watchedDirs);
}

/**
 * Translate an event type back from event mask
 * @param  eventMask event mask value
 * @return
 */
char * getTypeFromMask(const int eventMask)
{
	char *eventType;
	eventType = (char *)malloc(9 * sizeof(char *));
	strcpy (eventType, "NO_MATCH");

	if (eventMask & IN_MODIFY) {
		eventType = (char *)realloc(eventType, 10 * sizeof(char *));
		strcpy(eventType, "IN_MODIFY");
	}
	else if (eventMask & IN_ATTRIB) {
		eventType = (char *)malloc(10 * sizeof(char *));
		strcpy(eventType, "IN_ATTRIB");
	}
	else if (eventMask & IN_MOVED_FROM) {
		eventType = (char *)malloc(14 * sizeof(char *));
		strcpy(eventType, "IN_MOVED_FROM");
	}
	else if (eventMask & IN_MOVED_TO) {
		eventType = (char *)malloc(12 * sizeof(char *));
		strcpy(eventType, "IN_MOVED_TO");
	}
	else if (eventMask & IN_DELETE) {
		eventType = (char *)malloc(10 * sizeof(char *));
		strcpy(eventType, "IN_DELETE");
	}
	else if (eventMask & IN_CREATE) {
		eventType = (char *)malloc(10 * sizeof(char *));
		strcpy(eventType, "IN_CREATE");
	}
	else if (eventMask & IN_DELETE_SELF) {
		eventType = (char *)malloc(15 * sizeof(char *));
		strcpy(eventType, "IN_DELETE_SELF");
	}
	else if (eventMask & IN_MOVE_SELF) {
		eventType = (char *)malloc(13 * sizeof(char *));
		strcpy(eventType, "IN_MOVE_SELF");
	}
	else if (eventMask & IN_ACCESS) {
		eventType = (char *)malloc(10 * sizeof(char *));
		strcpy(eventType, "IN_ACCESS");
	}
	else if (eventMask & IN_CLOSE_WRITE) {
		eventType = (char *)malloc(15 * sizeof(char *));
		strcpy(eventType, "IN_CLOSE_WRITE");
	}
	else if (eventMask & IN_CLOSE_NOWRITE) {
		eventType = (char *)malloc(17 * sizeof(char *));
		strcpy(eventType, "IN_CLOSE_NOWRITE");
	}
	else if (eventMask & IN_OPEN) {
		eventType = (char *)malloc(8 * sizeof(char *));
		strcpy(eventType, "IN_OPEN");
	}
	else if (eventMask & IN_UNMOUNT) {
		eventType = (char *)malloc(11 * sizeof(char *));
		strcpy(eventType, "IN_UNMOUNT");
	}
	else if (eventMask & IN_Q_OVERFLOW) {
		eventType = (char *)malloc(14 * sizeof(char *));
		strcpy(eventType, "IN_Q_OVERFLOW");
	}
	else if (eventMask & IN_IGNORED) {
		eventType = (char *)malloc(11 * sizeof(char *));
		strcpy(eventType, "IN_IGNORED");
	}

	return eventType;
}

#endif