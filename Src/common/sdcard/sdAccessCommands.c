
#include "sdAccessCommands.h"
#include <stdint.h>
#include "uart.h"
#include "spi_sdcard_display.h"
#include "stringFunctions.h"
#include "fatLib.h"
#include "consoleHandler.h"
static uint8_t card_init = 0;

DirectoryPointerType * cwd;
DirectoryPointerType * ndir;

void sdInitCommand(char * cmd,void* context)
{
    char nrbfr[4];
    uint8_t retcode;
    if (card_init == 0)
    {
        printf("\r\rInitializing SD Card..\r\nReturn Code is: ");
        retcode = initSdCard();
        if (retcode > 0)
        {
            UInt8ToChar(retcode,nrbfr);
            printf(nrbfr);
        }
        else
        {
            printf("OK (0)");
        }
        //card_init = 1;
    }
    
    printf("\r\n");
    printf("mounting first FAT32 Partition\r\n");
    retcode = initFatSDCard();
    if (retcode > 0)
    {
        UInt8ToChar(retcode,nrbfr);
        printf(nrbfr);
    }
    else
    {
        printf("OK (0)");
    }
    
}

void mountCommand(char * cmd,void* context)
{
    char nrbfr[4];
    uint8_t retcode;
    printf("\r\n");
    printf("mounting first FAT32 Partition\r\n");
    retcode = initFatSDCard();
    if (retcode > 0)
    {
        UInt8ToChar(retcode,nrbfr);
        printf(nrbfr);
    }
    else
    {
        printf("OK (0)");
    }
}

void cdCommand(char * cmd,void * context)
{
    char dirname[16];
    char nrbfr[4];
    uint8_t retcode=0;
    getBracketContent(cmd,dirname);
    switch(validateDirName(dirname))
    {
        case 0:
            retcode = openDirectory(cwd,dirname,ndir);
            if (retcode != 0)
            {
                printf("\r\nfailure opening directory: ");
                UInt8ToChar(retcode,nrbfr);
                printf(nrbfr);
                printf("\r\n");
                return;
            }
            retcode = copyDirectoryPointer(&ndir,&cwd);
            if (retcode != 0)
            {
                printf("failure copying directory pointers: ");
                UInt8ToChar(retcode,nrbfr);
                printf(nrbfr);
                printf("\r\n");
                return;
            }
            if (dirname[0]=='.' && dirname[1]=='.')
            {
                removeLastPath();
            }
            else
            {
                addToPath(dirname);
            }
            break;
        case SDACCESS_INVALID_DIRNAME:
            printf("\r\nInvalid Directory Name\r\n");
            break;
        case SDACCESS_DIRNAME_TOO_LONG:
            printf("\r\nDirectory name too long, only 8.3-Format supported\r\n");
        default:
            break;
    }
}

void lsCommand(char * cmd,void * context)
{
    char nrbfr[16];
    char displayLine[16];
    uint8_t entriesRead=0x10;
    uint16_t c=0;
    uint8_t dcnt=0;
    printf("\r\n");
    DirectoryEntryType entriesArray[16];
    DirectoryEntryType * entries = entriesArray;
    uint8_t sector[512];
    cwd->clusterPtr = cwd->dirEntry->firstCluster;
    cwd->sectorPtr=0;
    cwd->clusterCntr=0;
    while(entriesRead == 0x10)
    {
        readSector(sector,getClusterLba(cwd->clusterPtr) + cwd->sectorPtr);
        entriesRead = getDirectoryEntries(sector,&entries);
        for(c=0;c<entriesRead;c++)
        {
            if ((*(entries + c)).attrib != 0x0F && (*(entries + c)).filename[0]!=(char)0xE5)
            {
                dcnt = displayFilename(entries + c,displayLine);
                while(dcnt < 15)
                {
                    displayLine[dcnt++]=' ';
                }
                displayLine[dcnt] = 0;
                printf(displayLine);
                printf("Size: ");
                UInt32ToChar((*(entries + c)).size,nrbfr);
                printf(nrbfr);
                printf("\r\n");
            }   
        }
    } 
}

void mkdirCommand(char * cmd,void * context)
{
    char dirname[16];
    char nrbfr[4];
    uint8_t retcode=0;
    getBracketContent(cmd,dirname);
    switch(validateDirName(dirname))
    {
        case 0:
        retcode = createDirectory(cwd,dirname);
        if (retcode != 0)
        {
            printf("\r\nfailure creating directory: ");
            UInt8ToChar(retcode,nrbfr);
            printf(nrbfr);
            printf("\r\n");
        }
        break;
        case SDACCESS_INVALID_DIRNAME:
            printf("\r\nInvalid Directory Name\r\n");
            break;
        case SDACCESS_DIRNAME_TOO_LONG:
            printf("\r\nDirectory name too long, only 8.3-Format supported\r\n");
    }
}

