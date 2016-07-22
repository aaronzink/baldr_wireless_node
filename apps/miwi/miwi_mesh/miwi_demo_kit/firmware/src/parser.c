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
#define MESSAGE_DELAY 10000

struct user_s
{
    char userName[16];
    char phoneNumber[12];
    bool master;
};

char defaultPassword[20] = "asdf1234";
struct user_s currentUsers[USER_LIMIT];
uint8_t userCount = 0;

void memoryTest()
{
    uint8_t testValue[5] = {1,2,3,4,5};
    uint8_t address = 5000;
    uint8_t count = (uint8_t)sizeof(testValue);
    uint8_t * testOutput[5];
    
    SSTWrite(&testValue, address, count);
    SSTRead(&testOutput, address, count);
    
    LCD_Erase();
    sprintf((char *)LCDText, "%03d,%03d,%03d,%03d",testOutput[0],testOutput[1],testOutput[2],testOutput[3]);
    sprintf((char *)&(LCDText[16]), (char*)"Memory Test     ");
    LCD_Update();
    
    DELAY_ms(10000);
    
}

void addUser(char * instr)
{
    char * token = strtok(instr, " ");
    char userName[16];
    char password[16];
    uint8_t stage = 0;
    bool error_flag = false;
    struct user_s newUser;
    
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
            strcpy(newUser.userName, userName);
            newUser.master = false;
            LCD_Erase();
            sprintf((char *)LCDText, (char*)newUser.userName);
            sprintf((char *)&(LCDText[16]), (char*)"Added as user   ");
            LCD_Update();
            currentUsers[userCount] = newUser;
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
        uint8_t counter = 0;
        bool exists = false;
        for(int i = 0; i<userCount; i++)
        {
            if(strcmp(currentUsers[i].userName,userName) == 0)
            {
                exists = true;
                break;
            }
            counter++;
        }
        
        if(exists)
        {
            currentUsers[counter].master = true;
            LCD_Erase();
            sprintf((char *)LCDText, (char*)currentUsers[counter].userName);
            sprintf((char *)&(LCDText[16]), (char*)"   Made Master  ");
            LCD_Update();
        }else{
            LCD_Erase();
            sprintf((char *)LCDText, (char*)userName);
            sprintf((char *)&(LCDText[16]), (char*)"Is not a user   ");
            LCD_Update();
        }
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
        sprintf((char *)LCDText, (char*)currentUsers[i].userName);
        if(currentUsers[i].master)
        {
            sprintf((char *)&(LCDText[16]), (char*)"MASTER         ");

        }else{
            sprintf((char *)&(LCDText[16]), (char*)"               ");

        }
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
    }else if(strcmp(token,"<config-deleteUser>") == 0){
        num = PARSE_CMD_deleteUser;
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