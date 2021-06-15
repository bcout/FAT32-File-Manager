# FAT32-File-Reader

This program allows a user to perform read operations on a FAT32 formatted disk image. As of now, the makefile's `run` command uses the disk image in the
data directory called `diskimage`. So, to change what disk image is used you could either change the variable value in the makefile, or replace `diskimage` with your own, keeping the same name.

# Documentation

This program was made by following the FAT32 whitepaper, available here: https://www.cs.virginia.edu/~cr4bd/4414/F2018/files/fatspec.pdf

# Usage

```
$ make
$ make run
> info : Prints information about the disk image
> dir : Prints all the files and directories contained within the current directory
> cd <new directory> : Changes the current directory to the new directory
> get <filename> : Downloads the specified file into ./files/<filename>
> exit : Exits the program cleanly
```
