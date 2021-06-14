/********************************************************************
    Module: shell.h
    Author: Brennan Couturier

    Shell loop to manage user interaction
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "../include/shell.h"
#include "../include/FAT32_io.h"
#include "../include/FAT32_disk_management.h"

#define BUFFER_SIZE 256
#define CMD_INFO "INFO"
#define CMD_DIR "DIR"
#define CMD_CD "CD"
#define CMD_GET "GET"
#define CMD_PUT "PUT"
#define CMD_EXIT "EXIT"

/********************************************************************
Loop that reads user input and executes commands
********************************************************************/
void run_shell(){

    int i;
    bool running = true;
    char input[BUFFER_SIZE];

    while(running){

        //print prompt
        fprintf(stdout, "> ");

        //read user input
        if(fgets(input, BUFFER_SIZE, stdin) == NULL){
            fprintf(stderr, "\nError in run_shell() : fgets() returned NULL\n");
            running = false;
            continue; //skip the rest of the loop
        }

        //remove newline and make input uppercase
        input[strlen(input) - 1] = '\0';
        for(i = 0; i < strlen(input) + 1; i++){
            input[i] = toupper(input[i]);
        }

        //check input
        if(strncmp(input, CMD_EXIT , strlen(CMD_EXIT )) == 0){

            running = false;
            continue;

        }else if(strncmp(input, CMD_CD , strlen(CMD_CD)) == 0){

            //Tokenize input
            char* save_ptr;
            __strtok_r(input, " ", &save_ptr); //diregard first token
            char* destination = __strtok_r(NULL, " ", &save_ptr);

            if(destination == NULL){
                fprintf(stderr, "Usage: \"cd <directory>\"\n");
            }else{
                change_directory(destination);
            }
            
        }else if(strncmp(input, CMD_DIR , strlen(CMD_DIR )) == 0){

            print_current_directory();
            
        }else if(strncmp(input, CMD_INFO , strlen(CMD_INFO )) == 0){

            print_boot_sector_info();
            
        }else if(strncmp(input, CMD_GET , strlen(CMD_GET )) == 0){
    
            //Tokenize input
            char* save_ptr;
            __strtok_r(input, " ", &save_ptr); //diregard first token
            char* file_name = __strtok_r(NULL, " ", &save_ptr);

            if(file_name == NULL){
                fprintf(stderr, "Usage: \"get <file name>\"\n");
            }else{
                download_file(file_name);
            }
            
        }else if(strncmp(input, CMD_PUT , strlen(CMD_PUT )) == 0){


            
        }else{
            
            fprintf(stderr, "\nCommand not found\n");

        }

    }

    fprintf(stdout, "\nExiting...\n");

}