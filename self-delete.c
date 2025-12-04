#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *filePointer;
    filePointer = fopen("input.txt", "r");
    if (filePointer == NULL)
    {
        printf("Rename this file to 'runme'");
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

    if (remove("input.txt") == 0)
    {
        printf("Deleted successfully. The only way that you can stop your files from being permamently deleted is if you play this game. I am holding your entire home directory for ransom. If you close the program prematurely, it will delete a *random* file from it.\n");
    }
    else
    {
        printf("Unable to delete the file\n");
    }

    char *input;
    input = (char *)malloc(100 * sizeof(char));
    printf("Type anything to continue: ");
    fgets(input, 100, stdin);

    FILE *fileWritePointer;
    fileWritePointer = fopen("input.txt", "w");
    if (fileWritePointer == NULL)
    {
        printf("Error opening file for writing.\n");
        return 1;
    }

    fwrite(buffer, 1, fileSize, fileWritePointer);
}