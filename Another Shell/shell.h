#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdlib.h>

#include "token.h"
#include "myVector.h"

#define commandCapacity 1000
#define promptCapacity 1000
#define characterCapacity 2000

typedef enum
{
    fg,
    sp,
    bg
} JobState;

typedef struct
{
    int fd[2];
} Pipe;

typedef struct
{
    Tokenizer *tok;     // tokenizer
    Pipe *inPipe;
    Pipe *outPipe;
} Command;

typedef struct
{
    pid_t pid;
    JobState state;
    char *command;
} Job;

typedef struct
{
    char *cwd;
    char *prompt;
    char *line;

    VVector *jobTable;
    VVector *suspStack;
    Job *active;
} Shell;

void shellConstructor(Shell *shellConstr);    //shell constructor
void shellDestructor(Shell *shellDestr);    //shell destructor

Shell * newShell(void); //function to create new shell object
void setShellPrompt(Shell *setShell, const char *tokenPrompts); //initialize the shell for prompts

void commandConstructor(Command *commandConstructor, const char *tokens);   //command constructor
void commandDestructor(Command *commandDestructor); //command destructor
Command *newCommand(const char *input); //function to create a new commands
void jobConstructor(Job *jobConstructor, const char * tokens, pid_t pid, JobState stateStatus); //job constructor
void jobDestructor(Job *destroyJob); //job destructor
Job* newJob(const char * inputs, pid_t pid, JobState stateStatus);  //functioin to create job
void deleteJob(Job *jobDelete); //delete job

#endif //__SHELL_H__
