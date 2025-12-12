/*
TODO:
- Add more rules
- More files?
- Fix ctrl+c and also arrow keys not working
- Saving
// - Global progress bar
*/

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
#include <cJSON.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>
// #include <ncurses.h>
// #include <mathcalls.h>

#define MAX_PATH_LEN 1024
#define DEBUG true
// #define FILENAME = "runme"

// Global to store original settings so we can restore them on exit
struct termios orig_termios;

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode); // Ensure terminal restores even if program crashes

    struct termios raw = orig_termios;
    // ECHO: Don't print keys
    // ICANON: Read byte-by-byte (not line-by-line)
    // ISIG: specific to Ctrl+C (and Ctrl+Z) - prevents them from killing the app
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// DO NOT FREE STRINGS IN RULE STRUCTURE
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

    base.valid = strlen(msg) == 17; // to account for newline character
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
    if (DEBUG)
    {
        printf("Getting randomn't file...\n");
        return "example.txt";
    }
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

/**
 * Performs the calculation: round(0.0681818x^2 + 0.95303x - 6.16667)
 * For score -> global convincement mapping
 * * @param x The integer input.
 * @return The rounded integer result.
 */
int calculate_value(int x)
{
    // Define the coefficients using double for precision
    const double a = 0.0681818;
    const double b = 0.95303;
    const double c = -6.16667;

    // Calculate the polynomial value
    // Note: 'x' is automatically promoted to double for the calculation
    double result_double = (a * pow((double)x, 2.0)) + (b * (double)x) + c;

    result_double *= 2; // To speed up the game a bit
    // Round the result and cast the final value back to int
    return (int)round(result_double);
}

void clear_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

char *pretty_rule_print(Rule rule)
{
    // 30 for formatting, 20 extra
    char *returnString = (char *)malloc((strlen(rule.name) + strlen(rule.message) + 50) * sizeof(char));

    sprintf(returnString, "Rule \033[1m%s[\033[0m:\n\033[3m%s\033[0m\n", rule.name, rule.message);
    return returnString;
}

