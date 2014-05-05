reunited
========

Listener daemon to propagate source files from multiple packages into their live directories to allow near real time testing.

####  IMPROVEMENTS NEEDED BEFORE PRODUCTION ####

1. recursive copying for files
2. configurable external conf file
   a. build config file - COMPLETE
   b. build parsing library - COMPLETE
   c. cut over to parsing library - COMPLETE
   d. add advance options to config
3. Implement regex pattern ignoring from config options
4. hashing and logging of changes for package building
5. add daemonization and /var/log/ output
6. line arguments and help info
7. replace change source file with inotify
8. merge branchpush and do full sync on startup
9. create init.d service start/stop scripts

