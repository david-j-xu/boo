# Boo

## Overview
A minimal version of Git (only supporting single branches so far) implemented in C++ for the lolzies. It supports minimal versions of init, commit, log, reset, and status commands. Changes are automatically staged for now.

Each commit is placed on the end of the current commit log, and you can reset to previous commits by using the `reset` command. Note that `reset` does not erase any commits, and commits from a point earlier in the branch will still commit to the end of the commit log.

## Usage
Currently, the usage is as follows:
`boo [-h help] [-v verbose] command [command arguments]`

Boo will search for the first repository that exists in the path from the working directory to root, and will operate on that.

The supported arguments are:
- `init`: Creates a repository in the current directory. I'm actually pretty sure this supports nested inits as well, though I haven't really checked this.

- `commit`: Commits the current state of the repository to the end of the commit log, and moves the `HEAD` to this new commit. The default message is "No message provided", but this can be changed using the `-m` argument.

- `status`: Outputs the current changes at file granularity to the terminal.

- `reset`: A catchall for navigating between commits. Requires a `-c` argument to specify which commit to jump to. Also displays the changes at file granularity.

- `log`: Outputs the commit log including commit hashes, messages, and where the current head is


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

Lastly, I have a file called `head` containing the current head commit