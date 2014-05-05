reunited
========

Daemon listens for file changes in multiple scattered package directories and pushes changes to a united staging area.

#### Brief Summary
With the help of the iNotify library, this daemon watches a project directory.  Changed files are matched against target directories specified in the configuration file.  If the file is eligble to be pushed to the staging area, the file path is split at the matching point and copied to the target root.

##### Example
Configuration:
Watching directory "../project" with a target path of "/var/www"

File changed:
../project/packages/my-package/var/www/index.php

Split performed at start of match and copies:
from ../project/packages/my-package/var/www/index.php
to /var/www/index.php

####  IMPROVEMENTS NEEDED BEFORE PRODUCTION ####
1. recursive copying for files
2. configurable external conf file
   * build config file - COMPLETE
   * build parsing library - COMPLETE
   * cut over to parsing library - COMPLETE
   * add advance options to config
3. Implement regex pattern ignoring from config options
4. hashing and logging of changes for package building
5. add daemonization and /var/log/ output
6. line arguments and help info
7. replace change source file with inotify
8. merge branchpush and do full sync on startup
9. create init.d service start/stop scripts

