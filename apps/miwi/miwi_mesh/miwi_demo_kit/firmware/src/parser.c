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

void addUser(char * instr)
{
    char * token = strtok(instr, " ");
    char userName[16];
    char password[16];
    uint8_t stage = 0;
    bool error_flag = false;
    
    LCD_Erase();
    sprintf((char *)LCDText, (char*)"%16s", instr);
    sprintf((char *)&(LCDText[16]), (char*)"ADD USER BEGIN  ");
    LCD_Update();  
    
    DELAY_ms(5000);
    
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
        sprintf((char *)&(LCDText[16]), (char*)password);
        LCD_Update();   
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
    }else{
        num = PARSE_CMD_error;
    }
    /*
    LCD_Erase();
    sprintf((char *)LCDText, "%16s", token );
    sprintf((char *)&(LCDText[16]), "        %01d       ", num);
    LCD_Update();
    */
    
    DELAY_ms(5000);
    
    return num;    
}

void executeCommands()
{
    char input[150] = "<config-begin>\n<config-addUser> ckreid asdf1234\n<config-master> ckreid asdf1234\n<config-end>";
    char * instr[10];
    uint8_t instr_num = 0;
    bool instr_er = false;
    uint8_t i = 0;
    uint8_t j = 0;
    char * token;
    char * userName;
    char * password;
    uint8_t stage = 0;
    
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

            default :
                instr_er = true;
        }
            
        if(instr_er)
        {
            LCD_Erase();
            sprintf((char *)LCDText, (char*)"      ERROR     "  );
            sprintf((char *)&(LCDText[16]), (char*)"      ERROR     ");
            LCD_Update();
            DELAY_ms(5000);
            break;
        }

        DELAY_ms(5000);
    }
    return;
}


void strtokTest()
{
    char str[80] = "claytonreid1234 testestestestes";
    char * token;
    
    token = strtok(str, " ");
    
    while(token != NULL)
    {
        LCD_Erase();
        sprintf((char *)LCDText, "%15s", token  );
        sprintf((char *)&(LCDText[16]), (char*)"STRTOK TEST    ");
        LCD_Update();
        DELAY_ms(5000);
        
        token = strtok(NULL, " ");   
    }
    
}
