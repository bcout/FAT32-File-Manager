/********************************************************************
    Module: FAT32_helpers.h
    Author: Brennan Couturier

    Extra functions to perform tedious calculations
********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>

#include "../include/FAT32_structs_globals.h"
#include "../include/FAT32_helpers.h"

#pragma region Get_Functions

/********************************************************************
Calculate how many root directory sectors there are. If FAT32 this 
	will be 0, otherwise we should stop the program because we
	aren't operating on FAT12/16.
********************************************************************/
uint16_t get_root_dir_sectors(){

    uint16_t root_entry_count = boot_sector->BPB_RootEntCnt;
    uint16_t bytes_per_sector = boot_sector->BPB_BytesPerSec;

    uint16_t root_dir_sectors = ((root_entry_count * 32) + (bytes_per_sector - 1)) / bytes_per_sector;

    return root_dir_sectors;

}

/********************************************************************
Calculate the sector number of the first data sector relative
	to the sector that contains the BPB
********************************************************************/
uint32_t get_first_data_sector(){

    uint16_t root_dir_sectors = get_root_dir_sectors();
    uint16_t reserved_sector_count = boot_sector->BPB_RsvdSecCnt;
    uint8_t num_FATs = boot_sector->BPB_NumFATs;
    uint32_t FAT_size = boot_sector->BPB_FATSz32;

    uint32_t first_data_sector = reserved_sector_count + (num_FATs * FAT_size) + root_dir_sectors;

    return first_data_sector;

}

/********************************************************************
Calculate the first sector of a given cluster number
********************************************************************/
uint32_t get_first_sector_of_cluster(uint32_t cluster_number){

    uint32_t first_data_sector = get_first_data_sector();
    uint8_t sectors_per_cluster = boot_sector->BPB_SecPerClus;

    uint32_t first_sector_of_cluster = ((cluster_number - 2) * sectors_per_cluster) + first_data_sector;

    return first_sector_of_cluster;

}

/********************************************************************
Calculate the number of sectors in the data region of the volume
********************************************************************/
uint32_t get_num_data_region_sectors(){

    uint16_t root_dir_sectors = get_root_dir_sectors();
    uint32_t total_sectors = boot_sector->BPB_TotSec32;
    uint16_t reserved_sector_count = boot_sector->BPB_RsvdSecCnt;
    uint8_t num_FATs = boot_sector->BPB_NumFATs;
    uint32_t FAT_size = boot_sector->BPB_FATSz32;
    
    uint32_t num_data_region_sectors = total_sectors - (reserved_sector_count + (num_FATs * FAT_size) + root_dir_sectors);

    return num_data_region_sectors;

}

/********************************************************************
Calculate the number of data clusters in total starting at cluster 2
    This computation rounds down
********************************************************************/
uint32_t get_num_clusters(){

    uint32_t num_data_region_sectors = get_num_data_region_sectors();
    uint8_t sectors_per_cluster = boot_sector->BPB_SecPerClus;

    uint32_t num_clusters = num_data_region_sectors / sectors_per_cluster;

    return num_clusters;

}

/********************************************************************
Calculate FAT entry number for given cluster number N
	This is the sector number of the FAT sector that contains the 
	entry for cluster N in the first FAT
********************************************************************/
uint32_t get_FAT_sector_number_for_cluster(uint32_t cluster_number){

    uint32_t FAT_offset = cluster_number * 4;
    uint16_t reserved_sector_count = boot_sector->BPB_RsvdSecCnt;
    uint16_t bytes_per_sector = boot_sector->BPB_BytesPerSec;

    uint32_t FAT_sector_number_for_cluster = reserved_sector_count + (FAT_offset / bytes_per_sector);

    return FAT_sector_number_for_cluster;

}

