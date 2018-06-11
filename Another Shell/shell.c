#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "token.h"
#include "myVector.h"

void shellConstructor(Shell *shellConstr){    //shell constructor
    //initialize all of the shell variables
    (*shellConstr).cwd = malloc(commandCapacity * sizeof(char) + 1);
    (*shellConstr).cwd[0] = '\0';

    (*shellConstr).prompt = malloc(promptCapacity * sizeof(char) + 1);
    (*shellConstr).prompt[0] = '\0';

    (*shellConstr).line = malloc(characterCapacity * sizeof(char) + 1);
    (*shellConstr).line[0] = '\0';

    (*shellConstr).jobTable = VVector_new_reg(1, &deleteJob);
    (*shellConstr).suspStack = VVector_new( 1 );

    (*shellConstr).active = NULL;
}

void shellDestructor(Shell *shellDestr){    //shell destructor
    free((*shellDestr).cwd);
    (*shellDestr).cwd = NULL;

    free((*shellDestr).prompt);
    (*shellDestr).prompt = NULL;

    free((*shellDestr).line);
    (*shellDestr).line = NULL;

    VVector_deleteFull((*shellDestr).jobTable);
    (*shellDestr).jobTable = NULL;

    VVector_delete((*shellDestr).suspStack);
    (*shellDestr).suspStack = NULL;

    (*shellDestr).active = NULL;
}

Shell * newShell(void){    //function to create new shell object
    Shell *shellOut = malloc(sizeof(Shell));    //allocate size for shell
    shellConstructor(shellOut); //call constructor
    return shellOut;    //return the new shell
}

void setShellPrompt(Shell *setShell, const char *tokenPrompts){//initialize the shell for prompts
    strcpy((*setShell).prompt, tokenPrompts);
}

void commandConstructor(Command *commandConstructor, const char *tokens){   //command constructor
    (*commandConstructor).tok = Tokenizer_new(tokens, " "); //initialize the command
    (*commandConstructor).inPipe = (*commandConstructor).outPipe = NULL;    //set the in as NULL
}

void commandDestructor(Command *destructorCommand){ //command destructor
    Tokenizer_delete((*destructorCommand).tok);    //delete all commands and tokens
    (*destructorCommand).inPipe = (*destructorCommand).outPipe = NULL;  //set the pointer to the object as null
}

Command *newCommand(const char *input){ //function to create a new command
    Command *allocateSpace = malloc(sizeof(Command));  //set aside memory for command
    commandConstructor(allocateSpace, input);  //create the command
    return allocateSpace;  //return it
}

void jobConstructor(Job *jobConstructor, const char * tokens, pid_t pid, JobState stateStatus){ //job constructor
    (*jobConstructor).pid = pid;    //set the pid of job
    (*jobConstructor).state = stateStatus;  //set state of job
    (*jobConstructor).command = strdup(tokens); //set the command of job
}

void jobDestructor(Job *destroyJob){ //job destructor
    free((*destroyJob).command); //remove the job from memory
}

Job* newJob(const char * inputs, pid_t pid, JobState stateStatus){  //functioin to create job
    Job * jobRet = malloc(sizeof(Job));  //allocate memory for job
    jobConstructor(jobRet, inputs, pid, stateStatus);   //call constructor for job
    return jobRet;  //return job object
}

void deleteJob(Job *jobDelete){ //delete job
    jobDestructor(jobDelete);   //call destructor on the job
    free(jobDelete);    //free memory from deleted job
}

