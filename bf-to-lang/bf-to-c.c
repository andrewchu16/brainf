#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>

typedef struct node
{
    char ins;
    long long int count; // times instruction is repeated
    struct node *next;
} Node;

// macro for catching errors
#define THROW_IF(cond, code, ...) if (cond) { printf(__VA_ARGS__); exit(code); }

// change for larger or smaller tab sizes
#define TAB_SIZE 4

#define write_tabs(fp, tab_count) for (int i = 0; i < TAB_SIZE * (tab_count); i++) fputc(' ', fp)

// iterate through the list with a pointer
#define for_each_node_ref(head, it) for (Node *it = head; it != NULL; it = it->next)


bool file_exists(char *filename);

// utilities for parsing the *.bf file
// filters out all unrelated characters
long long int parse_file(FILE *fp, char *input); 

// organizes repeated commands into a list
Node *optimize(long long int len, char *input); 

// uses list to write the file
void write_file(char *old_name, Node *head);

// utilities for basic use of lists
// note: also frees head of the list
void free_list(Node *head); 

// note: moves cur
void add_node(Node **_cur, char _ins, long long int _count); 


int main(int argc, char* argv[])
{
    // error checking for file
    THROW_IF(argc != 2, 1,
            "Usage: '%s [file.bf]'.\n", argv[0]);
    THROW_IF(!file_exists(argv[1]), 2,
            "Error: filename is wrong or does not exist (%s).\n", argv[1]);

	FILE* rptr = fopen(argv[1], "r");

    THROW_IF(rptr == NULL, 3,
            "Error: file pointer NULL (%s).\n", argv[1]);

    // get file length
    fseek(rptr, 0, SEEK_END);
	long long int length = ftell(rptr);
	fseek(rptr, 0, SEEK_SET);

    THROW_IF(length == 0, EXIT_FAILURE,
            "Error: file is empty (%s).\n", argv[1]);

    char *input = (char *) malloc(length + 1);

    // update length to new filtered code
    length = parse_file(rptr, input);

    Node *head = optimize(length, input);

    write_file(argv[1], head);

    // exit program
    fclose(rptr);

    free(input);
    free_list(head);

    return 0;
}

// file_exists written by codebunny & Adam Liss @ https://stackoverflow.com/a/230070
bool file_exists(char *filename)
{
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
}

long long int parse_file(FILE *fp, char *input)
{
    char tmp;
    long long int read_ind = 0;

    while((tmp = fgetc(fp)) != EOF)
    {
        // me when i am falling through cases
        switch(tmp)
        {
            case '+': // add
            case '-': // subtract
            case '>': // move right
            case '<': // move left
            case '.': // write char
            case ',': // read char
            case '[': // begin loop
            case ']': // end loop
                input[read_ind++] = tmp;
        }
    }

    input[read_ind++] = '\0'; // add end-of-string char
    return read_ind;
}

Node *optimize(long long int length, char *input)
{
    Node *head = (Node *) malloc(sizeof(Node));
    *head = (Node) {
        .ins = 'x',
        .count = 0,
        .next = NULL
    };

    Node *cur = head;
    long long int prev_count = 1;
    long long int cur_layer = 0;
    
    switch(input[0])
    {
        case '[':
            cur_layer++;
            break;
        case ']':
            cur_layer--;
            break;
    }

    for (long long int i = 1; i < length; i++)
    {
        THROW_IF(cur_layer < 0, -1, "Error: Invalid square bracket syntax.\n");
        switch(input[0])
        {
            case '[':
                cur_layer++;
                break;
            case ']':
                cur_layer--;
                break;
        }

        if (input[i - 1] != input[i])
        {
            add_node(&cur, input[i - 1], prev_count);
            prev_count = 0;
        }
        prev_count++;
    }

    THROW_IF(cur_layer != 0, -1, "Error: Invalid square bracket syntax.\n");

    return head;
}

