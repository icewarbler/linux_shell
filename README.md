# Linux Shell
A functional linux shell written in C

## Instructions
1 - Download files

2 - Run with 'make debug'

3 - Exit with `exit` or `ctrl+Z`

## Pre-defined Commands
`cd` - change working directory

`!history` - list the command history of the current shell session

`#[INTEGER]` - re-runs command in the history

## IMPORTANT - Fork Bombs
This application utilizes fork(). To avoid accidentally fork-bombing you system, make sure you set `ulimit`. 