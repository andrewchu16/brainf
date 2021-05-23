#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>

#define THROW(code, ...) { printf(__VA_ARGS__); exit(code); }
#define THROW_IF(cond, code, ...) if (cond) { THROW(code, __VA_ARGS__); }

bool      valid_file(char *filename);
long long vaild_line(char *line);

void run_file(char *filename);
void run_prompt();
void run_line(char *line);

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
            THROW(1, "Usage:\n"
                    "%s        - run brainf code interactively.\n"
                    "%s [file] - run brainf code from a script.\n\n", 
                    argv[0], argv[0]);
            break;
    }
}

void run_prompt()
{
    char *line = calloc(1001, sizeof(char));
    THROW_IF(line == NULL, 2, "Error: unable to allocate memory.\n");

    while (true)
    {

    }

    free(line);
}

void run_file(char *filename) 
{
    THROW_IF(!valid_file(filename), 2, "Error: file does not exist [%s].\n", filename);


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
    long long index, count;

    for (index = 0, count = 0; line[index] != '\0'; index++)
    {
        if (line[index] == '[') count++;
        else if (line[index] == ']') count--;

        if (count < 0) return index;
    }

    return (count != 0) ? index : -1;
}
