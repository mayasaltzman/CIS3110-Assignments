#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#define MAX 256 // max length of a file name

bool readFile(char *fileName, char mode);

void hexdump(char *ascii);

int main()
{
    char menuOption = ' ';
    char fileName[MAX];
    char mode = 'a';
    bool opened = false;
    printf("Type o to enter a file name\nType d to select display mode\nType x to exit\n");

    scanf(" %c", &menuOption);
    getchar(); // removes trailing newline from input

    while (menuOption != 'x' && menuOption != 'X')
    {
        while (menuOption != 'o' && menuOption != 'd' && menuOption != 'x' && menuOption != 'O' && menuOption != 'D' && menuOption != 'X' && menuOption != 'm' && menuOption != 'M') // validating input
        {
            printf("Invalid input\n");

            printf("Type o to enter a file name\nType d to select display mode\nType x to exit\n");
            scanf("%c", &menuOption);
            getchar();
        }

        if (menuOption == 'o' || menuOption == 'O') // prompts for file name then displays contents of file in display mode
        {
            printf("Enter a file name\n");
            fgets(fileName, MAX, stdin);           // only reads line until max length of a file name
            fileName[strcspn(fileName, "\n")] = 0; // removing trailing newline from file name
            opened = readFile(fileName, mode);

            if (opened == true) // checking if file was opened
            {
                printf("Type m to return to main menu\nType x to exit\n");
                scanf(" %c", &menuOption);
                getchar();

                while (menuOption != 'x' && menuOption != 'D' && menuOption != 'X' && menuOption != 'm' && menuOption != 'M') // checking for invalid input
                {
                    printf("Invalid input1\n");
                    printf("Type m to return to main menu\nType x to exit\n");
                    scanf(" %c", &menuOption);
                    getchar();
                }

                if (menuOption == 'm' || menuOption == 'M')
                {
                    printf("Type o to enter a file name\nType d to select display mode\nType x to exit\n");
                    scanf(" %c", &menuOption);
                    getchar();
                }
            }
            else
            {
                printf("Type o to enter a file name\nType d to select display mode\nType x to exit\n");
                scanf(" %c", &menuOption);
                getchar();
            }
        }
        else if (menuOption == 'd' || menuOption == 'D') // allows user to change the display mode
        {
            printf("Enter a display mode\nType a for ASCII\nType h for hex\n");
            scanf(" %c", &mode);
            getchar();

            while (mode != 'a' && mode != 'h' && mode != 'A' && mode != 'H') // validating input
            {
                printf("Invalid input\n");
                printf("Enter a display mode\nType a for ASCII\nType h for hex\n");
                scanf(" %c", &mode);
                getchar();
            }

            printf("Type o to enter a file name\nType d to select display mode\nType x to exit\n");
            scanf(" %c", &menuOption);
            getchar();
        }
    }

    return 0;
}

/******
readFile: reads text from a file into a buffer and displays it in either ascii or hex
depending on the display mode
In: char *fileName, char mode
Out: void
*******/
bool readFile(char *fileName, char mode)
{
    char *buffer = NULL;
    int fptr;
    int n;
    int temp = 0;
    bool opened = true;

    fptr = open(fileName, O_RDONLY);                      // opening file
    int length = lseek(fptr, 0, SEEK_END);                // determining size of file
    buffer = (char *)malloc((length + 1) * sizeof(char)); // allocating space for file to be read

    if (fptr < 0) // error checking if the file opened
    {
        fprintf(stderr, "Could not open file %s\n", fileName);
        opened = false;
    }

    lseek(fptr, 0, SEEK_SET); // seek back to start

    while ((n = read(fptr, buffer, length)) > 0) // reading the buffer
    {
        if (n < 0)
        { // error checking for file read error
            fprintf(stderr, "read error %s\n", fileName);
        }

        for (int i = 0; i < length; i++) // replacing char 0x0 - 0x9, 0xB - 0x1F with space and 0x7F with ?
        {
            temp = (int)(buffer[i]) + 0; // casting the chars in buffer to be decimal values
            if (temp >= 0 && temp <= 9)
            {
                buffer[i] = ' ';
            }
            else if (temp >= 127)
            {
                buffer[i] = '?';
            }
        }

        if (mode == 'a')
        { // display string in ascii
            fprintf(stdout, "%s\n", buffer);
        }
        else if (mode == 'h')
        { // display string in hex
            hexdump(buffer);
        }
    }

    close(fptr);
    free(buffer);

    return opened;
}

/******
hexdump: displays ascii string as a hex string
In: char *ascii
Out: void
*******/
void hexdump(char *ascii)
{

    int count = 0;

    for (int i = 0; i < strlen(ascii); i++)
    {
        if (i % 16 == 0) // prints a new line for every 16 chars
        {
            count++;

            printf("\n");

            printf("%07x ", i); // printing char index for each line
        }

        if (ascii[i] == '\n')
        {
            printf("0%x ", ascii[i]); //%x formats ascii as it's hex value
        }
        else
        {
            printf("%x ", ascii[i]); //%x formats ascii as it's hex value
        }
    }

    printf("\n%07lx\n", strlen(ascii)); // printing the index of the last char in ascii string
}
