# Linux Shell
A functional linux shell written in C

## Instructions
1 - Download files

2 - Run with `make debug`

3 - Exit with `exit` or `ctrl+Z`

## Pre-defined Commands
`cd <PATH>` - change working directory

`!history` - list the command history of the current shell session

`#<n>` - re-runs <i>n</i>-th command in the history

## Logical Operators:

**Will not work with `!history`, `#<n>`, or `!<PREFIX>`**

`<CMD> || <CMD>` - runs the first command OR the second command. The second command will only be run if the first command fails.

`<CMD> && <CMD>` - runs the first command AND the second command. If the first command fails, the second command will not be run.

`<CMD>; <CMD>` - runs the first command, then the second command. The second command will still be run if the first command fails.

## IMPORTANT - Fork Bombs
This application utilizes fork(). To avoid accidentally fork-bombing you system, make sure you set `ulimit`. 