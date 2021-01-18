#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include<fcntl.h> 

#define MAX_LINE 80 /* The maximum length command */

const char delim[] = " \t\r\n\v\f"; // remove these chacter

// Requirement
/**
* After reading user input, the steps are:
* (1) fork a child process using fork()
* (2) the child process will invoke execvp()
* (3) parent will invoke wait() unless command included &

- Your program should also manage basic error handling. 
  If there is no recent command in the history, entering !! should result in a message “No commands in history.”
- Your implementation will support input and output redirection, as well as pipes as a form of IPC between a pair of commands.
*/

// error:
// fixing !! when execute >

// Test case
// ls | sort
// ls | !!
// !! | sort

// functions prototype
int isExit(char** args);                        // use to check exit
int isExclaimSymbol(char** args);                     // use to check !!
int exClaimExecution(char** args, char historyArg[]);                  
int isEmpersandSymbol(char** args, int lastIndex);    // use to check empersand
int isGreatSymbol(char** args, int lastIndex);  // use to check >
int greatExecution(char** args, int lastIndex);  // execute >
int isLessSymbol(char** args, int lastIndex);   // use to check <
int lessExecution(char** args, int lastIndex);   // execute <
int isPipeSymbol(char** args, int lastIndex);   // use to check |
void pipeExecution(char** args, int lastIndex);

int main(void) {
    int size = MAX_LINE/2 + 1;      // max size
    char *args[size];               // command line arguments
    char command[size];             // the input line, being tokennize later
    char assignCommand[size];       // use to hold current command
    int should_run = 1;             // flag to determine when to exit program

    // these for history
    char historyArg[size];      // hold history string
    historyArg[0] = '\0';       // initialize history
    char updateHistory[size];   // use to update history later
    int isExclaim;              // flag to know if input is !!

    // start shell
    while (should_run) {
        
        // print shell signal
        printf("osh>");
        fflush(stdout);
        int count = 0;      // use as index

        // get input from command
        if (fgets(command, size, stdin)) {

            // keep current command, to update previous command
            strncpy(updateHistory, command, size);         

            // tokenize line
            args[count] = strtok(command, delim);

            // check if empty command
            if (args[count] == NULL) {
                continue;
            }

            // check if history
            isExclaim = isExclaimSymbol(args);
            if (isExclaim && historyArg[0] == '\0') {
                printf("No commands in history.\n");
                continue;
            }

            // update history, except exclaim symbol
            if (!isExclaim) {
                strncpy(historyArg, updateHistory, size);
            }

            // update current command
            if (isExclaim) {                               
                strncpy(assignCommand, historyArg, size);
                args[count] = strtok(assignCommand, delim);   
            }

            // set last null
            while (args[count] != NULL) {
                args[++count] = strtok(NULL, delim);
            }
        }

        // check if exit
        if (isExit(args))
            return 0;

        // check if parent should wait or not
		int isEmpersand = isEmpersandSymbol(args, count);
        
        // remove empersand
        if (isEmpersand) {
            args[--count] = NULL;
        }        

        // check > symbol
        int isGreat = isGreatSymbol(args, count);

        // check < symbol
        int isLess = isLessSymbol(args, count);

        // check | symbol
        int isPipe = isPipeSymbol(args, count);

        // fork
        int pid = fork();

        // faild fork
        if (pid < 0) {
            fprintf(stderr, "Fork Failed");
            exit(EXIT_FAILURE);
            return 0;
        }

        // child proces
        else if (pid == 0) {

            // use to check error         
            int c;

            // execute pipe | command
            if (isPipe) {                
                pipeExecution(args, count);
                return 0;
            }

            // execute redirection > command
            if (isGreat) {
                c = greatExecution(args, count);
            }

            // execute redirection < command
            if (isLess) {
                c = lessExecution(args, count);
            }
            
            // execute history !! command
            if (isExclaim) {
                if (!isGreat) {
                    c = exClaimExecution(args, historyArg);
                }
                else {
                    printf("%s", historyArg);
                }
            }
            
            // execute normal command, not predefined command
            if (!isPipe && !isLess && !isGreat && !isExclaim) {
                c = execvp(args[0], args);
            }

            // error command
            if (c < 0) {
                printf("%s: command not found\n", args[0]);
            }

            // child return back to parent
            return 0;
        }

        // parent process
        else {
            
            // if not &, wait
			if (!isEmpersand) {            
				wait(NULL);
			}        
        }        
      }
    return 0;
}

int isExit(char** args) {
    if (strcmp(args[0], "exit") == 0
    || strcmp(args[0], "Exit") == 0
    || strcmp(args[0], "EXIT") == 0) {
        return 1;
    }
    return 0;
}

int isExclaimSymbol(char** args) {
    if (strcmp(args[0], "!!") == 0)
        return 1;
    return 0;
}

int exClaimExecution(char** args, char historyArg[]) {
    printf("%s", historyArg);
    int c = execvp(args[0], args);
    return c;
}

int isEmpersandSymbol(char** args, int lastIndex) {
    int flag = 0;
    if (strcmp(args[lastIndex - 1], "&") == 0) {
	    flag = 1;
	}
    return flag;
}

int isGreatSymbol(char** args, int lastIndex) {
	int flag = 0;
	for (int i = 0; i < lastIndex; i++) {
  		if (strcmp(args[i], ">") == 0) {
  			flag = 1;
  		}
	}
    return flag;
}