/********************************************************************
Calculate FAT entry offset value for given cluster number
********************************************************************/
uint32_t get_FAT_entry_offset_for_cluster(uint32_t cluster_number){

    uint32_t FAT_offset = cluster_number * 4;
    uint16_t bytes_per_sector = boot_sector->BPB_BytesPerSec;

    uint32_t FAT_entry_offset_for_cluster = FAT_offset % bytes_per_sector; //Remainder of FAT_offset / bytes_per_sector

    return FAT_entry_offset_for_cluster;

}

/********************************************************************
Fetch the contents of the given cluster's FAT entry
********************************************************************/
uint32_t get_FAT_entry_contents(uint32_t cluster_number){

    ssize_t bytes_read;
    uint32_t FAT_entry;
    uint32_t FAT_sector_number = get_FAT_sector_number_for_cluster(cluster_number);
    uint32_t FAT_entry_offset = get_FAT_entry_offset_for_cluster(cluster_number);
    __off_t FAT_entry_byte_location = (FAT_sector_number * boot_sector->BPB_BytesPerSec) + FAT_entry_offset;

    lseek(disk_image_fd, FAT_entry_byte_location, SEEK_SET);
    bytes_read = read(disk_image_fd, (void*)(&FAT_entry), sizeof(uint32_t));
    if(bytes_read == -1){
        fprintf(stderr, "\nError in get_FAT_entry_contents() : Read returned -1\n");
        exit(EXIT_FAILURE);
    }

    //The actual entry is only 28-bits, so mask out the high four bits
    return FAT_entry & FAT_ENTRY_MASK;

}

/********************************************************************
Returns a string representing the FAT version we're using
********************************************************************/
char* get_FAT_type(){

    uint32_t count_of_clusters = get_num_clusters();
    char* return_value = "";

    if(count_of_clusters < 4085){
        //volume is FAT12
        return_value = "FAT12";
    }else if(count_of_clusters < 65525){
        //volume is FAT16
        return_value = "FAT16";
    }else{
        //volume is FAT32
        return_value = "FAT32";
    }

    return return_value;

}

#pragma endregion Get_Functions

#pragma region Value_Check_Functions

/********************************************************************
Checks if the FAT entry is EOC or not
********************************************************************/
bool is_FAT_entry_EOC(uint32_t FAT_entry){

    if(FAT_entry >= EOC_LOW_BOUND){
        return true;
    }

    return false;

}

#pragma endregion Value_Check_Functions

#pragma region Clusterchain_Functions

file_cluster_node* build_clusterchain(uint32_t cluster_number_in){

    uint32_t FAT_entry;
    bool is_EOC;
    
    file_cluster_node* to_return = malloc(sizeof(file_cluster_node));
    if(to_return == NULL){
        fprintf(stderr, "\nError in build_clusterchain() : Could not allocate space for first cluster node\n");
        exit(EXIT_FAILURE);
    }

    to_return->cluster_number = cluster_number_in;
    to_return->next = NULL;

    file_cluster_node* curr = to_return;

    FAT_entry = get_FAT_entry_contents(to_return->cluster_number);
    is_EOC = is_FAT_entry_EOC(FAT_entry);
    while(!is_EOC){
        //FAT entry is the next cluster number
        file_cluster_node* to_add = malloc(sizeof(file_cluster_node));
        if(to_add == NULL){
            fprintf(stderr, "\nError in build_clusterchain() : Could not allocate space for to_add cluster node\n");
            exit(EXIT_FAILURE);
        }

        to_add->cluster_number = FAT_entry;
        to_add->next = NULL;
        curr->next = to_add;
        curr = curr->next;

        FAT_entry = get_FAT_entry_contents(curr->cluster_number);
        is_EOC = is_FAT_entry_EOC(FAT_entry);
    }

    return to_return;

}

/********************************************************************
Print out all the cluster numbers in the chain, ending with EOC
********************************************************************/
void print_clusterchain(file_cluster_node* chain_head){

    file_cluster_node* curr = chain_head;
    while(curr != NULL){
        fprintf(stdout, "%d->", curr->cluster_number);
        curr = curr->next;
    }
    fprintf(stdout, "EOC\n");

}

