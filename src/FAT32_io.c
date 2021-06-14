/********************************************************************
    Module: FAT32_io.c
    Author: Brennan Couturier

    Functions for opening/closing files and printing information
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#include "../include/FAT32_io.h"
#include "../include/FAT32_helpers.h"
#include "../include/FAT32_structs_globals.h"

#define _GNU_SOURCE


#pragma region File_Descriptor_Functions

/********************************************************************
Open the formatted FAT32 disk image for reading and writing
********************************************************************/
void open_disk_image(char* disk_image_path_in){

    disk_image_path = disk_image_path_in;

    disk_image_fd = open(disk_image_path, O_RDWR);
    if(disk_image_fd == -1){
        fprintf(stderr, "\nError in open_disk_image() : Failed to open %s : %s\n", disk_image_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Opened %s for reading and writing\n", disk_image_path);

}

/********************************************************************
Closes the disk image file descriptor that was opened at the beginning
********************************************************************/
void close_disk_image(){

    int err = close(disk_image_fd);
    if(err == -1){
        fprintf(stdout, "Error in close_disk_image() : Could not close file descriptor : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Closed %s\n", disk_image_path);

}

#pragma endregion File_Descriptor_Functions

#pragma region Printing_Functions

/********************************************************************
Prints out some values contained in the boot sector to a file
	in the debug directory
********************************************************************/
void print_boot_sector_info(){

    /*
    char* boot_sector_output_filename = "./debug/boot_sector_output.txt";

    FILE* boot_sector_output_file = fopen(boot_sector_output_filename, "w+");
    if(boot_sector_output_file == NULL){
        fprintf(stderr, "\nError in print_boot_sector() : Could not open %s : %s\n", 
                boot_sector_output_filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    */
    
    //Device info
    char* OEM_name = boot_sector->BS_OEMName;
    char* label = boot_sector->BS_VolLab;
    char* file_system_type = get_FAT_type();
    uint8_t media_type = boot_sector->BPB_Media;
    long long size = (long long)boot_sector->BPB_BytesPerSec * (long long)boot_sector->BPB_TotSec32;
    uint8_t drive_number = boot_sector->BS_DrvNum;

    //Geometry
    uint16_t bytes_per_sector = boot_sector->BPB_BytesPerSec;
    uint8_t sectors_per_cluster = boot_sector->BPB_SecPerClus;
    uint32_t total_sectors = boot_sector->BPB_TotSec32;
    uint16_t sectors_per_track = boot_sector->BPB_SecPerTrk;
    uint16_t heads = boot_sector->BPB_NumHeads;
    uint32_t hidden_sectors = boot_sector->BPB_HiddSec;

    //FS Info
    char* volume_id = root_directory->DIR_Name;
    uint8_t version_high = boot_sector->BPB_FSVerHigh;
    uint8_t version_low = boot_sector->BPB_FSVerLow;
    uint16_t reserved_sectors = boot_sector->BPB_RsvdSecCnt;
    uint8_t number_of_FATs = boot_sector->BPB_NumFATs;
    uint32_t FAT_size = boot_sector->BPB_FATSz32;
    char* mirrored_FAT = (((boot_sector->BPB_ExtFlags & 0x80) == 0) ? "0 (yes)" : "1 (no)"); //0x80 is just a mask used to isolate bit 7
    uint16_t boot_sector_backup_sector_no = boot_sector->BPB_BkBootSec;

    fprintf(stdout, "%s\n", root_directory->DIR_Name);

    //Print all that info
    fprintf(stdout, 
        "---- Device Info ----\n"
        " OEM Name: %s\n"
        " Label: %s\n"
        " File System Type: %s\n"
        " Media Type: %#x\n"
        " Size: %lld\n"
        " Drive Number: %d\n\n"
        "--- Geometry ---\n"
        " Bytes per Sector: %d\n"
        " Sectors per Cluster: %d\n"
        " Total Sectors: %d\n"
        " Geom: Sectors per Track: %d\n"
        " Geom: Heads: %d\n"
        " Hidden Sectors: %d\n\n"
        "--- FS Info ---\n"
        " Volume ID: %s\n"
        " Version: %d:%d\n"
        " Reserved Sectors: %d\n"
        " Number of FATs: %d\n"
        " FAT Size: %d\n"
        " Mirrored FAT: %s\n"
        " Boot Sector Backup Sector No: %d\n",
        OEM_name, label, file_system_type, media_type,
        size, drive_number, bytes_per_sector, sectors_per_cluster,
        total_sectors, sectors_per_track, heads, hidden_sectors,
        volume_id, version_high, version_low, reserved_sectors, number_of_FATs,
        FAT_size, mirrored_FAT, boot_sector_backup_sector_no
    );

    /*
    int err = fclose(boot_sector_output_file);
    if(err == EOF){
        fprintf(stderr, "\nError in print_boot_sector() : Could not close %s : %s\n", 
                boot_sector_output_filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    */

}

/********************************************************************
Prints out information about the root directory, for debugging
********************************************************************/
void print_root_directory(){

    uint8_t dir_attr = root_directory->DIR_Attr;
    uint16_t crt_date = root_directory->DIR_CrtDate;
    uint16_t crt_time = root_directory->DIR_CrtTime;
    uint32_t file_size = root_directory->DIR_FileSize;
    uint16_t fst_clus_hi = root_directory->DIR_FstClusHI;
    uint16_t fst_clus_lo = root_directory->DIR_FstClusLO;
    uint16_t lst_acc_date = root_directory->DIR_LstAccDate;
    char* dir_name = root_directory->DIR_Name;
    uint8_t ntres = root_directory->DIR_NTRes;
    uint16_t wrt_date = root_directory->DIR_WrtDate;
    uint16_t wrt_time = root_directory->DIR_WrtTime;

    fprintf(stdout,
        "dir_attr: %#x\n"
        "crt_date: %d\n"
        "crt_time: %d\n"
        "file_size: %d\n"
        "fst_clus_hi: %d\n"
        "fst_clus_lo: %d\n"
        "lst_acc_date: %d\n"
        "dir_name: %s\n"
        "ntres: %d\n"
        "wrt_date: %d\n"
        "wrt_time: %d\n",
        dir_attr, crt_date, crt_time,
        file_size, fst_clus_hi, fst_clus_lo,
        lst_acc_date, dir_name, ntres,
        wrt_date, wrt_time
    );

}

/********************************************************************
Prints out information about the current directory and all the files
    and subfolders in it
********************************************************************/
void print_current_directory(){

    fprintf(stdout, "\nDIRECTORY LISTING\n");
    fprintf(stdout, "Volume ID: %s\n\n", root_directory->DIR_Name);

    uint8_t* data_buffer = read_clusterchain(build_clusterchain(current_directory_cluster));
    FAT32_Directory_Entry* dir = (FAT32_Directory_Entry*)&data_buffer[0];
    int dirs_parsed = 0;

    //Do not print the first 2 entries of the root directory, they are garbage
    if(current_directory_cluster == boot_sector->BPB_RootClus){
        dir++;
        dir++;
        dirs_parsed = 2;
    }

    bool continue_parsing = true;
    while(continue_parsing){
        if(dir->DIR_Name[0] == 0x00){
            //No more entries
            continue_parsing = false;
            continue;
        }
        if(dir->DIR_Name[0] == 0xE5){
            //An empty entry
            continue;
        }
        if(dir->DIR_Name[0] == 0x05){
            //Byte is actually 0xE5
            dir->DIR_Name[0] = 0xE5;
        }
        char dir_name[strlen(dir->DIR_Name) + 1];
        if(dir->DIR_Attr == 0x10){
            //entry is a directory
            trim_directory_name(dir->DIR_Name, dir_name);
            fprintf(stdout, "<%s>\t\t%d\n", dir_name, dir->DIR_FileSize);
        }else{
            trim_file_name(dir->DIR_Name, dir_name);
            fprintf(stdout, "%s\t\t%d\n", dir_name, dir->DIR_FileSize);
        }


        dir++;
        dirs_parsed++;

        //print the . and .. entries, then skip every other entry (they are garbage)
        if(dirs_parsed >= 2){
            dir++;
        }
    }

    //prnt free space
    long long bytes_free = ((long long)fs_info_sector->FSI_Free_Count) * ((long)boot_sector->BPB_SecPerClus * (long)boot_sector->BPB_BytesPerSec);
    fprintf(stdout, "---Bytes Free: %lld\n", bytes_free);

    //print done message
    fprintf(stdout, "---DONE\n");
}

#pragma endregion Printing_Functions