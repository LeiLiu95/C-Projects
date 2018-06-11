#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "shell.h"
#include "token.h"
#include "myVector.h"
#include "debug.h"

//two global variables of PID and shell will be used
pid_t pid;
Shell *shell;

//global define
#define MODES (S_IWUSR | S_IROTH | S_IRUSR | S_IRGRP)

void printReadyYash(Shell *shellPrompt){    //first command in yash, to take input from the console
    printf("%s", (*shellPrompt).prompt);
}

int myReplaceFileCmd(int indexClose, const char* file, int flagsCheck, mode_t writeMode){
    int currentStatus = 0;
    currentStatus = close(indexClose);
    if(currentStatus != 0){ //if status is not 0 then return 0 for done
        return 0;
    }
    else{//else open file
        int fileDirectory = open(file, flagsCheck, writeMode);
        if(fileDirectory < 0){  //if file is -1 then return 0
            return 0;
        }
    }
    return 1;   //if no case fails then return 1
}

void readInput(Shell *shellRead){
    size_t index = 0;
    char* bufferAmount = (*shellRead).line;
    char command;
    while(1){
        command = getchar();

        if(command == EOF){ //if nothing then exit the process
            exit(0);
        }
        else if(command == '\n'){   //if new line then add buffer to the end and return
            bufferAmount[index] = '\0';
            return;
        }
        else{   //add the command or args to the array
            bufferAmount[index] = command;
        }
        index+=1;   //increment the index
    }   
}

int tokenParser(Command *command, char **argumentList[]){
    const char * tokenPointer;
    Tokenizer *token = (*command).tok;
    VVector *argumentsPointer = VVector_new(1);
    while(Tokenizer_hasTokens(token)){  //loop to iterate through tokens
        tokenPointer = Tokenizer_next(token);   //grabs the next token to evaluate
        if(!strcmp(tokenPointer, "<")){ //if the "<" command is read then stdin function is done
            if(!Tokenizer_next(token)){    //check to see if there is a next arg
                return 0;
            }
            else{
                myReplaceFileCmd(STDIN_FILENO, Tokenizer_next(token), O_RDONLY, MODES);
            }
        }
        else if (!strcmp(tokenPointer, "2>")){  //if the "2>" command is read then the stderr is done
            if(!Tokenizer_next(token)){    //check to see if there is a next arg
                return 0;
            }
            else{
                myReplaceFileCmd(STDERR_FILENO, Tokenizer_next(token), O_CREAT | O_WRONLY, MODES);
            }
        }
        else if(!strcmp(tokenPointer, ">" )){   //if the ">" command is read then the stdout function is done
            if(!Tokenizer_next(token)){    //check to see if there is a next arg
                return 0;
            }
            else{
                myReplaceFileCmd(STDOUT_FILENO, Tokenizer_next(token), O_CREAT | O_WRONLY, MODES);
            }
        }
        else{   //if none of the commands were found then add files
            VVector_push(argumentsPointer, tokenPointer);
        }
    }
    int argCommand = VVector_length(argumentsPointer);    //setup argument lists
    char ** argVariables = malloc(sizeof(char *) * (argCommand + 1) );       //keep size at 2000 plus one in case for 2000 characters + NULL
    for(int i=0; i < argCommand; i+=1){ //loop to iterate through and add tokens
        argVariables[i] = VVector_get(argumentsPointer, i);    // set up the arguments
    }
    argVariables[argCommand] = NULL;    //final NULL statement for the end of line, and if nothing was inputed then a Null will be put at the beggining of the argList
    VVector_delete(argumentsPointer);   //cleanup function
    *argumentList = argVariables;
    return 1;
}

void initializePipes(Command *command){ //function to initialize pipes for read and write
    Pipe *readPipe = (*command).outPipe;  //create the output pipe
    Pipe *writePipe = (*command).inPipe;    //create the input pipe

    if(writePipe){  //initialize inputPipe
        close((*writePipe).fd[1]);               // write end of the pipe is closed off
        dup2((*writePipe).fd[0], STDIN_FILENO);  // use dup for stdin
    }
    if(readPipe){ //initialize outputPipe
        close((*readPipe).fd[0]);                  // read end of the pip is closed off
        dup2((*readPipe).fd[1], STDOUT_FILENO);    // use dup for stdout
    }
}

