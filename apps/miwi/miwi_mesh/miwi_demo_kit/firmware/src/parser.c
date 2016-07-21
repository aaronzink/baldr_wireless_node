/*
 * File:   parser.c
 * Author: Clayton Reid
 *
 * Created on July 15, 2016, 3:05 PM
 */

#include <system.h>
#include <stdio.h>
#include <string.h>
#include <parser.h>

#define USER_LIMIT 3
#define MESSAGE_DELAY 5000

char defaultPassword[20] = "asdf1234";
char currentUsers[USER_LIMIT][16];
uint8_t userCount = 0;

struct user_s
{
    char userName[16];
    char phoneNumber[12];
    bool master;
};

void addUser(char * instr)
{
    char * token = strtok(instr, " ");
    char userName[16];
    char password[16];
    uint8_t stage = 0;
    bool error_flag = false;
    
    while(token != NULL)
    {
        if(stage == 1){
            strcpy(userName, token);
        }else if(stage == 2){
            strcpy(password, token);
        }else if(stage > 2){
            error_flag = true;
        }
        stage++;
        token = strtok(NULL, " ");
    }
    
    if(!error_flag)
    {
        if(strcmp(password,defaultPassword) == 0)
        {
            strcpy(currentUsers[userCount], userName);
            LCD_Erase();
            sprintf((char *)LCDText, (char*)currentUsers[userCount]);
            sprintf((char *)&(LCDText[16]), (char*)"Added as user   ");
            LCD_Update();   
            userCount++;
        }else{
            LCD_Erase();
            sprintf((char *)LCDText, (char*)"Incorrect Pass  ");
            sprintf((char *)&(LCDText[16]), (char*)password);
            LCD_Update();
        }
    }else{
        LCD_Erase();
        sprintf((char *)LCDText, (char*)"  ADDUSER ERROR ");
        sprintf((char *)&(LCDText[16]), (char*)"  EXTRA TOKENS  ");
        LCD_Update();  
    }
    
}

void master(char * instr)
{
    char * token = strtok(instr, " ");
    char userName[16];
    char password[16];
    uint8_t stage = 0;
    bool error_flag = false;
    
    while(token != NULL)
    {
        if(stage == 1){
            strcpy(userName, token);
        }else if(stage == 2){
            strcpy(password, token);
        }else if(stage > 2){
            error_flag = true;
        }
        stage++;
        token = strtok(NULL, " ");
    }
    
    if(!error_flag)
    {
        LCD_Erase();
        sprintf((char *)LCDText, (char*)userName);
        sprintf((char *)&(LCDText[16]), (char*)"   Made Master  ");
        LCD_Update();   
    }else{
        LCD_Erase();
        sprintf((char *)LCDText, (char*)"  MASTER ERROR  ");
        sprintf((char *)&(LCDText[16]), (char*)"  EXTRA TOKENS  ");
        LCD_Update();  
    }
}

void deleteUser(char * instr)
{
    
}

void listUsers()
{
    LCD_Erase();
    sprintf((char *)LCDText, "List of Users   ");
    sprintf((char *)&(LCDText[16]), (char*)"               ");
    LCD_Update();
    
    for(int i = 0; i<userCount; i++)
    {
        DELAY_ms(MESSAGE_DELAY);
        
        LCD_Erase();
        sprintf((char *)LCDText, (char*)currentUsers[i]);
        sprintf((char *)&(LCDText[16]), (char*)"               ");
        LCD_Update();
    }
}

//TODO: Might cause memory leak? Delete the string pointers?
uint8_t getInstrNum(char * instr)
{
    char input[20];
    strcpy(input, instr);
    char * token;
    uint8_t num;
    
    token = strtok(input," ");
    
    if(strcmp(token,"<config-begin>") == 0){
        num = PARSE_CMD_begin;
    }else if(strcmp(token,"<config-end>") == 0){
        num = PARSE_CMD_end;
    }else if(strcmp(token,"<config-addUser>") == 0){
        num = PARSE_CMD_addUser;
    }else if(strcmp(token,"<config-master>") == 0){
        num = PARSE_CMD_master;
    }else if(strcmp(token,"<config-listUsers>") == 0){
        num = PARSE_CMD_listUsers;
    }else{
        num = PARSE_CMD_error;
    }
    
    return num;    
}

void executeCommands(char * inputInstr)
{
    char input[150];
    strcpy(input,inputInstr);
    char * instr[10];
    uint8_t instr_num = 0;
    bool instr_er = false;
    uint8_t i = 0;
    uint8_t j = 0;
    
    LCD_BacklightON();
    LCD_Erase();
    sprintf((char *)LCDText, (char*)"   Parse Demo   "  );
    sprintf((char *)&(LCDText[16]), (char*)"                ");
    LCD_Update();
    
    DELAY_ms(3000);
    
    instr[i] = strtok(input,"\n");
    
    while(instr[i] != NULL)
    {
        i++;
        instr[i] = strtok(NULL,"\n");
    }
        
    LCD_BacklightOFF();
    LCD_Erase();
    sprintf((char *)LCDText, (char*)"   Parse Demo   "  );
    sprintf((char *)&(LCDText[16]), "        %01d       ", i);
    LCD_Update();
    
    DELAY_ms(5000);
    
    for(j = 0; j<i; j++)
    {        
        instr_num = getInstrNum(instr[j]);
        switch(instr_num)
        {
            case PARSE_CMD_begin :
                LCD_Erase();
                sprintf((char *)LCDText, (char*)" <config-begin> "  );
                sprintf((char *)&(LCDText[16]), (char*)"                ");
                LCD_Update();
                break;

            case PARSE_CMD_end :
                LCD_Erase();
                sprintf((char *)LCDText, (char*)"  <config-end>  "  );
                sprintf((char *)&(LCDText[16]), (char*)"                ");
                LCD_Update();
                break;  
            
            case PARSE_CMD_addUser :
                addUser(instr[j]);
                break;
                
            case PARSE_CMD_master :
                master(instr[j]);
                break;
                
            case PARSE_CMD_deleteUser :
                deleteUser(instr[j]);
                break;
                
            case PARSE_CMD_listUsers :
                listUsers();
                break;

            default :
                instr_er = true;
        }
            
        if(instr_er)
        {
            LCD_Erase();
            sprintf((char *)LCDText, (char*)"      ERROR     "  );
            sprintf((char *)&(LCDText[16]), (char*)"      ERROR     ");
            LCD_Update();
            DELAY_ms(MESSAGE_DELAY);
            break;
        }

        DELAY_ms(MESSAGE_DELAY);
    }
    return;
}