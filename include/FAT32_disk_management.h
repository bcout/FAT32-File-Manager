/********************************************************************
    Module: FAT32_disk_management.h
    Author: Brennan Couturier

    Functions to read/write data on the disk
********************************************************************/

#ifndef FAT32_DM_H
#define FAT32_DM_H

#pragma region Read_Functions

/********************************************************************
Read sizeof(FAT32_BS) bytes from the provided file descriptor into
	an allocated FAT32_BS struct. Sets the global boot sector reference
	to point to this memory
********************************************************************/
void read_boot_sector();

/********************************************************************
Reads the FSInfo data into an allocated FAT32_FSInfo struct
********************************************************************/
void read_FS_info();

/********************************************************************
Reads the root directory into an allocated FAT32_Directory_Entry struct
********************************************************************/
void read_root_directory();

/********************************************************************
Searches the current directory for a file with the provided name
    If the file is found, write the clusterchain to a file in memory
********************************************************************/
void download_file(char* file_name);

#pragma endregion Read_Functions

#pragma region Set_Functions

/********************************************************************
Searches the current directory for the specified directory
    If it exists, get its cluster number and set the global variable
********************************************************************/
void change_directory(char* destination);

#pragma endregion Set_Functions

#endif