void write_file(char *old_name, Node *head)
{
    /*
     *  implemented using an array instead of a pointer
     *
     *  how: pointer becomes the index in an array
     *  why: because I don't want to malloc space for a pointer
     *
     *  sorry I don't want to #include <stdlib.h>
     */

    int f_name_len = strlen(old_name);
    char *f_name = (char *) malloc(f_name_len);

    // add file base
    strncpy(f_name, old_name, f_name_len - 2);

    // add file extension
    strcpy((f_name + f_name_len - 2), "c");

    FILE *wptr = fopen(f_name, "w");
    THROW_IF(wptr == NULL, 3, "Error: file pointer is NULL (%s)\n", f_name);


    // current number of tabs inside the loops
    unsigned int layer = 0;

    // pre-written translation commands
    char *text[4] = {
        "putchar(tape[index]);\n",    // '.'
        "tape[index] = getchar();\n", // ','
        "while(tape[index]) {\n",     // '['
        "}\n"                         // ']'
    };

    // how much to change the layer by
    int layer_off;

    // temporary char used for '>' and '<' commands
    char tmp;

    // program header
    fprintf(wptr, "// <Autogenerated>\n"
                "#include <stdio.h>\n"
                "\n"
                "typedef unsigned char byte;\n"
                "\n"
                "byte tape[30000] = {0};\n"
                "\n"
                "int main(void)\n"
                "{\n");
    write_tabs(wptr, 1);
    fprintf(wptr, "int index = 0;\n");
    write_tabs(wptr, 1);
    fprintf(wptr, "// START\n");

    for_each_node_ref(head, it)
    {
        char ins = it->ins;
        long long int count = it->count;

        // index of pre-written commands to choose
        int text_index = -1;

        // indent lines
        if (ins == ']') write_tabs(wptr, layer);
        else write_tabs(wptr, layer + 1);

        switch(ins)
        {
            case '+':
            case '-':
                if (count == 1) // -- or ++
                    fprintf(wptr, "tape[index]%c%c;\n", ins, ins); 
                else // -= or +=
                    fprintf(wptr, "tape[index] %c= %lld;\n", ins, count); 
                break;
            case '>':
            case '<':
                tmp = (ins == '>') ? '+' : '-';

                if (count == 1) // -- or ++
                    fprintf(wptr, "index%c%c;\n", tmp, tmp); 
                else // -= or +=
                    fprintf(wptr, "index %c= %lld;\n", tmp, count); 
                break;
            case '.':
                text_index = 0;
                layer_off = 0;
                break;
            case ',':
                text_index = 1;
                layer_off = 0;
                break;
            case '[':
                text_index = 2;
                layer_off = 1;
                break;
            case ']':
                text_index = 3;
                layer_off = -1;
                break;
        }

        if (text_index != -1)
        {   
            for (long long int i = 0; i < count; i++)
            {
                fprintf(wptr, "%s", text[text_index]);
                layer = layer + layer_off;
                if (i < count - 1) write_tabs(wptr, layer + (layer_off >= 0));
            }
        }
    }

    write_tabs(wptr, 1);
    fprintf(wptr, "// END\n");
    write_tabs(wptr, 1);
    fprintf(wptr, "return 0;\n"
                "}");

    printf("%s written successfully.\n", f_name);

    // close file
    fclose(wptr);

    free(f_name);
}


void free_list(Node *head)
{
    Node *cur = head;

    while(cur != NULL)
    {
        Node *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
}

void add_node(Node **_cur, char _ins, long long int _count)
{
    Node *tmp = (Node *) malloc(sizeof(Node));

    THROW_IF(tmp == NULL, EXIT_FAILURE,
            "Error allocating node for list (0x%p)\n", tmp);

    // create next node
    *tmp = (Node) {
        .ins = _ins,
        .count = _count,
        .next = NULL
    };

    // attach node to list
    (*_cur)->next = tmp;

    // move cur to next in list
    *_cur = (*_cur)->next;
}
