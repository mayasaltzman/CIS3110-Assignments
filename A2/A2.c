#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#define MAX 256
#define MAX_COMMAND_LINE 2097152 // max number of command line arguments found with command: getconf ARG_MAX

int fd[MAX_COMMAND_LINE][2];
int numChildren[MAX_COMMAND_LINE];
int childCount = 1;
int count = 0;
int readBuffer[26];

void handler(int sigNum);
int *histogram(int fptr);
void saveToFile(int child_pid);

int main(int argc, char *argv[])
{
    int child_pid;
    int fptr = 0;
    int exitCount = 0;
    char *fileName = NULL;

    if (argc < 2) // no command line argument supplied exit program
    {
        fprintf(stdout, "error: must supply command line argument\n");
        exit(1);
    }

    // registering signal handler for SIGCHILD
    if (signal(SIGCHLD, handler) == SIG_ERR)
    {
        fprintf(stderr, "failed to catch SIGCHLD");
    }

    // for each file the parent will fork a child
    for (int i = 1; i < argc; i++)
    {
        if (pipe(fd[i]) == -1) // error checking for pipe issues
        {
            perror("pipe");
            exit(1);
        }

        if ((child_pid = fork()) == 0) // child code fork returns 0
        {
            sleep(1);

            // copying file name from parent into child
            fileName = malloc(sizeof(char *) * MAX);

            strcpy(fileName, argv[i]);

            close(fd[i][0]);                 // closing read side of the pipe
            fptr = open(fileName, O_RDONLY); // opening file

            if (fptr < 0) // child fails to open file
            {
                // closes write end of pipe and terminates with exit value 1
                printf("failed to read file %s\n", fileName);
                close(fd[i][1]);
                free(fileName);
                exit(1);
            }

            int *histo = histogram(fptr);

            sleep(10 + 2 * i);

            write(fd[i][1], histo, sizeof(int) * 26); // writing histo values

            close(fd[i][1]); // closing write end
            close(fptr);
            free(fileName);
            exit(0); // exiting so each child doesn't continue to fork new children
        }
        else if (child_pid > 0) // parent process
        {
            // handling special input
            if (strcmp(argv[i], "SIG") == 0)
            {
                kill(child_pid, SIGINT);
            }
        }
        else // error
        {
            fprintf(stderr, "failed to spawn child");
        }
    }

    // put parent in a loop to do nothing
    for (;;)
    {
        // waits until signal handler returns
        pause();

        // checking to see number of children that exited
        for (int i = 1; i < argc; i++)
        {
            if (numChildren[i] == 1) // if numChildren was set to 1 in handler they exited
            {
                exitCount++;
            }
        }

        if (exitCount == argc - 1) // if num children exited equals command line arguments parent can terminate
        {
            exit(0);
        }

        exitCount = 0;
    }

    return 0;
}

/******
handler: signal handler for SIGCHLD
In: int sigNum
Out: void
*******/
void handler(int sigNum)
{
    int child_status = 0;

    int pid = waitpid(-1, &child_status, WNOHANG);

    int killedBy = WIFSIGNALED(child_status);

    int killedBy2 = WTERMSIG(child_status);

    char *killedSignal = strsignal(child_status);

    if (signal(SIGCHLD, handler) == SIG_ERR)
    {
        fprintf(stderr, "Can't catch SIGCHILD %d\n", pid);
    }

    if (sigNum == SIGCHLD)
    {
        fprintf(stdout, "received SIGCHLD %d\n", pid);
        numChildren[childCount] = 1; // child exited properly set to 1 so we can check in parent process
        childCount += 1;
    }

    if (WIFEXITED(child_status) == 0)
    {
        printf("Child exited abnormally\n");
        printf("WIFSIGNALED: %d\n", killedBy);
        printf("WTERMSIG: %d\n", killedBy2);
        printf("Killed: %s\n", killedSignal);
    }
    else
    {
        count += 1;
        close(fd[count][1]);                                // closing write end
        read(fd[count][0], readBuffer, sizeof(readBuffer)); // reading
        close(fd[count][0]);                                // parent closes read end of pipe
        saveToFile(pid);
    }

    return;
}

/******
histogram: calculates histogram of a file pointer
In: int fptr
Out: int*
*******/
int *histogram(int fptr)
{
    char *buffer = NULL;
    int n;
    static int histo[26];
    char letter = ' ';
    int asInt = 0;

    for (int i = 0; i < 26; i++)
    {
        histo[i] = 0;
    }

    int length = lseek(fptr, 0, SEEK_END);                // determining size of file
    buffer = (char *)malloc((length + 1) * sizeof(char)); // allocating space for file to be read
    lseek(fptr, 0, SEEK_SET);                             // seek back to start

    while ((n = read(fptr, buffer, length)) > 0) // reading the buffer
    {
        if (n < 0)
        { // error checking for file read error
            fprintf(stderr, "read error %d\n", fptr);
        }
    }

    // calculating histogram
    for (int i = 0; i < length; i++)
    {
        letter = buffer[i];
        asInt = (int)letter; // casting to decimal value

        if (asInt >= 97 && asInt <= 122) // lower case letters
        {
            asInt = asInt - 97; // find index of letter
            histo[asInt] += 1;
        }

        if (asInt >= 65 && asInt <= 90) // upper case letters
        {
            asInt = asInt - 65; // find index of letter
            histo[asInt] += 1;
        }

        if (asInt < 65 && asInt > 122) // not a letter
        {
            continue; // go to next index
        }
    }

    close(fptr);
    free(buffer);

    return histo;
}

/******
saveToFile: creates a file based on the child PID and saves the histogram output to it
In: int child_pid
Out: void
*******/
void saveToFile(int child_pid)
{
    int fd;
    char fileName[MAX];
    char str[100];
    char *alpha = "abcdefghijklmnopqrstuvwxyz";

    sprintf(fileName, "file%d.histo", child_pid); // making file name using childs ID

    fd = open(fileName, O_WRONLY | O_CREAT, 0777); // open file for creating

    if (fd < 0) // file error checking
    {
        fprintf(stderr, "failed to create file %s\n", fileName);
    }
    else
    {
        for (int i = 0; i < 26; i++)
        {
            sprintf(str, "%c %d\n", alpha[i], readBuffer[i]); // formatting histo out put
            write(fd, str, sizeof(char) * strlen(str) + 1);   // writing to file
        }
    }

    close(fd);
}