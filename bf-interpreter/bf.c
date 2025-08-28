#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>


/* BYTECODE */

enum OPS {
    OP_ADDN, // +
    OP_SUBN, // -
    OP_MOVL, // <
    OP_MOVR, // >
    OP_JMPL, // [
    OP_JMPR, // ]
    OP_SCAN, // ,
    OP_PRNT, // .
    OP_NULL  // non-keywords
};

typedef struct {
    int OP_type;
    int val;
} INS;

/* PROTOTYPES */

void run_file(char *filename);
void run_prompt();
void run_line(long long length);


long long make_bytecode(long long length);
long long make_ins(int prev_op, int cur_op, long long bytecode_length);

bool      valid_file(char *filename);
long long valid_line(char *line);

long long get_file_length(char *filename);
long long get_line_length(char *line);
int get_op(char c);

void show_error(const long long error_point, const char *line);

// routine for freeing global heap-allocated variables
void free_mem(void);

/* MACROS */

#define FAIL(code, ...) { printf(__VA_ARGS__); exit(code); }
#define FAIL_IF(cond, code, ...) if (cond) { FAIL(code, __VA_ARGS__); }

#ifndef _WIN32
#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)
#endif

typedef unsigned char byte;

/* GLOBALS */

#define ARR_SIZE 30000

char *line = NULL;
FILE *rptr = NULL;
INS *bytecode = NULL;
byte arr[ARR_SIZE] = {0}; // array for bf code

/* START */
int main(int argc, char *argv[])
{
    atexit(free_mem);

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

void run_prompt()
{
    line = malloc(1001);
    FAIL_IF(line == NULL, 2, "Error: unable to allocate memory.\n");

    for (int total_lines = 1, res; ; total_lines++)
    {
        printf("[%u] $ ", total_lines);

        FAIL_IF(scanf("%1000[^\n]s", line) == EOF, 0, "\n");

        getc(stdin); // remove newline

        long long error_point = valid_line(line);
        if (error_point != -1) show_error(error_point, line);
        else run_line(get_line_length(line));

        // "clear" the string
        line[0] = '\0';
    }

    free_mem();
}

void run_file(char *filename) 
{
    FAIL_IF(!valid_file(filename), 2, "Error: file does not exist [%s].\n", filename);

    rptr = fopen(filename, "r");
    FAIL_IF(rptr == NULL, 2, "Error: file pointer null.\n");

    long long length = get_file_length(filename);

    line = malloc(length + 1);
    FAIL_IF(line == NULL, 2, "Error: unable to allocate memory.\n");
    fread(line, sizeof(char), length, rptr);
    line[length] = '\0';

    long long error_point = valid_line(line);
    if (error_point != -1) {
        show_error(error_point, line);
        FAIL(3, "Error: bad loop.\n");
    }

    run_line(length);

    free_mem();
}

long long make_bytecode(long long length) 
{
    bytecode = malloc(sizeof(INS) * length);
    FAIL_IF(bytecode == NULL, 2, "Error: unable to allocate memory.\n");
    long long bytecode_length = 0;

    for (int i = 0, j; i < length; i++) 
    {
        int cur_op = get_op(line[i]);

        switch(cur_op)
        {
            case OP_JMPL:
                bytecode[bytecode_length] = (INS) {OP_JMPL, -1};
                bytecode_length++;
                break;
            case OP_JMPR:
                j = bytecode_length - 1;
                while(bytecode[j].OP_type != OP_JMPL || bytecode[j].val != -1) j--;
                bytecode[j].val = bytecode_length;

                bytecode[bytecode_length] = (INS) {OP_JMPR, j};
                bytecode_length++;
            case OP_NULL: break;
            default:
                if (bytecode_length > 0 && bytecode[bytecode_length - 1].OP_type == cur_op) bytecode[bytecode_length - 1].val++;
                else 
                {
                    bytecode[bytecode_length] = (INS) {cur_op, 1};
                    bytecode_length++;
                }
                break;
        }
    }

    return bytecode_length;
}


void run_line(long long length)
{
    long long bytecode_length = make_bytecode(length);
    static int index = 0;

    // for (int i = 0; i < bytecode_length; i++) printf("%i ", bytecode[i].OP_type); printf("\n");
    // for (int i = 0; i < bytecode_length; i++) printf("%i ", bytecode[i].val); printf("\n");

    for (long long i = 0; i < bytecode_length; i++)
    {
        // printf("code=%i val=%i i=%lli\n", bytecode[i].OP_type, bytecode[i].val, i);
        switch(bytecode[i].OP_type)
        {
            case OP_ADDN:
                arr[index] += bytecode[i].val;
                break;
            case OP_SUBN:
                arr[index] -= bytecode[i].val;
                break;
            case OP_MOVL:
                index -= bytecode[i].val;
                break;
            case OP_MOVR:
                index += bytecode[i].val;
                break;
            case OP_JMPL:
                if (arr[index] == 0) i = bytecode[i].val;
                break;
            case OP_JMPR:
                i = bytecode[i].val - 1; // subtract 1 because of i++
                break;
            case OP_SCAN:
                for (int j = 0; j < bytecode[i].val; j++) arr[index] = getchar();
                break;
            case OP_PRNT:
                for (int j = 0; j < bytecode[i].val; j++) putchar(arr[index]);
                break;
                // case OP_NULL: break;
        }
        // for (int j = 0; j < 10; j++) printf("%i ", arr[j]); printf("\nindex=%i\n", index);
    }
    
    // printf("free %p\n", bytecode);
    free(bytecode);
    bytecode = NULL;
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
    for (long long i = 0; ; i++) 
        if (line[i] == '\0') return i;

    return (1ll << 63) - 1; // if this causes a warning just ignore it <3
}

long long get_file_length(char *filename)
{
    struct stat buffer;
    stat(filename, &buffer);
    return buffer.st_size;
}

int get_op(char c)
{
    switch(c) 
    {
        case '+': return OP_ADDN;
        case '-': return OP_SUBN;
        case '>': return OP_MOVR;
        case '<': return OP_MOVL;
        case '.': return OP_PRNT;
        case ',': return OP_SCAN;
        case '[': return OP_JMPL;
        case ']': return OP_JMPR;
        default:  return OP_NULL;
    }
}

void show_error(const long long error_point, const char *line)
{
    printf("Error at character %lli\n", error_point);

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
    // printf("free_mem bytecode=%p\n", bytecode);
    if (line != NULL) { 
        free(line);
        line = NULL;
    }

    if (rptr != NULL) { 
        fclose(rptr);
        rptr = NULL;
    }

    if (bytecode != NULL) { 
        free(bytecode);
        bytecode = NULL;
    }
}
