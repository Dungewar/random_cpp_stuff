#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h> // Needed for mkdir
#include <errno.h>    // Needed for errno
#include <stdbool.h>

#define MAX_PATH_LEN 1024
// #define FILENAME = "runme"

typedef struct
{
    const char *message;
    bool valid;
} Rule;

typedef Rule (*OperationFunc)(char *);

Rule allUpper(char *msg)
{
    Rule base;
    base.message = "Lowercase letters are BANNED.\n";

    int i = 0;
    while (msg[i] != '\0')
    {
        // if(msg[i] >= 65 && msg[i] <= 90) {
        if (msg[i] >= 97 && msg[i] <= 122)
        {
            base.valid = false;
            return base;
        }
        i++;
    }
    base.valid = true;
    return base;
}

int main()
{
    srand(time(NULL));

    // ------------------ TESTING -----------------------

    printf("cheese->%d, CHEESe->%d, CHEESE->%d", allUpper("cheese").valid, allUpper("CHEESe").valid, allUpper("CHEESE").valid);

    return 0;
    // ------------------ TESTING -----------------------

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

    // printf("%s", buffer);

    // DIR *dir_ptr = opendir(".");
    // if (dir_ptr == NULL)
    // {
    //     printf("An error has occured\n");
    //     return 100;
    // }
    char *file;
    {
        DIR *d;
        struct dirent *dir;
        d = opendir(".");
        if (d)
        {
            char *files[20];
            int i = 0;

            while ((dir = readdir(d)) != NULL && i < 20)
            {
                // We definitely don't want to touch directories
                if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                    continue;

                files[i] = strdup(dir->d_name);
                if (files[i] == NULL)
                {
                    return 103;
                }
                printf("%s\n", files[i]);

                i++;
            }
            if (closedir(d) == -1)
                return 104;

            if (i == 0)
            {
                printf("File in unknown location, try moving it to a more familiar spot.");
                return 1;
            }
            int randomNumber = rand() % i;
            file = files[randomNumber];
            // printf("File stolen");
            // printf(file);
            // for (int j = 0; j < i; j++)
            // {
            //     free(files[j]);
            // }
        }
    }

    char newPath[MAX_PATH_LEN];
    if (remove(FILENAME) == 0)
    {
        // Now try to move their file

        // This is the destination directory
        const char *destDir = ".archive/";

        // 1. Create the new full path string: "archive/report.txt"
        snprintf(newPath, MAX_PATH_LEN, "%s%s", destDir, file);

        if (mkdir(destDir, 0700) != 0)
        {
            if (errno == EEXIST)
            {
                printf("Directory '%s' already exists.\n", destDir);
            }
            else
            {
                // Handle other potential errors (like permission denied)
                printf("An error has occured\n");
                return 105;
            }
        }
        rename(file, newPath);

        printf("This program has deleted itself, along with one of your files: \033[31m%s\033[0m. Don't worry, your files will get returned if you play this game.\n", file);
    }
    else
    {
        printf("An error has occured\n");
        return 101;
    }

    char *input;
    input = (char *)malloc(11 * sizeof(char));
    int strikes = -1;
    while (strcmp(input, "I consent\n") != 0)
    {
        strikes++;
        if (strikes == 3)
        {
            printf("You failed, files deleted.");
            return 0;
        }

        printf("Type \"I consent\" to continue, \033[31m%d/3 strikes\033[0m: ", strikes);
        fgets(input, 11, stdin);
    }
    printf("Thanks for consenting\n");
    // ---------------------- START OF ACTUAL GAME ----------------------

    // ----------------------- END OF ACTUAL GAME -----------------------
    if (rename(newPath, file) == 0)
    {
        printf("File \033[32m%s\033[0m returned :D\n", file);
    }

    FILE *fileWritePointer;
    fileWritePointer = fopen(FILENAME, "w");
    if (fileWritePointer == NULL)
    {
        printf("Error undeleting script\n");
        return 102;
    }
    fwrite(buffer, 1, fileSize, fileWritePointer);
}