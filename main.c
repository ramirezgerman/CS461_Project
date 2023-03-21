/*
    German Ramirez
    CS461 - Operating Systems
    Simple Shell
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

void freeArgs(char *args[],int argv) {
    int i = 0;
    while(args[i] != NULL && (i < argv)) {
        free(args[i]);
        i++;
        if (i == 80) break;
    }
}
void readCommandFromUser(char *args[], int *hasAmp, int *argv) {
    char userCommand[MAXLINE];
    int length = 0;
    char delimiter[] = " ";
    length = read(STDIN_FILENO, userCommand, 80);
    if (userCommand[length - 1] == '\n') {
        userCommand[length - 1] = '\0';
    }
    if (strcmp(userCommand, "!!") == 0) {
        if (*argv == 0) {
            printf("No commands in history.\n");
        }
        return;
    }
    freeArgs(args, *argv);
    *argv = 0;
    *hasAmp = 0;
    char *ptr = strtok(userCommand, delimiter);
    while (ptr != NULL) {
        if (ptr[0] == '&') {
            *hasAmp = 1;
            ptr = strtok(NULL, delimiter);
            continue;
        }
        *argv += 1;
        args[*argv - 1] = strdup(ptr);
        ptr = strtok(NULL, delimiter);
    }
    args[*argv] = NULL;
}

int main(void) {
    char *args[MAXLINE / 2 + 1];
    int runFlag = 1;
    pid_t pid;
    int hasAmp = 0;
    int argv = 0;
    int usingPipe = 0;
    while (runFlag) {
        usingPipe = 0;
        printf("osh>");
        fflush(stdout);
        readCommandFromUser(args, &hasAmp, &argv);
        pid = fork();
        if (pid == 0) {
            if (argv == 0) {
                continue;
            } else {
                int redirectCase = 0;
                int file;
                for (int i = 1; i <= argv - 1; i++) {
                    if (strcmp(args[i], "<") == 0) {
                        file = open(args[i + 1], O_RDONLY);
                        if (file == -1 || args[i+1]  == NULL) {
                            printf("Invalid Command!\n");
                            exit(1);
                        }
                        dup2(file, STDIN_FILENO);
                        args[i] = NULL;
                        args[i + 1] = NULL;
                        redirectCase = 1;
                        break;
                    } else if (strcmp(args[i], ">") == 0) {
                        file = open(args[i + 1], O_WRONLY | O_CREAT, 0644);
                        if (file == -1 || args[i+1]  == NULL) {
                            printf("Invalid Command!\n");
                            exit(1);
                        }
                        dup2(file, STDOUT_FILENO);
                        args[i] = NULL;
                        args[i + 1] = NULL;
                        redirectCase = 2;
                        break;
                    } else if (strcmp(args[i], "|") == 0) {
                        usingPipe = 1;

                        int fd1[2];

                        if (pipe(fd1) == -1) {
                            fprintf(stderr, "Pipe Failed\n");
                            return 1;
                        }
                        char *firstCommand[i + 1];
                        char *secondCommand[argv - i - 1 + 1];
                        for (int j = 0; j < i; j++) {
                            firstCommand[j] = args[j];
                        }
                        firstCommand[i] = NULL;
                        for (int j = 0; j < argv - i - 1; j++) {
                            secondCommand[j] = args[j + i + 1];

                        }
                        secondCommand[argv - i - 1] = NULL;

                        int pid_pipe = fork();
                        if (pid_pipe > 0) {
                            wait(NULL);
                            close(fd1[1]);
                            dup2(fd1[0], STDIN_FILENO);
                            close(fd1[0]);
                            if (execvp(secondCommand[0], secondCommand) == -1) {
                                printf("Invalid Command!\n");
                                return 1;
                            }

                        } else if (pid_pipe == 0) {

                            close(fd1[0]);
                            dup2(fd1[1], STDOUT_FILENO);
                            close(fd1[1]);
                            if (execvp(firstCommand[0], firstCommand) == -1) {
                                printf("Invalid Command!\n");
                                return 1;
                            }
                            exit(1);
                        }
                        close(fd1[0]);
                        close(fd1[1]);
                        break;
                    }
                }
                if (usingPipe == 0) {
                    if (execvp(args[0], args) == -1) {
                        printf("Invalid Command!\n");
                        return 1;
                    }
                }
                if (redirectCase == 1) {
                    close(STDIN_FILENO);

                } else if (redirectCase == 2) {
                    close(STDOUT_FILENO);
                }
                close(file);
            }
            exit(1);
        } else if (pid > 0) {
            if (hasAmp == 0) wait(NULL);
        } else {
            printf("Error fork!!");
        }
    }
}
