/* ICCS227: Project 1: icsh
 * Name: Nattamon Santrakul
 * StudentID: 6381020
 */

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "ctype.h"
#include "unistd.h"

#define MAX_CMD_BUFFER 255

char** tokenize(char buffer[]) {
    char *token = strtok(buffer, " \t\n");

    char** buffer_list = malloc(MAX_CMD_BUFFER * sizeof(char*));    // allocate space for list of input

    int i = 0;
    while(token != NULL) {
      buffer_list[i] = token;
      token = strtok(NULL, " \t\n");
      i++;
    }
    buffer_list[i] = NULL;  // point the last element as NULL to indicate the end of the list
    return buffer_list;
}

// helper function to print char**
void printBuffer(char** buffer, int start) {
    for (int i = start; buffer[i] != NULL; i++) {
        printf("%s", buffer[i]);
        if (buffer[i] != NULL) { printf(" "); }
    }
    printf("\n");
}

// helper function to copy char** for copying list of token
char** copy(char** tokenList) {
    // malloc for outside list
    char** copyList = malloc(MAX_CMD_BUFFER * sizeof(char*));
    int i = 0;
    while (tokenList[i] != NULL) {
        // malloc for inside list
        copyList[i] = malloc(strlen(tokenList[i]) * sizeof(char));
        copyList[i] = strdup(tokenList[i]);
        i++;
    }
    copyList[i] = NULL;
    return copyList;
}

// running an external program (ex. ls)
void externalProg(char** shellCMD) {
    int status;
    int pid;
 
    /* Create a process space for the ls */
    if ((pid = fork()) < 0) {
        perror("Fork failed");
        exit(1);

    } else if (!pid) {
        /* This is the child, so execute the ls */ 
        status = execvp(shellCMD[0], shellCMD);   // execute the command

        // execvp unsuccessful
        if (status == -1) {
            printf("bad command");
        }
        exit(1);

    } else if (pid) {
        /* We're in the parent; let's wait for the child to finish */
        waitpid(pid, NULL, 0);
      }
}

void command(char** buffer, char** prev_buffer) {

    // echo
    if (strcmp(buffer[0], "echo") == 0 && buffer[1] != NULL) {
        if (strcmp(buffer[1], "$?") == 0) {
            // read exit number from file
            FILE *fp = fopen("exitNum", "r");
            if (fp == NULL) {
                printf("no previous exit number\n");
            } else {
                char num[MAX_CMD_BUFFER];
                while (fgets(num, sizeof(num), fp) != NULL) {
                    fputs(num, stdout);
                    printf("\n");
                    fclose(fp);
                }
            }
        } else {
            printBuffer(buffer, 1);
        }
    }

    // !!
    else if (strcmp(buffer[0], "!!") == 0 && buffer[1] == NULL) {
        if (prev_buffer == NULL) {
            // back to prompt if there is no previous command
        } else {
            printBuffer(prev_buffer, 0);
            command(prev_buffer, NULL);
        }
    }

    // exit
    // check if the first word is exit, the second word is a number, and the last word is null.
    else if (strcmp(buffer[0], "exit") == 0 && isdigit(*buffer[1]) && buffer[2] == NULL) {
        int exit_num = atoi(buffer[1]);

        // save exit number in a file
        FILE *fp = fopen("exitNum", "w");
        fputs(buffer[1], fp);
        fclose(fp);

        printf("Closing IC shell\n");
        exit(exit_num & 0xFF);   // truncate to fit in 8 bits
    }

    // ## comment
    else if (strcmp(buffer[0], "##") == 0) {
        // do nothing

    } else {
        // printf("bad command\n");
        externalProg(buffer);
    }
}

void readScript(FILE *file) {
    if (file == NULL) {
        printf("invalid file\n");
    } else {
        char buffer[MAX_CMD_BUFFER];
        char** prev_buffer = NULL;

        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            if (buffer[0] != '\n') {
                fputs(buffer, file);
                char** curr_buffer = tokenize(buffer);
                command(curr_buffer, prev_buffer);
                prev_buffer = copy(curr_buffer);
                free(curr_buffer);
            }
        }
        fclose(file);
    }
}

int main(int arg, char *argv[]) {

    // script mode
    if (arg > 1) {
        // create a FILE from argument (file path) - read mode
        FILE *file = fopen(argv[1], "r");
        readScript(file);

    } else {    // interactive mode
        char buffer[MAX_CMD_BUFFER];
        char** prev_buffer = NULL;
        printf("Starting IC shell\n");
        while (1) {
            printf("icsh $ ");
            fgets(buffer, 255, stdin);
            // printf("you said: %s\n", buffer);
            char** curr_buffer = tokenize(buffer);
            command(curr_buffer, prev_buffer);
            prev_buffer = copy(curr_buffer);
            free(curr_buffer);      // free current buffer before getting replace if still in loop
        }
    }
}
