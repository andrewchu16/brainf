#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <sys/stat.h> // struct stat


bool file_exists (char *filename);
void optimizer();
long long int parse_file(FILE *fp, long long int len); // removes other characters

char *input; // filtered brainf code
long long int length;

int main(int argc, char* argv[]) 
{
    // error checking for file
	if (argc != 2 && argc != 3)
	{
		printf("Usage: './brainf [file.bf]\nOR\n./brainf [file.bf] [option]\n");
		return 1;
	}
    else if (!file_exists(argv[1]))
	{
		printf("Error: filename is wrong or does not exist in this directory\n");
		return 2;
	}
	
	FILE* fptr = fopen(argv[1], "r");
	if (fptr == NULL)
	{	
		printf("Error: file pointer NULL\n");
		return 3;
	}

    fseek(fptr, 0, SEEK_END); // go to end
	length = ftell(fptr); // get file length
	fseek(fptr, 0, SEEK_SET); // go to beginning

    length = parse_file(fptr, length);
    printf("parsed: %s\n", input);

    fclose(fptr);

    return 0;
}

// file_exists written by codebunny & Adam Liss @ https://stackoverflow.com/a/230070
bool file_exists (char *filename) 
{
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

void optimizer() 
{
    char *reader = (char *) malloc(length * sizeof(char));

    char last_ins = '_';
    long long int adds = 0, subs = 0, mv_right = 0, mv_left = 0;
    long long int read_ptr = 0;

    for (long long int i = 0; i < length; i++)
    {
        
    }

}

long long int parse_file(FILE *fp, long long int len)
{
    input = (char *) malloc(length * sizeof(char));

    char tmp = '_', last_ins = '_';
    long long int read_ptr = 0;

    for (long long int i = 0; i < len; i++)
    {
        fread(&tmp, sizeof(char), 1, fp);

        switch(tmp) 
        {
            case '+': // add
            case '-': // subtract
            case '>': // move right
            case '<': // move left
            case '.': // write char
            case ',': // read char
                input[read_ptr++] = tmp;
        }
    }
    input[read_ptr] = '\0'; // add end-of-string char

    return len;
}