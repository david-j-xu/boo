# Boo

## Overview
A minimal version of Git (only supporting single branches so far) implemented in C++ for the lolzies. It supports minimal versions of init, commit, log, reset, and status commands. Changes are automatically staged for now.


## .boo Format
`.boo` is my analogous version of `.git`. It contains a folder per commit containing the commit information (named as the commit name), as well as a commit log in `log`. The format of the log is as follows:

For each commit, there is 
```
commit_hash
number_of_bytes_in_commit_message
commit_message
<CRLF>
```
Additionally, there is a file for each commit, containing for each file in the repository, named meta<COMMIT_NAME>
```
file_path
file_hash
<CRLF>
```

Lastly, I have a file called `head` containing the commit of head