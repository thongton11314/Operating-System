#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <errno.h>

#define MAX_LINE 80 /* The maximum length command */
const char delim[] = " \t\r\n\v\f"; // remove these chacter

// functions prototype
int isExit(char** args);                              // use to check exit
int isExclaimSymbol(char** args);                     // use to check !!
int isEmpersandSymbol(char** args);    // use to check empersand
int isGreatSymbol(char** args);        // use to check >
int executeDirectIn(char** args);      // execute >, re-direct in
int isLessSymbol(char** args);         // use to check <
int executeDirectOut(char** args);     // execute <, re-direct out
int isPipeSymbol(char** args);         // use to check |
void executePipe(char** args);         // execute |

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

    // pre-define command
    int isExclaim;              // flag to know if input has !!
    int isGreat;                // flag to know if input has >
    int isLess;                 // flag to know if input has <
    int isPipe;                 // flag to know if input has |
    int isEmpersand;            // flag to know if input has &
    
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
                fflush(stdout);
                continue;
            }

            // update history, except exclaim(!!) symbol
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

            // check if exit
            if (isExit(args))
                return 0;

            // check if parent should wait or not
            isEmpersand = isEmpersandSymbol(args);
            
            // remove empersand
            if (isEmpersand) {
                args[--count] = NULL;
            }
        }

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

            // execute !!
            if (isExclaim) {
                printf("%s", historyArg);
                fflush(stdout);
            }

            // execute pipe | command
            if (isPipeSymbol(args)) {
                executePipe(args);
                // seperate of two commands come out after envoke executePipe
                // command start run after this block of code
            }

            // execute redirection > command
            if (isGreatSymbol(args)) {
                c = executeDirectIn(args);
            }

            // execute redirection < command
            else if (isLessSymbol(args)) {
                c = executeDirectOut(args);
            }
            
            // normal command, maybe Unix, Linux command
            else {
                c = execvp(args[0], args);
            }

            // error command
            if (c < 0) {
                printf("%s: command not found\n", args[0]);
                fflush(stdout);
            }

            // child return back to parent
            exit(0);
        }

        // parent process
        else {
            
            // if not &, wait
			if (!isEmpersand) {               
				waitpid(pid, NULL, 0); // make parent wait
			}            
        }
    }
    return 0;
}

// -------------------------------- isExit ------------------------------------
// use to terminate the shell if user input exit
int isExit(char** args) {
    if (strcmp(args[0], "exit") == 0
    || strcmp(args[0], "Exit") == 0
    || strcmp(args[0], "EXIT") == 0) {
        return 1;
    }
    return 0;
}

// ----------------------------- isExclaimSymbol ------------------------------
// use to check if input from shell contain !! symbol
int isExclaimSymbol(char** args) {
    if (strcmp(args[0], "!!") == 0)
        return 1;
    return 0;
}

// ---------------------------- isEmpersandSymbol -----------------------------
// use to check if input from shell contain & symbol
int isEmpersandSymbol(char** args) {
    int flag = 0;
    int i = 0;    
    while (args[i] != NULL) {
        i++;
    }
    if (strcmp(args[i - 1], "&") == 0) {
	    flag = 1;
	}
    return flag;
}

// ------------------------------ isGreatSymbol -------------------------------
// use to check if input from shell contain > symbol
int isGreatSymbol(char** args) {
	int flag = 0;
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
  			flag = 1;
  		}
        i++;
    }
    return flag;
}

// ------------------------------ executeDirectIn -----------------------------
// execute command re-direct in
int executeDirectIn(char** args) {
    
    // get last index
    int lastIndex = 0;
    while (args[lastIndex] != 0) {
        lastIndex++;
    }

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
        FILE *aFile;
        aFile = fopen(fileName, "w"); // for write

        // run descriptor
        dup2(fileno(aFile), STDOUT_FILENO);

        // close files
        fclose(aFile);

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

// ------------------------------- isLessSymbol -------------------------------
// use to check if input from shell contain > symbol
int isLessSymbol(char** args) {
	int flag = 0;
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "<") == 0) {
  			flag = 1;
  		}
        i++;
    }
    return flag;
}

// ------------------------------ executeDirectOut ----------------------------
// execute command re-direct out
int executeDirectOut(char** args) {

    // get last index
    int lastIndex = 0;
    while (args[lastIndex] != 0) {
        lastIndex++;
    }

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
    FILE *aFile = NULL;
    aFile = fopen(fileName, "r"); // for read
    if (aFile == NULL) {
        perror(fileName);
    }

    // run descriptor
    dup2(fileno(aFile), STDIN_FILENO);

    // close files
    fclose(aFile);

    // execute code 
    c = execvp(args[0], args);
    return c;
}

// ------------------------------- isPipeSymbol -------------------------------
// use to check if input from shell contain | symbol
int isPipeSymbol(char** args) {

    // get last index
    int lastIndex = 0;
    while (args[lastIndex] != 0) {
        lastIndex++;
    }

    // check if pipeline
    int flag = 0;
    for (int i = 0; i < lastIndex && flag == 0; i++) {
        if (strcmp(args[i], "|") == 0) {
            flag = 1;
        }
    }
    return flag;
}

// ------------------------------ pipeExecution -------------------------------
// execute command pipeline
void executePipe(char** args) {

    // get last index
    int lastIndex = 0;
    while (args[lastIndex] != 0) {
        lastIndex++;
    }

    // arguments
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

    // check error right
    int size = MAX_LINE/2 + 1;
    char command[size];
    int count = 0;

    // if right side is not provide, keep asking
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

    // use to check bad command, by using pipe to communicate
    int leftCheck[2];
    pipe(leftCheck);
    
    // process argumen1, argument2
    int pfd[2];                // open read/write pipe fd
	int pipeId = pipe(pfd);    // pipe 

    if (pipeId < 0) {
        printf("Error opening pipe.\n");
        exit(EXIT_FAILURE);
    }
    
    int pid1 = fork();

    // error open new process
    if (pid1 < 0) {
        printf("Error opening new process.\n");
    }

    // chill process, execute first command
	else if (pid1 == 0) {    

        // assign to left command
        for(int i = 0; i < leftSize; i++) {
            args[i] = leftCommand[i];
        }
        args[leftSize] = NULL;   

        // check error left, empty left argument
        if (leftCommand[0] == NULL) {
            printf("bash: syntax error near unexpected token `|'\n");
            fflush(stdout);
            return;
        }

        // close read descriptor
        close(pfd[0]);

        // stdout to pfd pipe for write 
        dup2(pfd[1], STDOUT_FILENO);

        // close write end pipe
        close(pfd[1]);
	}

    // parent process, execute right command
    else {

        // assign to right command
        for (int i = 0; i < rightSize; i++) {
            args[i] = rightCommand[i];
        }
        args[rightSize] = NULL;

        // close read descriptor
        close(pfd[1]);

        // stdin to fd pipe for read
        dup2(pfd[0], STDIN_FILENO); 

        // close read end pipe
        close(pfd[0]);
    }
    wait(NULL);
}