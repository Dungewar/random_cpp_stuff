#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define FILENAME = "runme";

int main()
{
    const char *FILENAME = "runme";
    FILE *filePointer;
    filePointer = fopen(FILENAME, "r");
    if (filePointer == NULL)
    {
        // char *message = malloc(100);
        // message = ;
        // stdout << "Rename this file to ";
        printf("Rename this file to %s", FILENAME);
        return 1;
    }
    fseek(filePointer, 0, SEEK_END);
    long fileSize = ftell(filePointer);
    // fseek(filePointer, 0, SEEK_SET); // or rewind(fp);
    rewind(filePointer);

    char *buffer = (char *)malloc(fileSize + 1);
    // fgets()
    fread(buffer, 1, fileSize, filePointer);
    buffer[fileSize] = '\0';
    fclose(filePointer);

    printf("%s", buffer);

    if (remove(FILENAME) == 0)
    {
        printf("Deleted successfully. The only way that you can stop your files from being permamently deleted is if you play this game. I am holding your entire home directory for ransom. If you close the program prematurely, it will delete a \033[31mrandom\033[0m file from it.\n");
    }
    else
    {
        printf("Error: UD\n");
        return 1;
    }

    char *input;
    input = (char *)malloc(11 * sizeof(char));
    while (strcmp(input, "I consent\n") != 0)
    {
        printf("Type \"I consent\" to continue: ");
        fgets(input, 11, stdin);
        printf("%s", input);
    }
    printf("Thanks for consenting\n");

    FILE *fileWritePointer;
    fileWritePointer = fopen(FILENAME, "w");
    if (fileWritePointer == NULL)
    {
        printf("Error opening file for writing.\n");
        return 1;
    }

    fwrite(buffer, 1, fileSize, fileWritePointer);
}