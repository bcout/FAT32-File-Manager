/********************************************************************
    Module: FAT32_io.h
    Author: Brennan Couturier

    Functions for opening/closing files and printing information
********************************************************************/

#ifndef FAT32_IO_H
#define FAT32_IO_H

#pragma region File_Descriptor_Functions

/********************************************************************
Open the formatted FAT32 disk image for reading and writing, sets
	the global disk_image_fd varible to this new file descriptor
********************************************************************/
void open_disk_image(char* disk_image_path_in);

/********************************************************************
Closes the disk image file descriptor that was opened at the beginning
********************************************************************/
void close_disk_image();

#pragma endregion File_Descriptor_Functions

#pragma region Printing_Functions

/********************************************************************
Prints out some values contained in the boot sector to a file
	in the debug directory
********************************************************************/
void print_boot_sector_info();

/********************************************************************
Prints out information about the root directory, for debugging
********************************************************************/
void print_root_directory();

/********************************************************************
Prints out information about the current directory and all the files
    and subfolders in it
********************************************************************/
void print_current_directory();

#pragma endregion Printing_Functions

#endif