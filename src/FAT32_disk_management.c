/********************************************************************
    Module: FAT32_disk_management.c
    Author: Brennan Couturier

    Functions to read/write data on the disk
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/FAT32_structs_globals.h"
#include "../include/FAT32_disk_management.h"
#include "../include/FAT32_helpers.h"

#pragma region Read_Functions

/********************************************************************
Reads sizeof(FAT32_BS) bytes from the provided disk image file pointer
    and puts them into an allocated FAT32_BS struct.
********************************************************************/
void read_boot_sector(){

    boot_sector = malloc(sizeof(FAT32_BS));
    if(boot_sector == NULL){
        fprintf(stderr, "\nError in read_boot_sector() : Could not allocate space for FAT32_BS struct\n");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read = read(disk_image_fd, (void*)boot_sector, sizeof(FAT32_BS));
    if(bytes_read == -1){
        fprintf(stderr, "\nError in read_boot_sector() : read() returned -1 : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //Make sure that all the values that need to be 0 are zero. If not, there was a problem reading BS
    if(boot_sector->BPB_RootEntCnt != 0){
        fprintf(stderr, "\nError in read_boot_sector() : Expected 0 but value is non-zero : BPB_RootEntCnt - %#x\n",
                        boot_sector->BPB_RootEntCnt);
        exit(EXIT_FAILURE);
    }
    if(boot_sector->BPB_TotSec16 != 0){
        fprintf(stderr, "\nError in read_boot_sector() : Expected 0 but value is non-zero : BPB_TotSec16 - %#x\n",
                        boot_sector->BPB_TotSec16);
        exit(EXIT_FAILURE);
    }
    if(boot_sector->BPB_FATSz16 != 0){
        fprintf(stderr, "\nError in read_boot_sector() : Expected 0 but value is non-zero : BPB_FATSz16 - %#x\n",
                        boot_sector->BPB_FATSz16);
        exit(EXIT_FAILURE);
    }
    
    //Check signatures to ensure proper read
    if(boot_sector->BS_SigA != 0x55 || boot_sector->BS_SigB != 0xAA){
        fprintf(stderr, "\nError in read_boot_sector() : Bad signature : BS_SigA(0x55) was read as %#x : BS_SigB(0xAA) was read as %#x\n",
                        boot_sector->BS_SigA, boot_sector->BS_SigB);
        exit(EXIT_FAILURE);
    }

}

/********************************************************************
Reads the FSInfo data into an allocated FAT32_FSInfo struct
********************************************************************/
void read_FS_info(){

    fs_info_sector = malloc(sizeof(FAT32_FSInfo));
    if(fs_info_sector == NULL){
        fprintf(stderr, "\nError in read_FS_info() : Could not allocate space for FAT32_FSInfo struct\n");
        exit(EXIT_FAILURE);
    }
    
    ssize_t bytes_read = read(disk_image_fd, (void*)fs_info_sector, sizeof(FAT32_FSInfo));
    if(bytes_read == -1){
        fprintf(stderr, "\nError in read_FS_info() : read() returned -1 : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //Check signature to ensure successful read
    if(fs_info_sector->FSI_LeadSig != 0x41615252){
        fprintf(stderr, "\nError in read_FS_info() : Bad signature : FSI_LeadSig(0x41615252) was read as %#x\n",
                        fs_info_sector->FSI_LeadSig);
        exit(EXIT_FAILURE);
    }
    if(fs_info_sector->FSI_StrucSig != 0x61417272){
        fprintf(stderr, "\nError in read_FS_info() : Bad signature : FSI_LeadSig(0x61417272) was read as %#x\n",
                        fs_info_sector->FSI_StrucSig);
        exit(EXIT_FAILURE);
    }
    if(fs_info_sector->FSI_TrailSig != 0xAA550000){
        fprintf(stderr, "\nError in read_FS_info() : Bad signature : FSI_LeadSig(0xAA550000) was read as %#x\n",
                        fs_info_sector->FSI_TrailSig);
        exit(EXIT_FAILURE);
    }

    //NULL terminate the label string
    boot_sector->BS_VolLab[BS_VolLab_LENGTH] = '\0';
    
}

/********************************************************************
Reads the root directory into an allocated FAT32_Directory_Entry struct
********************************************************************/
void read_root_directory(){

    uint32_t root_dir_cluster_number;
    file_cluster_node* chain_head;
    uint8_t* bulk_data_buffer;

    //Allocate space for root directory
    root_directory = malloc(sizeof(FAT32_Directory_Entry));
    if(root_directory == NULL){
        fprintf(stderr, "\nError in read_root_directory() : Could not allocate space for root directory\n");
        exit(EXIT_FAILURE);
    }

    //Read the cluster into a buffer
    root_dir_cluster_number = boot_sector->BPB_RootClus;
    chain_head = build_clusterchain(root_dir_cluster_number);
    bulk_data_buffer = read_clusterchain(chain_head);

    //Copy the buffer into the allocated struct
    root_directory = (FAT32_Directory_Entry*)(&bulk_data_buffer[0]);
    current_directory_cluster = root_dir_cluster_number;
    
}

/********************************************************************
Searches the current directory for a file with the provided name
    If the file is found, write the clusterchain to a file in memory
********************************************************************/
void download_file(char* file_name){

    int file_descriptor;
    uint8_t* directory_data = read_clusterchain(build_clusterchain(current_directory_cluster));
    FAT32_Directory_Entry* dir = (FAT32_Directory_Entry*)&directory_data[0];

    //Search for the file in the current directory
    bool continue_parsing = true;
    bool found_file = false;
    
    while(continue_parsing){

        //check some FAT32-specific things
        if(dir->DIR_Name[0] == 0x00){
            //No more entries, stop searching
            continue_parsing = false;
            continue;
        }
        if(dir->DIR_Name[0] == 0xE5){
            //empty entry, skip it
            dir++;
            continue;
        }

        //format dir->DIR_Name to look like user input (name.extension)
        char trimmed_name[strlen(dir->DIR_Name) + 1];
        trim_file_name(dir->DIR_Name, trimmed_name);

        if(strcmp(file_name, trimmed_name) == 0 && dir->DIR_Attr != 0x10){
            continue_parsing = false;
            found_file = true;
            continue;
        }

        dir++;
    }

    if(found_file){

        uint32_t file_cluster_number = (dir->DIR_FstClusHI << 16) | dir->DIR_FstClusLO;
        uint8_t* file_data = read_clusterchain(build_clusterchain(file_cluster_number));

        char path[strlen(file_name) + strlen(FILE_OUTPUT_FOLDER)];
        sprintf(path, "%s%s", FILE_OUTPUT_FOLDER, file_name);

        file_descriptor = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0777);
        if(file_descriptor == -1){
            fprintf(stderr, "\nError in download_file() : Could not create output file : %s\n", strerror(errno));
        }else{
            write(file_descriptor, (void*)file_data, dir->DIR_FileSize);
            close(file_descriptor);
            fprintf(stdout, "Downloaded %d bytes to %s\n", dir->DIR_FileSize, path);
        }
        free(file_data);
    }else{
        fprintf(stderr, "Error: No such file\n");
    }
}

#pragma endregion Read_Functions

#pragma region Set_Functions

/********************************************************************
Searches the current directory for the specified directory
    If it exists, get its cluster number and set the global variable
********************************************************************/
void change_directory(char* destination){

    uint8_t* directory_data = read_clusterchain(build_clusterchain(current_directory_cluster));
    FAT32_Directory_Entry* dir = (FAT32_Directory_Entry*)&directory_data[0];

    bool continue_parsing = true;
    bool found_folder = false;
    while(continue_parsing){

        //Check some FAT32-specific values
        if(dir->DIR_Name[0] == 0x00){
            //No more directory entries to search
            continue_parsing = false;
            continue;
        }
        if(dir->DIR_Name[0] == 0xE5){
            //Entry is empty, skip it
            dir++;
            continue;
        }

        //Remove whitespaces from dir->DIR_Name
        int num_characters = 0;
        int j;
        while(dir->DIR_Name[num_characters] != ' '){
            num_characters++;
        }
        char no_space_name[num_characters];
        for(j = 0; j < num_characters; j++){
            no_space_name[j] = dir->DIR_Name[j];
        }
        no_space_name[j] = '\0';

        //Special case for Japanese characters. 0x05 means the character is actually 0xE5
        if(dir->DIR_Name[0] == 0x05){
            no_space_name[0] = 0xE5;
        }

        //Compare no_space_name to destination, and make sure it is a directory
        if((strcmp(no_space_name, destination) == 0) && (dir->DIR_Attr == 0x10)){

            found_folder = true;
            continue_parsing = false;
            
            //If destination is '.', do nothing
            if(strcmp(destination, ".") == 0){
                continue;
            }

            //Build the new cluster number
            uint32_t new_cluster_number = (dir->DIR_FstClusHI << 16) | dir->DIR_FstClusLO;

            //If destination is '..' and the value there is 0, '..' is the root directory
            if(strcmp(destination, "..") == 0 && new_cluster_number == 0){
                new_cluster_number = boot_sector->BPB_RootClus;
            }

            current_directory_cluster = new_cluster_number;
        }

        dir++;
    }
    
    if(!found_folder){
        fprintf(stderr, "Error: No such directory\n");
    }

}

#pragma endregion Set_Functions