void rmdirCommand(char * cmd,void * context)
{
    char dirname[16];
    char nrbfr[4];
    uint8_t retcode=0;
    getBracketContent(cmd,dirname);
    switch(validateDirName(dirname))
    {
        case 0:
        retcode = openDirectory(cwd,dirname,ndir);
        if (retcode != 0)
        {
            printf("\r\nfailure removing directory, getting directory pointer: ");
            UInt8ToChar(retcode,nrbfr);
            printf(nrbfr);
            printf("\r\n");
            return;
        }
        retcode = deleteDirectory(cwd,ndir);
        if (retcode != 0)
        {
            printf("\r\nfailure removing directory: ");
            UInt8ToChar(retcode,nrbfr);
            printf(nrbfr);
            printf("\r\n");
        }
        break;
        case SDACCESS_INVALID_DIRNAME:
            printf("\r\nInvalid Directory Name\r\n");
            break;
        case SDACCESS_DIRNAME_TOO_LONG:
            printf("\r\nDirectory name too long, only 8.3-Format supported\r\n");
    }
}

void rmCommand(char * cmd,void* context)
{
    char filename[16];
    char nrbfr[4];
    uint8_t retcode=0;
    FilePointerType fp;
    getBracketContent(cmd,filename);
    fp.dirEntry = 0;
    retcode = openFile(cwd,filename,&fp);
    if (retcode != 0)
    {
        printf("\r\nfailure removing file while getting file pointer: ");
        UInt8ToChar(retcode,nrbfr);
        printf(nrbfr);
        printf("\r\n");
        return;
    }
    retcode = deleteFile(cwd,&fp);
    if (retcode != 0)
    {
        printf("\r\nfailure removing file: ");
        UInt8ToChar(retcode,nrbfr);
        printf(nrbfr);
        printf("\r\n");
        return;
    }
}

void readCommand(char * cmd,void * context)
{
    char filename[16];
    char nrbfr[4];
    uint8_t retcode=0;
    FilePointerType fp;
    uint16_t bytesRead;
    char outbfr[513];
    fp.dirEntry=0;
    getBracketContent(cmd,filename);
    retcode = openFile(cwd,filename,&fp);
    if (retcode != 0)
    {
        printf("\r\nfailure opening file: ");
        UInt8ToChar(retcode,nrbfr);
        printf(nrbfr);
        printf("\r\n");
        return;
    }
    bytesRead = readFile(&fp);
    while (bytesRead > 0)
    {
        for(uint16_t c2=0;c2<bytesRead;c2++)
        {
            outbfr[c2]=(char)fp.sectorBuffer[c2];
        }
        outbfr[bytesRead]=0;
        printf(outbfr);
        bytesRead = readFile(&fp);
    }
}

uint8_t validateDirName(char *dirName)
{
    uint8_t c=0;
    if(*dirName== '.' && *(dirName + 1) == '.' && *(dirName+2) == 0) // let pass ".."
    {
        return 0;
    }
    if (*dirName== '.' && *(dirName + 1) == 0) // let pass "."
    {
        return 1;
    }
    while(*(dirName + c)!= 0)
    {
        if (*((uint8_t*)dirName + c) == '"' 
        || *((uint8_t*)dirName + c) == '*'
        || *((uint8_t*)dirName + c) == '/'
        || *((uint8_t*)dirName + c) == ':'
        || *((uint8_t*)dirName + c) == '<'
        || *((uint8_t*)dirName + c) == '>'
        || *((uint8_t*)dirName + c) == '?'
        || *((uint8_t*)dirName + c) == '\\'
        || *((uint8_t*)dirName + c) == '|'
        || *((uint8_t*)dirName + c) == '+'
        || *((uint8_t*)dirName + c) == ','
        || *((uint8_t*)dirName + c) == '.'
        || *((uint8_t*)dirName + c) == ';'
        || *((uint8_t*)dirName + c) == '='
        || *((uint8_t*)dirName + c) == '['
        || *((uint8_t*)dirName + c) == ']'
            )
        {
            return SDACCESS_INVALID_DIRNAME;
        }
        c++;
        if (c > 8)
        {
            return SDACCESS_DIRNAME_TOO_LONG;
        }
    }
    return 0;
}

uint8_t displayFilename(DirectoryEntryType * entry,char * res)
{
    uint8_t fcnt=0;
    for(uint8_t c=0;c<8;c++)
    {
        if(entry->filename[c] != ' ')
        {
            *(res+fcnt++) = entry->filename[c];
        }
    }
    if (entry->attrib == 0x20)
    {
        *(res+fcnt++) = '.';
        for(uint8_t c=0;c<3;c++)
        {
            if(entry->fileext[c] != ' ')
            {
                *(res+fcnt++) = entry->fileext[c];
            }
        }
    }
    return fcnt;
}
