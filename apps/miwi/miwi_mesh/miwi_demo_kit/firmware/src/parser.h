/* 
 * File: parser.h
 * Author: Clayton Reid
 * Comments: 
 * Revision history: 
 */
 
//#ifndef __PARSER_H
//#define	__PARSER_H

#include "system.h"

#define PARSE_CMD_error -1
#define PARSE_CMD_begin 0
#define PARSE_CMD_end 1
#define PARSE_CMD_addUser 2
#define PARSE_CMD_master 3
#define PARSE_CMD_deleteUser 4
#define PARSE_CMD_listUsers 5

void addUser(char * instr);

void master(char * instr);

void deleteUser(char * instr);

void listUsers();

uint8_t getInstrNum(char * instr);

void executeCommands(char * inputInstr);