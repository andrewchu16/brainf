#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>


void run_file(char *filename);
void run_prompt();
void run_line(char *line);

bool      valid_file(char *filename);
long long valid_line(char *line);

long long get_line_length(char *line);

void show_error(const long long error_point, const char *line);

// routine for freeing global heap-allocated variables
void free_mem(void);

#define FAIL(code, ...) { printf(__VA_ARGS__); free_mem(); exit(code); }
#define FAIL_IF(cond, code, ...) if (cond) { FAIL(code, __VA_ARGS__); }

#ifndef _WIN32
#define max(a, b) (a > b) ? a : b
#define min(a, b) (a < b) ? a : b
#endif


int main(int argc, char *argv[])
{
    switch (argc) 
    {
        case 1:
            run_prompt();
            break;
        case 2:
            run_file(argv[1]);
            break;
        default:
            FAIL(1, "Usage:\n"
                "%s        - run brainf code interactively.\n"
                "%s [file] - run brainf code from a script.\n\n", 
                argv[0], argv[0]);
            break;
    }
}

char *line = NULL;

void run_prompt()
{
    line = malloc(1001);
    FAIL_IF(line == NULL, 2, "Error: unable to allocate memory.\n");

    for (int total_lines = 1, res; ; total_lines++)
    {
        printf("[%u] $ ", total_lines);
        
        res = scanf("%[^\n]s", line);
        if (res == EOF) { 
            putc('\n', stdout);
            break;
        }

        getc(stdin); // remove newline

        long long error_point = valid_line(line);
        if (error_point != -1) show_error(error_point, line);
        else 
        {
            printf("%s\n", line);
        }

        // "clear" the string
        line[0] = '\0';
    }

    free_mem();
}

void run_file(char *filename) 
{
    FAIL_IF(!valid_file(filename), 2, "Error: file does not exist [%s].\n", filename);

    FILE *fptr = fopen(filename, "r");

    fseek(fptr, 0, SEEK_END);
    int length = ftell(fptr); // length of file
    fseek(fptr, 0, SEEK_SET);

    line = malloc(length + 1);

    FAIL_IF(line == NULL, 2, "Error: unable to allocate memory.\n");

    fread(line, sizeof(char), length, fptr);


    free_mem();
}

void run_line(char *line)
{
    
}

bool valid_file(char *filename) 
{
    struct stat buffer;   
    return (stat(filename, &buffer) == 0);
}

// goes through the string and returns index of the first invalid bracket, returns -1 if it is OK
long long valid_line(char *line)
{
    long long index, count, error_point = -1;

    for (index = 0, count = 0; line[index] != '\0'; index++)
    {
        if (line[index] == '[') 
        {
            error_point = index;
            count++;
        }
        else if (line[index] == ']') count--;

        if (count < 0) return index;

    }

    return (count != 0) ? error_point : -1;
}

long long get_line_length(char *line) 
{
    for (long long i = 0; i + 1 >= 0 ; i++) 
    {
        if (line[i] == '\0') return i + 1;
    }

    return ((long long) 1 << 63) - 1; // this causes a warning just ignore it <3
}

void show_error(const long long error_point, const char *line)
{
    printf("Error at line number %03lli\n", error_point);

    long long start_point = max(0, error_point - 10), end_point = error_point + 10;

    for (long long i = start_point; i < end_point && line[i] != '\0'; i++) 
        printf("%c", line[i]);
    printf("\n");

    for (int i = start_point; i < error_point; i++) 
        printf(" ");
    printf("^\n");
}

void free_mem(void) 
{
    if (line != NULL) free(line);
}