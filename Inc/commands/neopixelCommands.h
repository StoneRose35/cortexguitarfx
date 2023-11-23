#ifndef _NEOPIXEL_COMMANDS_H_
#define NEOPIXEL_COMMANDS_H_

#define N_COLOR_COMMANDS 16 //!< the number of commands which set all neopixels to the same color
#define TASK_STATE_STOPPED 0 //!< enum value: the command is stopped
#define TASK_STATE_RUNNING 1 //!< enum value: the command is running
#define TASK_STATE_PAUSED 2 //!< enum value: the command is paused


void colorCommand(char * cmd,void * context);
void rgbCommand(char* cmd,void* context);
void startCommand(char * cmd,void* context);
void stopCommand(char * cmd,void* context);
void printColorCommands();
void interpCommand(char * cmd,void* context);
void istepCommand(char * cmd,void* context);
void desciCommand(char * cmd,void* context);
void destroyCommand(char * cmd,void* context);
void saveCommand(char * cmd,void* context);
void loadCommand(char * cmd,void* context);
void npEngineCommand(char * cmd,void*context);
#endif