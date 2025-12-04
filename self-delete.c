#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *filePointer;
    filePointer = fopen("input.txt", "r");
    if (filePointer == NULL)
    {
        printf("Rename this file to 'runme'");
    }
    else
    {
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
    }
}