int greatExecution(char** args, int lastIndex) {
    
    // get file name
    int flag = 1;
    char fileName[MAX_LINE/2 + 1];
    int i = 0;
    for (i; i < lastIndex && flag == 1; i++) {
        if (strcmp(args[i], ">") == 0 && i < lastIndex - 1) {
            strcpy(fileName, args[i + 1]);
            flag = 0;
        }
    }

    // assign argument without > and file name
    args[i--] = NULL;
    args[i] = NULL;              

    // use to check bad command, by using pipe to communicate
    int fds[2];
    int pipeId = pipe(fds);

    // fail to open pipe
    if (pipeId == -1) { 
        return 1;
    }

    // sub fork
    int tempF = fork();
    int c;

    // child
    if (tempF == 0) {

        // open file
        FILE *usedFile;
        usedFile = fopen(fileName, "w"); // for write
        if (usedFile == NULL) {
            printf("cannot access '%s' file", fileName);
        }
                        
        // run descriptor
        dup2(fileno(usedFile), STDOUT_FILENO);

        // close files
        fclose(usedFile);

        // execute code 
        c = execvp(args[0], args);
                                
        // transfer data from child to current parent
        close(fds[0]);
        if (write(fds[1], &c, sizeof(c)) == -1) {
            return 1; // something bad happend
        }
        close(fds[1]);
        exit(1); 
    }

    // get return command line from child
    else {
        wait(NULL);
        int cFromChild;
        close(fds[1]);
        if (read(fds[0], &cFromChild, sizeof(cFromChild)) == -1) {
            return 1; // something bad happend
        }
        c = cFromChild;
    }
    return c;
}

int isLessSymbol(char** args, int lastIndex) {
	int flag = 0;
	for (int i = 0; i < lastIndex; i++) {
  		if (strcmp(args[i], "<") == 0) {
  			flag = 1;
  		}
	}
    return flag;
}

int lessExecution(char** args, int lastIndex) {

    // get file name
    int flag = 1;
    char fileName[MAX_LINE/2 + 1];
    int i = 0;
    for (i; i < lastIndex && flag == 1; i++) {
        if (strcmp(args[i], "<") == 0 && i < lastIndex - 1) {
            strcpy(fileName, args[i + 1]);
            flag = 0;
        }
    }

    // assign argument without > and file name
    args[i--] = NULL;
    args[i] = NULL;  

    int c; // check bad command

    // open file
    FILE *usedFile;
    usedFile = fopen(fileName, "r"); // for read
    if (usedFile == NULL) {
        printf("cannot access '%s' file", fileName);
    }

    // run descriptor
    dup2(fileno(usedFile), STDIN_FILENO);

    // close files
    fclose(usedFile);

    // execute code 
    c = execvp(args[0], args);
    return c;
}

int isPipeSymbol(char** args, int lastIndex) {
    int flag = 0;
    for (int i = 0; i < lastIndex && flag == 0; i++) {
        if (strcmp(args[i], "|") == 0) {
            flag = 1;
        }
    }
    return flag;
}

void pipeExecution(char** args, int lastIndex) {
    char *leftCommand[MAX_LINE/2 + 1];  // left argument
    char *rightCommand[MAX_LINE/2 + 1]; // right argument

    // check pivot
    int pivot = 0;
    for (int i = 0; i < lastIndex && pivot == 0; i++) {        
        if (strcmp(args[i], "|") == 0) {
            pivot = i;
        }        
    }
    
    // assign to left command
    int leftSize = 0;
    for(int i = 0; i < pivot; i++) {
        leftCommand[leftSize++] = args[i];
    }
    leftCommand[leftSize] = NULL;    
    
    // assign to right command
    int rightSize = 0;
    for (int i = pivot + 1; i < lastIndex; i++) {
        rightCommand[rightSize++] = args[i];
    }
    rightCommand[rightSize] = NULL;
      
    // check error left
    if (leftCommand[0] == NULL) {
        printf("bash: syntax error near unexpected token `|'\n");
    }
    
    // check error right
    int size = MAX_LINE/2 + 1;
    char command[size];
    int count = 0;
    
    /*
    while (rightCommand[0] == NULL) {
        printf(">");
        fgets(command, size, stdin);

        // tokenize line
        rightCommand[count] = strtok(command, delim);

        // set last null
        while (rightCommand[count] != NULL) {
            rightCommand[++count] = strtok(NULL, delim);
        }
    }
    */
    
    // process
    int pfd[2]; // open read/write pipe fd
	pipe(pfd);    // pipe 

	if (fork() == 0) {      
        printf("Execute left %s\n", leftCommand[0]);
        for(int i = 0; i < pivot; i++) {
            printf("%s ", leftCommand[i]);
        }
        printf("\n");

        /*
        // close stdout
        close(STDOUT_FILENO);

        // stdout to pipe for write
        dup2(STDIN_FILENO, 1);

        // close pipe write
        close(pfd[1]);

        // execute command
        execvp(leftCommand[0], leftCommand);
        */
	}

	if (fork() == 0) {
        printf("Execute right %s\n", rightCommand[0]);
        for (int i = 0; i < rightSize; i++) {
            printf("%s ", rightCommand[i]);
        }
        printf("\n");

        /*
        // close stdin
		close(STDIN_FILENO);

        // stdin to pipe for read
		dup2(STDOUT_FILENO, 0);

        // close pipe read
		close(pfd[0]);

        // execute command
	    execvp(rightCommand[0], rightCommand);
        */
	}
	wait(NULL);	// wait 
}
