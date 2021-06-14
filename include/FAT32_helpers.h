/********************************************************************
    Module: FAT32_helpers.h
    Author: Brennan Couturier

    Extra functions to perform tedious calculations
********************************************************************/

#ifndef FAT32_HELP_H
#define FAT32_HELP_H

#include <inttypes.h>
#include <stdbool.h>

#include "FAT32_structs_globals.h"

#pragma region Get_Functions

/********************************************************************
Calculate how many root directory sectors there are. If FAT32 this 
	will be 0, otherwise we should stop the program because we
	aren't operating on FAT12/16.
********************************************************************/
uint16_t get_root_dir_sectors();

/********************************************************************
Calculate the sector number of the first data sector relative
	to the sector that contains the BPB
********************************************************************/
uint32_t get_first_data_sector();

/********************************************************************
Calculate the first sector of a given cluster number
********************************************************************/
uint32_t get_first_sector_of_cluster(uint32_t cluster_number);

/********************************************************************
Calculate the number of sectors in the data region of the volume
********************************************************************/
uint32_t get_num_data_region_sectors();

/********************************************************************
Calculate the number of data clusters in total starting at cluster 2
	This computation rounds down
********************************************************************/
uint32_t get_num_clusters();

/********************************************************************
Calculate FAT entry number for given cluster number N
	This is the sector number of the FAT sector that contains the 
	entry for cluster N in the first FAT
********************************************************************/
uint32_t get_FAT_sector_number_for_cluster(uint32_t cluster_number);

/********************************************************************
Calculate FAT entry offset value for given cluster number
********************************************************************/
uint32_t get_FAT_entry_offset_for_cluster(uint32_t cluster_number);

/********************************************************************
Fetch the contents of the given cluster's FAT entry
********************************************************************/
uint32_t get_FAT_entry_contents(uint32_t cluster_number);

/********************************************************************
Returns a string representing the FAT version we're using
	"FAT12", "FAT16" or "FAT32"
********************************************************************/
char* get_FAT_type();

#pragma endregion Get_Functions

#pragma region Value_Check_Functions

/********************************************************************
Checks if the FAT entry is EOC or not
********************************************************************/
bool is_FAT_entry_EOC(uint32_t FAT_entry);

#pragma endregion Value_Check_Functions

#pragma region Clusterchain_Functions

/********************************************************************
This function builds a linked list of clusters. It starts at the
	cluster specified by cluster_number, then adds clusters into
	the list until it finds an EOC marker in the FAT. It then returns
	a pointer to the head of this list
********************************************************************/
file_cluster_node* build_clusterchain(uint32_t cluster_number);

/********************************************************************
Print out all the cluster numbers in the chain, ending with EOC
********************************************************************/
void print_clusterchain(file_cluster_node* chain_head);

/********************************************************************
Read all the clusters into one bit char array (byte array), to be
	formatted by the caller
********************************************************************/
uint8_t* read_clusterchain(file_cluster_node* chain_head);

/********************************************************************
Free all the nodes in the clusterchain
********************************************************************/
void free_clusterchain(file_cluster_node* chain_head);

#pragma endregion Clusterchain_Functions

#pragma region String_Trim_Functions

/********************************************************************
Gets rid of all the trailing whitespace in the file name
********************************************************************/
void trim_directory_name(char* name_in, char* name_out);

/********************************************************************
Gets rid of the whitespace between the file name and the extension,
	replaces it with 1 '.'
********************************************************************/
void trim_file_name(char* name_in, char* name_out);

#pragma endregion String_Trim_Functions

#endif