void closeAllPipes(Pipe *pip, int numberOfPipes){ //function to reset pipes
    for(int i=0; i<numberOfPipes; i+=1){   //use a for loop to iterate through and close off all pipes
        close(pip[i].fd[0]);  //close read end of pipe
        close(pip[i].fd[1]);  //close write end of pipe
    }
}

void myExecFunc(Command *command, Pipe *pip, int numberOfPipes){
    char **argumentList;
    if(!tokenParser(command, &argumentList)){   //only if child failed will this if ever pass
        printf("Child failed to setup exec!\n");    //output failure
    }
    if(pip){    //if pip is null then create pip then close it
        initializePipes(command);
        closeAllPipes(pip, numberOfPipes);
    }
    execvp(argumentList[0], argumentList); //execute the command with the following arguments and replace the current process with the new function
    free(argumentList);    //remove all data from the array
    exit(0);   //successfully exit the process when finished
}

pid_t forkListFunc(Command *command, Pipe *pip, int numberOfPipes){ //function to run through process with pipe information
    pid_t cpid;
    cpid = fork();
    if(cpid == 0){  //if cpid is 0 then child process runs
       myExecFunc(command, pip, numberOfPipes);
    }
    return cpid;    //return process id
}

pid_t forkGroupExecFunc(Command **command, int numberOfCommands){
    pid_t cpid;
    cpid = fork();
    if(cpid == 0){  //child process check
        setpgid(0,0); // set
        Pipe *pipePointer = malloc( sizeof(Pipe) * (numberOfCommands-1) );
        for(int i=0; i < numberOfCommands-1; i+=1){ //first loop to iterate through to grab all pipes needed
            pipe(pipePointer[i].fd);
        }
        (*command[0]).inPipe = NULL;             // initialize the first step of pipes
        (*command[0]).outPipe = &pipePointer[0];    //setup pipe
        for(int i=1; i < numberOfCommands-1; i+=1){ //excluding the first command, iterate through other args to grab pipes
            (*command[i]).inPipe = &pipePointer[i-1];   //get the write and read side of the pipes
            (*command[i]).outPipe = &pipePointer[i];
        }
        (*command[numberOfCommands-1]).inPipe = &pipePointer[numberOfCommands-2];  // initialize second step of pipes and setup write and read of pipe
        (*command[numberOfCommands-1]).outPipe = NULL;

        for(int i=1; i < numberOfCommands; i+=1){ //for loop to fork all needed child processes
            pid_t cpid; //setup child process id
            cpid = fork();  //create child process
            if(cpid == 0){
                myExecFunc(command[i], pipePointer, numberOfCommands-1);
            }
            //forkListFunc( cmds[i], pipes, numberOfCommands-1 );
        }
        myExecFunc(command[0], pipePointer, numberOfCommands-1);    //run the final command for process
    }
    return cpid;
}

void addNewJob(Shell* passedShell, Job* passedJob){ //new job function
    VVector_push((*passedShell).jobTable, passedJob);
}
void removeJob(Shell* passedShell, Job* passedJob){ //remove job function
    VVector_remove( (*passedShell).jobTable, passedJob);
}
void pushSuFunc(Shell* passedShell, Job* passedJob){    //push function onto vector stack
    VVector_push((*passedShell).suspStack, passedJob);
}
void popSuFunc(Shell* passedShell, Job** jobHandlerVar){    //pop function off of vector stack
    *jobHandlerVar = VVector_pop((*passedShell).suspStack);
}

void waitTilActiveProcess(Shell *shellWait){  //function to wait for active process
    int waitingStatusVar;
    int waitingPid;
    if((*shellWait).active){    //if shell is active then run process
        dprintf("Waiting on %d \"%s\"\n", (*shellWait).active->pid, (*shellWait).active->command);  //output if shell is waiting
        waitingPid = waitpid((*shellWait).active->pid, &waitingStatusVar, WUNTRACED | WCONTINUED ); //set the shell to be waiting
        if(WIFCONTINUED(waitingStatusVar)){ //output the process has been continued
            dprintf("%d continued\n", waitingPid);
        }
        else if(WIFEXITED(waitingStatusVar)){   //output the process has been exited
            dprintf("%d exited\n", waitingPid);
            removeJob(shellWait, (*shellWait).active);
            (*shellWait).active = NULL;
        }
        else if(WIFSTOPPED(waitingStatusVar)){  //output the shell has been stopped
            dprintf("%d stopped\n", waitingPid);
            (*shellWait).active->state = sp;              // suspend it
            pushSuFunc(shellWait, (*shellWait).active);  // push it onto the susp stack
            (*shellWait).active = NULL;
        }
    }
}