/********************************************************************
Read all the clusters into one bit char array (byte array), to be
	formatted by the caller
********************************************************************/
uint8_t* read_clusterchain(file_cluster_node* chain_head){

    //Declare variables
    file_cluster_node* curr = chain_head;
    int num_nodes = 0;
    int cluster_offset = 0;

    size_t cluster_size = boot_sector->BPB_BytesPerSec * boot_sector->BPB_SecPerClus;
    ssize_t bytes_read;
    uint8_t* bulk_buffer;
    __off_t byte_offset;


    //count number of nodes in chain
    while(curr != NULL){
        num_nodes++;
        curr = curr->next;
    }
    curr = chain_head;

    //Allocate the bulk buffer
    bulk_buffer = malloc(cluster_size * num_nodes);
    if(bulk_buffer == NULL){
        fprintf(stderr, "\nError in read_clusterchain() : Could not allocate space for bulk buffer\n");
        exit(EXIT_FAILURE);
    }

    //Go through each cluster in the chain, move their contents into the bulk buffer
    while(curr != NULL){
        byte_offset = get_first_sector_of_cluster(curr->cluster_number) * boot_sector->BPB_BytesPerSec;
        lseek(disk_image_fd, byte_offset, SEEK_SET);

        bytes_read = read(disk_image_fd, (void*)bulk_buffer + (cluster_size * cluster_offset), cluster_size);
        if(bytes_read == -1){
            fprintf(stderr, "\nError in read_clusterchain() : Read returned -1\n");
            exit(EXIT_FAILURE);
        }

        cluster_offset++;
        curr = curr->next;
    }

    free_clusterchain(chain_head);

    return bulk_buffer;

}

/********************************************************************
Free all the nodes in the clusterchain
********************************************************************/
void free_clusterchain(file_cluster_node* chain_head){

    file_cluster_node* tmp;

    while(chain_head != NULL){
        tmp = chain_head;
        chain_head = chain_head->next;
        free(tmp);
    }

}

#pragma endregion Clusterchain_Functions

#pragma region String_Trim_Functions

/********************************************************************
Gets rid of all the trailing whitespace in the file name
********************************************************************/
void trim_directory_name(char* name_in, char* name_out){

    strncpy(name_out, name_in, strlen(name_in));

    int i = 0;
    while(name_out[i] != ' '){
        i++;
    }
    name_out[i] = '\0';
}

/********************************************************************
Gets rid of the whitespace between the file name and the extension,
	replaces it with 1 '.'
********************************************************************/
void trim_file_name(char* name_in, char* name_out){

    char name[strlen(name_in) - 1];
    strncpy(name, name_in, strlen(name_in) - 1);

    int i;
    //Get the file name
    int file_name_length = 0;
    while(name[file_name_length] != ' '){
        file_name_length++;
    }
    char file_name[file_name_length];
    file_name[file_name_length] = '\0';

    for(i = 0; i < file_name_length; i++){
        file_name[i] = name[i];
    }

    //Get extension
    i = strlen(name) - 1;
    int extension_length = 0;
    while(name[i] != ' '){
        i--;
        extension_length++;
    }
    i++;

    char extension[extension_length];
    extension[extension_length] = '\0';

    int j;
    for(j = 0; j < extension_length; j++){
        extension[j] = name[i];
        i++;
    }

    //Combine the two
    char combined[file_name_length + extension_length + 1];
    strncpy(combined, file_name, file_name_length);
    strncpy(combined + file_name_length, ".", 1);
    strncpy(combined + file_name_length + 1, extension, extension_length);
    combined[file_name_length + extension_length + 1] = '\0';
    
    strncpy(name_out, combined, strlen(combined) + 1);
}

#pragma endregion String_Trim_Functions