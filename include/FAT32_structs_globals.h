/********************************************************************
    Module: FAT32_structs_globals.h
    Author: Brennan Couturier

    All the structs and global variables the program will use
********************************************************************/

#ifndef FAT32_SG_H
#define FAT32_SG_H

#include <inttypes.h>

#define BS_OEMName_LENGTH 8
#define BS_VolLab_LENGTH 11
#define BS_FilSysType_LENGTH 8
#define _FILE_OFFSET_BITS 64
#define FAT_ENTRY_MASK 0x0FFFFFFF
#define EOC_LOW_BOUND 0x0FFFFFF8 //If a FAT entry is >= EOC_LOW_BOUND, the entry is EOC
#define FILE_OUTPUT_FOLDER "./files/"

#pragma region Structs
/********************************************************************
Struct for FAT32 Boot Sector and BPB
********************************************************************/
#pragma pack(push)
#pragma pack(1)
typedef struct FAT32_BS_struct{
    //Starting at offset (byte) 0
    char BS_jmpBoot[3];
    char BS_OEMName[BS_OEMName_LENGTH]; //"MSWIN4.1" for example
    uint16_t BPB_BytesPerSec; //Dan: 512
	uint8_t BPB_SecPerClus; //Dan: 8
	uint16_t BPB_RsvdSecCnt; //non-zero, typically 32
	uint8_t BPB_NumFATs; //Dan: 2
	uint16_t BPB_RootEntCnt; //must be 0
	uint16_t BPB_TotSec16; //must be 0
	uint8_t BPB_Media; //Dan: 0xF8 Value here must be put in the low byte of the FAT[0] entry
	uint16_t BPB_FATSz16; //must be 0
	uint16_t BPB_SecPerTrk; //Dan: 63
	uint16_t BPB_NumHeads; //Dan: 255
	uint32_t BPB_HiddSec; //Dan: 62
	uint32_t BPB_TotSec32; //non-zero, total count of sectors on volume

    //Starting at offset (byte) 36
	uint32_t BPB_FATSz32; //Dan: 15423 Number of bits occupied by one FAT
	uint16_t BPB_ExtFlags; //Specific bits set as flags
	uint8_t BPB_FSVerLow; //Dan: 0 Minor revision version number
	uint8_t BPB_FSVerHigh; //Dan: 0 Major revision version number
	uint32_t BPB_RootClus; //First cluster of root directory, usually 2
	uint16_t BPB_FSInfo; //Sector number for FSINFO struct, usually 1
	uint16_t BPB_BkBootSec; //Usually 6, other value not recommended
	char BPB_reserved[12]; //Must never be accessed, always zeroed out
	uint8_t BS_DrvNum; //"same as FAT12/16", no further explanation
	uint8_t BS_Reserved1; //"same as FAT12/16", no further explanation
	uint8_t BS_BootSig; //"same as FAT12/16", no further explanation
	uint32_t BS_VolID; //"same as FAT12/16", no further explanation
	char BS_VolLab[BS_VolLab_LENGTH]; //"same as FAT12/16", no further explanation
	char BS_FilSysType[BS_FilSysType_LENGTH]; //"FAT32   ", 3 spaces
	char BS_CodeReserved[420];
	uint8_t BS_SigA; //Should be 0x55
	uint8_t BS_SigB; //Should be 0xAA
} FAT32_BS;
#pragma pack(pop)

/********************************************************************
Struct for File Stystem Info 
********************************************************************/
#pragma pack(push)
#pragma pack(1)
typedef struct FAT32_FSInfo_struct{
    uint32_t FSI_LeadSig; //Value 0x41615252
    char FSI_Reserved1[480]; //Must never be accessed, always zeroed out
    uint32_t FSI_StrucSig; //Value 0x61417272
    uint32_t FSI_Free_Count; //If 0xFFFFFFFF, free count is unknown
    uint32_t FSI_Nxt_Free; //If 0xFFFFFFFF, start search at cluster 2
    char FSI_Reserved2[12]; //Must never be accessed, always zeroed out
    uint32_t FSI_TrailSig; //Value 0xAA550000
} FAT32_FSInfo;
#pragma pack(pop)

/********************************************************************
Struct for each directory entry
********************************************************************/
#pragma pack(push)
#pragma pack(1)
typedef struct FAT32_Directory_Entry_struct{
	char DIR_Name[11]; //short name, [0] cannot be 0x20 (space)
	uint8_t DIR_Attr; //Upper 2 bits set to 0 when file is created, never used
	uint8_t DIR_NTRes; //Set to 0 when file is created, never used
	uint8_t DIR_CrtTimeTenth; //Millisecond stamp at file creation time, is a count of tenths of seconds
	uint16_t DIR_CrtTime; //Time file was created
	uint16_t DIR_CrtDate; //Date file was created
	uint16_t DIR_LstAccDate; //Date of last read or write. If write, should be set to same date as DIR_WrtDate
	uint16_t DIR_FstClusHI; //High word of this entry's first cluster number
	uint16_t DIR_WrtTime; //Time of last write. File creation is considered a write
	uint16_t DIR_WrtDate; //Date of last write. File creation is considered a write
	uint16_t DIR_FstClusLO; //Low word of this entry's first cluster number
	uint32_t DIR_FileSize; //32-bit DWORD holding this file's size in bytes
} FAT32_Directory_Entry;
#pragma pack(pop)

/********************************************************************
Linked list node to keep track of all the clusters in a file
********************************************************************/
#pragma pack(push)
#pragma pack(1)
typedef struct file_cluster_node_struct{
	uint32_t cluster_number;
	struct file_cluster_node_struct* next;
} file_cluster_node;
#pragma pack(pop)

#pragma endregion Structs


#pragma region Globals

/********************************************************************
Global variable used to store the disk image file descriptor
********************************************************************/
int disk_image_fd;

/********************************************************************
Path to diskimage from root directory (where makefile is)
********************************************************************/
char* disk_image_path;

/********************************************************************
Global reference to boot sector
********************************************************************/
FAT32_BS* boot_sector;

/********************************************************************
Global reference to file system sector
********************************************************************/
FAT32_FSInfo* fs_info_sector;

/********************************************************************
Global reference to root directory
********************************************************************/
FAT32_Directory_Entry* root_directory;

/********************************************************************
Global reference to the first cluster of the current directory
********************************************************************/
uint32_t current_directory_cluster;

#pragma endregion Globals

#endif