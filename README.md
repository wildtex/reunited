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