int parsingProcesses(Shell* shellParsing){
    if(!strcmp((*shellParsing).line, "bg")){    //if bg is the command then nothing needs to be done
        return 1;
    }
    if(!strcmp((*shellParsing).line, "fg")){    //check if the string is fg
        popSuFunc(shellParsing, &(*shellParsing).active);   //if it it then pop the process and do it
        if((*shellParsing).active){ //if shell is active then print the function and run it
            (*shellParsing).active->state = fg; //set the active state
            printf("fg %d\n", (*shellParsing).active->pid); //print out the active process pid
            kill((*shellParsing).active->pid, SIGCONT); //end the command
            waitTilActiveProcess(shellParsing);
        }
        else{
            printf("No suspended jobs\n");  //if none then print no jobs
        }
        return 1;
    }
    return 0;
}

void parseLine(Shell* shellParse){    //function to parse the lines
    if(parsingProcesses(shellParse)){   //if no tokens to parse then return
        return;
    }
    Tokenizer* parseToken = Tokenizer_new((*shellParse).line, "|");    //split pipe commands
    int numberOfCommands = Tokenizer_numTokens(parseToken);           // get the number of tasks
    JobState parseState = strchr((*shellParse).line,'&')? bg: fg;   // figure this out from presence of &
    pid_t cpid; //make a child process id data element
    if(numberOfCommands > 1){   //if more than 1 command then enter this case
        Command **commands = malloc( sizeof(Command *) * numberOfCommands );    //set size of 2000
        for(int i=0; i < numberOfCommands; i+=1){   //iterate through all commands and parse
            commands[i] = newCommand(Tokenizer_next(parseToken)); //add tokens to array
        }
        cpid = forkGroupExecFunc(commands, numberOfCommands);   //fork all commands in a group
        waitpid(-cpid);
        for(int i=0; i < numberOfCommands; i+=1){   //iterate through the number of commands
            free(commands[i]);  //free all the tokens from the array
        }
        free(commands); //free the last null
    }
    else if(numberOfCommands == 1){ //if only 1 command then enter this command
        Command command;    //new command
        Tokenizer *commandToken = Tokenizer_new(Tokenizer_next(parseToken), "&");   //parse command for &
        commandConstructor(&command, Tokenizer_next(commandToken));
        cpid = forkListFunc(&command, NULL, 0 );    //cpid gets a new fork function
        Job *childJob = newJob((*shellParse).line, cpid, parseState);  //create new job and add it
        addNewJob(shellParse, childJob);
        if(parseState == bg){   //if bg then nothing needs to be done
            printf("bg %d\n", (*childJob).pid);
        }
        else if(parseState == fg){  //if fg thebgn move process
            (*shellParse).active = childJob;
            waitpid((*childJob).pid);
        }
        commandDestructor(&command);
        Tokenizer_delete(commandToken);
    }
    Tokenizer_delete(parseToken);
}

void sigTerminate(int sigNumber){
    printReadyYash(shell);
    if(!(*shell).active){   //if it is inactive then return, no reason to end it
        return;
    }
    else{   //else end the process
        kill((*shell).active->pid, SIGTSTP);
    }
}

void sigIntTerminate(int sigNumber){
    printReadyYash(shell);
    if(!(*shell).active){   //if it is inactive then return, no reason to end it
        return;
    }
    else{   //else end the process
        kill((*shell).active->pid, SIGINT);
    }
}

int main(int argc, char *argv[]){
    Shell *yashShell = newShell();  //setup a new shell process
    setShellPrompt(yashShell, "# ");

    pid = getpid(); //initialize global variables
    shell = yashShell;

    signal(SIGINT, sigIntTerminate);    //initialize signals    
    signal(SIGTSTP, sigTerminate);

    while(1){   //loop to read and parse input until exited
        printReadyYash(yashShell);
        readInput(yashShell);
        parseLine(yashShell);
        waitTilActiveProcess(yashShell);
    }
}