void slow_print(const char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        putchar(str[i]);
        fflush(stdout);
        usleep(50000); // Sleep for 50 milliseconds
    }
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

        printf("\n\nCalculating values for 1-10:\n");
        for (int i = 1; i <= 10; i++)
        {
            printf("%d => %d\n", i, calculate_value(i));
        }
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
                if (DEBUG)
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
        if (!DEBUG) // to not scare testers
            printf("This program has deleted itself, along with one of your files: \033[31m%s\033[0m. Don't worry, your files will get returned if you play this game.\n", file);
    }
    else
    {
        printf("An error has occured\n");
        return 101;
    }

    char *consentInput;
    consentInput = (char *)malloc(11 * sizeof(char));
    int strikes = -1;
    while (strcmp(consentInput, "I consent\n") != 0)
    {
        strikes++;
        if (strikes == 3)
        {
            printf("You failed, files deleted.");
            return 0;
        }

        printf("Type \"I consent\" to continue, \033[31m%d/3 strikes\033[0m: ", strikes);
        fgets(consentInput, 11, stdin);
    }
    // Clear the input buffer
    if (consentInput[strlen(consentInput) - 1] != '\n')
        clear_buffer();
    free(consentInput);
    printf("Thanks for consenting\n");
    // =================== GAME ===================

    bool restart = true;

    curl_global_init(CURL_GLOBAL_ALL);
    while (restart)
    {
        restart = false;
        int totalRuleCount = sizeof(allRules) / sizeof(OperationFunc) - 1; // Exclude NULL terminator
        int onlineRuleCount = 0;                                           // Defined as the following being true: onlineRules[onlineRuleCount] == NULL
        OperationFunc offlineRules[totalRuleCount];                        // Probably big enough
        OperationFunc onlineRules[totalRuleCount];                         // Probably big enough
        onlineRules[0] = NULL;

        for (int i = 0; i < totalRuleCount; i++)
        {
            offlineRules[i] = allRules[i];
        }

        char *pastMessages = malloc(100 * sizeof(char));
        pastMessages[0] = '\0'; // Initialize as empty string

        int attempts = 0;
        int globalConvincement = 0; // 0 to 100
        printf("Now, convince me that your file \033[32m%s\033[0m should be returned to you.\n\033[41m[200 char limit per message]\033[0m\n", file);

        // ========================= GAME LOOP START =========================
        while (globalConvincement < 100)
        { // Initialize the memory structure for the response

            printf("\n\033[47m\033[30m--- Attempt %d ---\033[0m\nConvincement: \033[32m%d/100\033[0m\n", ++attempts, globalConvincement);
            if (onlineRuleCount > 0)
            {
                printf("Active Rules:\n");
                for (int i = 0; i < onlineRuleCount; i++)
                {
                    char *ruleDescription = pretty_rule_print(onlineRules[i](""));
                    printf("%s\n", ruleDescription);
                    free(ruleDescription);
                }
                printf("\n");
            }

            char *convincingInput;
            convincingInput = (char *)malloc(200 * sizeof(char));
            int strikes = -1;
            bool correctUserResponse = false;
            while (!correctUserResponse)
            {
                strikes++;
                if (strikes == 0)
                {
                    printf("You can type \033[1mRESTART\033[0m to restart!\n");
                }

                if (strikes == 5)
                {
                    printf("You failed, files deleted.");
                    return 0;
                }

                printf("\033[31m%d/5 strikes\033[0m\nYour message: ", strikes);
                fgets(convincingInput, 200, stdin);
                if (convincingInput[strlen(convincingInput) - 1] != '\n')
                {
                    clear_buffer();
                }

                correctUserResponse = true;
                // Check against all existing rules
                for (int i = 0; onlineRules[i] != NULL; i++)
                {
                    // Need a generic "" to get anything returned for name
                    if (DEBUG)
                        printf("\nRule %d: %s\nDoes it pass?\n", i + 1, onlineRules[i]("").name);

                    Rule result = onlineRules[i](convincingInput);
                    if (result.valid == false)
                    {
                        correctUserResponse = false;
                        char *prettyPrint = pretty_rule_print(result);
                        printf("You broke %s\n", prettyPrint);
                        free(prettyPrint);
                    }
                }
                if (correctUserResponse && strikes == 0)
                {
                    if (strcmp(convincingInput, "RESTART\n") == 0)
                    {
                        printf("Restarting program...\n");
                        // TODO: restart program
                        restart = true;
                        break;
                    }
                }
            }
            if (restart)
            {
                break;
            }

            // ============== ITS POST REQUEST TIME ==============
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
            CURLM *multi;
            CURLMcode res;

            // The data you want to send in the POST request (JSON format)
            // char *post_data = malloc(200 + strlen(convincingInput) + strlen(file) + strlen(pastMessages)); // Probably big enough
            // sprintf(post_data, "{\"message\": \"%s\", \"fileName\": \"%s\", \"pastMessages\": \"[Most recent messages first]: %s\"}", convincingInput, file, pastMessages);

            cJSON *root = cJSON_CreateObject();
            if (root == NULL)
            {
                // Handle error
                printf("Failed to create JSON object, critical error\n");
                return 109;
            }
            cJSON_AddStringToObject(root, "message", convincingInput);
            cJSON_AddStringToObject(root, "fileName", file); // Assuming 'file' could also contain special chars
            cJSON_AddStringToObject(root, "pastMessages", pastMessages);
            char *post_data = cJSON_PrintUnformatted(root);

            // Check for allocation failure
            if (post_data == NULL)
            {
                // Handle error
                cJSON_Delete(root);
                return 110;
            }

            // Headers are needed to tell the server the data type being sent
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            // You might also need an "Authorization" header for some APIs:
            // headers = curl_slist_append(headers, "Authorization: Bearer YOUR_TOKEN");

            curl = curl_easy_init();
            multi = curl_multi_init();

            if (!curl)
            {
                printf("CURL initialization failed\n");
                free(chunk.response); // Clean up allocated memory
                return 106;
            }
            curl_multi_add_handle(multi, curl);

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

            // TODO: FIX
            curl_multi_add_handle(multi, curl);
            int still_running; /* keep number of running handles */

            long lastTime = time(NULL);
            const int TIME_DELAY = 1; // in seconds
            // random number from 17 to 208
            const int START_COLOR = 17 + (rand() % (208 - 17 + 1));
            int color = START_COLOR;
            if (DEBUG)
                printf("Starts with color: \033[38;5;%dm%d\033[0m\n", START_COLOR, START_COLOR);

            printf("Thinking");
            do
            {
                res = curl_multi_perform(multi, &still_running);

                if (!res && still_running)
                    /* wait for activity, timeout or "nothing" */
                    res = curl_multi_poll(multi, NULL, 0, 1000, NULL);

                if (res)
                {
                    fprintf(stderr, "curl_multi_poll() failed, code %d.\n", (int)res);
                    break;
                }
                long currentTime = time(NULL);
                if (currentTime - lastTime >= TIME_DELAY)
                {
                    lastTime = currentTime;
                    // put("\033[3m");
                    // putchar('.');

                    printf("\033[38;5;%dm.\033[0m", color++);
                    if (color > 231)
                        color = 17;
                    fflush(stdout);
                }

                // usleep(500000); // Sleep for 500 milliseconds

            } while (still_running); /* if there are still transfers, loop */
            printf("\033[1K");

            if (res != CURLE_OK)
            {
                printf("CURL request failed: %s\n", curl_easy_strerror(res));
            }
            else
            {
                if (DEBUG)
                {

                    printf("\033[32mRequest successful\033[0m!\n");
                    printf("--------------------------------\n");
                    printf("Data Sent:\n%s\n", post_data);
                    printf("--------------------------------\n");
                    printf("Response received (Size: %zu bytes):\n%s\n", chunk.size, chunk.response);
                    printf("--------------------------------\n");
                }

                cJSON *json = cJSON_Parse(chunk.response);
                if (json == NULL)
                {
                    printf("Error parsing JSON response\n");
                    return 107;
                }
                else
                {
                    cJSON *reply = cJSON_GetObjectItemCaseSensitive(json, "message");
                    cJSON *score = cJSON_GetObjectItemCaseSensitive(json, "convincement");
                    if (cJSON_IsString(reply) && (reply->valuestring != NULL) && cJSON_IsNumber(score) && (score->valueint > 0 && score->valueint <= 10))
                    {
                        char *response = malloc(strlen(reply->valuestring) + 50);
                        sprintf(response, "\033[94m%s\033[0m\nScore: %d\n", reply->valuestring, score->valueint);
                        slow_print(response);
                        usleep(500);

                        free(response);

                        int scoreValue = score->valueint;
                        globalConvincement += calculate_value(scoreValue);

                        pastMessages = realloc(pastMessages, strlen(pastMessages) + strlen(convincingInput) + strlen(reply->valuestring) + 20);
                        sprintf(pastMessages, "User: %s\nAI: %s\n", convincingInput, reply->valuestring);
                    }
                    else
                    {
                        printf("No valid 'reply' field in JSON response\n");
                        return 108;
                    }
                    cJSON_Delete(json);
                }
            }

            // Clean up
            free(convincingInput);
            curl_easy_cleanup(curl);

            curl_slist_free_all(headers); // Free the header list
            free(chunk.response);         // Free the allocated response memory
            cJSON_free(post_data);
            cJSON_Delete(root);

            // Add rules if pass thresholds
            if ((globalConvincement > 20 && onlineRuleCount < 1) || (globalConvincement > 40 && onlineRuleCount < 2)) // Add new rule here
            {
                // May make it not random later
                int index = rand() % (totalRuleCount - onlineRuleCount);
                onlineRules[onlineRuleCount] = offlineRules[index];
                onlineRules[onlineRuleCount + 1] = NULL;
                onlineRuleCount++;
                char *ruleDescription = pretty_rule_print(offlineRules[index](""));
                printf("Added %s", ruleDescription);
                free(ruleDescription);
                // shift offline rules
                for (int i = index; i < totalRuleCount - 1; i++)
                {
                    offlineRules[i] = offlineRules[i + 1];
                }
            }
            else if ((globalConvincement < 20 && onlineRuleCount > 0) || (globalConvincement < 40 && onlineRuleCount > 1)) // Remove rule here
            {

                char *ruleDescription = pretty_rule_print(onlineRules[onlineRuleCount - 1](""));
                printf("Removed %s", ruleDescription);
                free(ruleDescription);
                offlineRules[totalRuleCount - onlineRuleCount] = onlineRules[onlineRuleCount - 1];
                onlineRules[onlineRuleCount - 1] = NULL;

                onlineRuleCount--;
                // int index = rand() % totalRuleCount - onlineRuleCount;
                // onlineRules[index] = offlineRules[index];
            }
        }
    } // ========================= GAME LOOP END =========================
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

    // =================== FINAL RETURN ===================
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
    curl_global_cleanup();
}