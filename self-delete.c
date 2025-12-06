#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h> // Needed for mkdir
#include <errno.h>    // Needed for errno
#include <stdbool.h>
#include <ctype.h>
#include <curl/curl.h>

#define MAX_PATH_LEN 1024
#define DEBUG true
// #define FILENAME = "runme"

typedef struct
{
    const char *message;
    const char *name;
    bool valid;
} Rule;

typedef Rule (*OperationFunc)(char *);

Rule allUpper(char *msg)
{
    Rule base;
    base.message = "Lowercase letters are BANNED.\n";
    base.name = "All Uppercase";

    int i = 0;
    while (msg[i] != '\0')
    {
        // if(msg[i] >= 65 && msg[i] <= 90) {
        if (msg[i] >= 'a' && msg[i] <= 'z')
        {
            base.valid = false;
            return base;
        }
        i++;
    }
    base.valid = true;
    return base;
}
Rule cheesePlease(char *msg)
{
    Rule base;
    base.message = "Each 'c' needs to be followed by one 'h' (all cases), each 'H' by an 'E' (this one only in uppercase), and each 'e' by an 's' (this one only in lowercase, BANNED in uppercase). Note that nothing needs to follow the last character in the message.\n";
    base.name = "Cheese Please";

    int i = 0;
    while (msg[i] != '\0')
    {
        if (i != 0)
        {
            // Check if the string fails
            if ((tolower(msg[i - 1]) == 'c' && tolower(msg[i]) != 'h') ||
                (msg[i - 1] == 'H' && msg[i] != 'E') ||
                (msg[i - 1] == 'e' && msg[i] != 's') ||
                (msg[i - 1] == 'E' && msg[i] == 'S'))
            {
                base.valid = false;
                return base;
            }
        }
        i++;
    }
    base.valid = true;
    return base;
}
Rule sixteenChars(char *msg)
{
    Rule base;
    base.message = "You need to have exactly 16 characters\n";
    base.name = "Sixteen Chars";

    base.valid = strlen(msg) == 16;
    return base;
}

OperationFunc allRules[] = {
    allUpper,
    cheesePlease,
    sixteenChars,
    NULL};

char *tests[] = {
    "cheese",
    "CHEESE",
    "CHEese",
    "ch",
    "c.h",
    "es",
    "ES",
    "he",
    "HE",
    "HAE",
    "pneumenoultramic",
    "CH",
    "CHE",
    "che",
    "ches",
    "CHES",
    "cHe",
    NULL};

char *getRandomFile()
{
    char *file = NULL;
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
                // Free previously allocated memory
                for (int j = 0; j < i; j++)
                {
                    free(files[j]);
                }
                return NULL;
            }
            printf("%s\n", files[i]);
            if (DEBUG)
            {
                printf("%s\n", files[i]);
            }
            i++;
        }
        if (closedir(d) == -1)
        {
            for (int j = 0; j < i; j++)
            {
                free(files[j]);
            }
            return NULL;
        }

        if (i == 0)
        {
            printf("File in unknown location, try moving it to a more familiar spot.");
            return NULL;
        }
        int randomNumber = rand() % i;
        file = files[randomNumber];
        // printf("File stolen");
        // printf(file);
        for (int j = 0; j < i; j++)
        {
            if (j != randomNumber)
            {
                free(files[j]);
            }
        }
    }
    return file;
}

// --- 1. Define a structure to hold the server's response data ---
struct memory
{
    char *response;
    size_t size;
};

