#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>

#define THROW_IF(cond, code, ...) if (cond) { printf(__VA_ARGS__); exit(code); }

bool file_exists(char *filename);


int main(int argc, char *argv[])
{
    THROW_IF(argc != 2, 1, "Usage: '%s [file.bf]'\n", argv[0]);
    THROW_IF(!file_exists(argv[1]), 2, "Error: filename is wrong or does not exist (%s)\n", argv[1]);

    FILE* rptr = fopen(argv[1], "r");

    THROW_IF(rptr == NULL, 3, "Error: file pointer NULL (%s)\n", argv[1]);

    // get file length
    fseek(rptr, 0, SEEK_END);
    long long int length = ftell(rptr);
    fseek(rptr, 0, SEEK_SET);

    THROW_IF(length == 0, EXIT_FAILURE, "Error: file is empty (%s)\n", argv[1]);
    
    char *input = (char *) malloc(length + 1);
    

    fclose(rptr);

    free(input);
    return 0;
}

bool file_exists(char *filename) 
{
    struct stat buffer;   
    return (stat(filename, &buffer) == 0);
}