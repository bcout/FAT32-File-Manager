/********************************************************************
    Module: main.c
    Author: Brennan Couturier

    Program that starts the shell
********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "../include/FAT32_io.h"
#include "../include/FAT32_disk_management.h"
#include "../include/FAT32_structs_globals.h"
#include "../include/shell.h"

int main(int argc, char* argv[]){


    //Check if enough arguments were supplied
    if(argc < 2){
        fprintf(stderr, "Usage: \"%s <disk image file>\"\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    //open the disk image for reading and writing
    open_disk_image(argv[1]);

    //Read the important stuff
    read_boot_sector();
    read_FS_info();
    read_root_directory();

    //go into the shell loop
    run_shell();

    //Free memory and close files
    free(boot_sector);
    free(fs_info_sector);
    free(root_directory);

    close_disk_image();

    return EXIT_SUCCESS;
}