#ifndef _SD_ACCESS_COMMANDS_H_
#define _SD_ACCESS_COMMANDS_H_
#include <stdint.h>
#include "fatLib.h"

void sdInitCommand(char * cmd,void* context);
void mountCommand(char * cmd,void* context);
void cdCommand(char * cmd,void * context);
void lsCommand(char * cmd,void * context);
void mkdirCommand(char * cmd,void * context);
void rmdirCommand(char * cmd,void * context);
void rmCommand(char * cmd,void * context);
void readCommand(char * cmd,void * context);

uint8_t validateDirName(char *dirName);
uint8_t displayFilename(DirectoryEntryType * entry,char * res);

//
// ERROR_CODES
//

#define SDACCESS_INVALID_DIRNAME 2
#define SDACCESS_DIRNAME_TOO_LONG 3
#endif