// --- 2. Callback function to capture the response ---
// This function is called by libcurl as data arrives.
// 'ptr' is the data, 'size' is 1, 'nmemb' is the number of items.
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)data;

    // Reallocate memory to accommodate the new data
    char *temp = realloc(mem->response, mem->size + realsize + 1);
    if (temp == NULL)
    {
        printf("Not enough memory (realloc failed)!\n");
        return 0; // Return 0 to signal an error
    }

    mem->response = temp;
    // Copy the new data to the end of the existing buffer
    memcpy(&(mem->response[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0; // Null-terminate the string

    return realsize;
}

int main()
{
    srand(time(NULL));

    // ================= TESTING =================

    if (DEBUG)
    {
        for (int i = 0; allRules[i] != NULL; i++)
        {
            // Need a generic "" to get anything returned for name
            printf("\nRule %d: %s\nDoes it pass?\n", i + 1, allRules[i]("").name);
            for (int j = 0; tests[j] != NULL; j++)
            {
                printf("%17s => %s\n", tests[j], allRules[i](tests[j]).valid ? "true" : "false");
            }
        }
        // return 0;
    }

    // ================= TESTING =================

    const char *FILENAME = "runme";
    FILE *filePointer;
    filePointer = fopen(FILENAME, "r");
    if (filePointer == NULL)
    {
        printf("Rename this file to %s", FILENAME);
        return 1;
    }
    fseek(filePointer, 0, SEEK_END);
    long fileSize = ftell(filePointer);
    // fseek(filePointer, 0, SEEK_SET); // or rewind(fp);
    rewind(filePointer);

    char *buffer = (char *)malloc(fileSize + 1);
    fread(buffer, 1, fileSize, filePointer);
    buffer[fileSize] = '\0';
    fclose(filePointer);

    char *file = getRandomFile();

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
    free(input);
    printf("Thanks for consenting\n");
    // =================== GAME LOOP ===================

    int ruleCount = sizeof(allRules) / sizeof(OperationFunc) - 1; // Exclude NULL terminator
    OperationFunc *availableRules[ruleCount];                     // Probably big enough
    OperationFunc *existingRules[ruleCount];                      // Probably big enough

    for (int i = 0; i < ruleCount; i++)
    {
        availableRules[i] = &allRules[i];
    }

    char *message = "Please return my file.";
    char *pastMessages = "User: Please return my file. AI: Why should I? User: Because I need it. AI: Convince me more.";
    // CURL *curl = curl_easy_init();
    // if(!curl) {
    //     printf("CURL initialization failed\n");
    //     return 106;
    // }
    // CURLcode res;
    // curl_easy_setopt(curl, CURLOPT_URL, "https://dungewar.com/api/convince-game");
    // res = curl_easy_perform(curl);
    // curl_easy_cleanup(curl);

    // if(res != CURLE_OK) {
    //     printf("CURL request failed: %s\nTrying again in a few seconds...", curl_easy_strerror(res));
    // }

    // Initialize the memory structure for the response
    struct memory chunk = {NULL, 0};
    // Allocate initial memory for the response (just a pointer)
    chunk.response = malloc(1);
    if (chunk.response == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }
    chunk.response[0] = 0; // Initialize as an empty null-terminated string

    CURL *curl;
    CURLcode res;

    // The data you want to send in the POST request (JSON format)
    char *post_data = malloc(200 + strlen(message) + strlen(file) + strlen(pastMessages)); // Probably big enough

    sprintf(post_data, "{\"message\": \"%s\", \"fileName\": \"%s\", \"pastMessages\": \"%s\"}", message, file, pastMessages);
    // Headers are needed to tell the server the data type being sent
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // You might also need an "Authorization" header for some APIs:
    // headers = curl_slist_append(headers, "Authorization: Bearer YOUR_TOKEN");

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (!curl)
    {
        printf("CURL initialization failed\n");
        free(chunk.response); // Clean up allocated memory
        return 106;
    }

    // --- Configuration for the POST Request ---

    // 1. Set the URL
    curl_easy_setopt(curl, CURLOPT_URL, "https://dungewar.com/api/convince-game");

    // 2. Set the request method to POST
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // 3. Set the data to be sent
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

    // Optional: Set the length of the data being sent
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(post_data));

    // 4. Set the HTTP headers (Crucial for JSON data)
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // --- Configuration for Capturing the Response ---

    // 5. Tell libcurl to use our custom function to handle incoming data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // 6. Pass the address of our 'chunk' structure to the callback function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // --- Perform the Request and Cleanup ---

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        printf("CURL request failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
        printf("✅ Request successful!\n");
        printf("--------------------------------\n");
        printf("Data Sent:\n%s\n", post_data);
        printf("--------------------------------\n");
        printf("Response received (Size: %zu bytes):\n%s\n", chunk.size, chunk.response);
        printf("--------------------------------\n");
    }

    // Clean up
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    curl_slist_free_all(headers); // Free the header list
    free(chunk.response);         // Free the allocated response memory
    /*
    0. They decide the mode to play
    [easy] Normal game
    [medium] 1 file gets 'deleted'
    [hard] 3 files get 'deleted', timer, files more hidden
    ----------------- Game Loop -----------------
    1. Ask the user to convince an AI of why their file should be returned
    2. The AI can progress by up to 10%, but can also regress by up to 5% if they're not convincing enough
    3. Every 20%, a new rule is added
    4. They have to follow the rule, or they get strikes (5 strikes and they lose, reset on success)
    5. That rule makes it much harder to convince the AI, which will judge their grammar etc
    ---------------------------------------------
    */

    // =================== GAME LOOP ===================-
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
    free(buffer);
    fclose(fileWritePointer);
}