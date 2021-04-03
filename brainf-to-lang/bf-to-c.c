#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <sys/stat.h>


typedef struct node 
{
    char ins;
    long long int count;
    struct node *next;
} Node;

// iterate through the list with a pointer
#define for_each_node_ref(head, it) for (Node *it = head; it != NULL; it = it->next)

bool file_exists (char *filename);
long long int parse_file(FILE *fp, long long int len, char *input); // filters bf code
void optimizer(long long int len, char *input, Node *head);
void write_file(Node *head);
void free_list(Node *head); // note: also frees head of the list
void add_node(Node **_cur, char _ins, long long int _count);


int main(int argc, char* argv[]) 
{
    // error checking for file
	if (argc != 2)
	{
		printf("Usage: '%s [file.bf]\n", argv[0]);
		return 1;
	}
    else if (!file_exists(argv[1]))
	{
		printf("Error: filename is wrong or does not exist (%s)\n", argv[1]);
		return 2;
	}
	
	FILE* fptr = fopen(argv[1], "r");
	if (fptr == NULL)
	{	
		printf("Error: file pointer NULL\n");
		return 3;
	}

    // get file length
    fseek(fptr, 0, SEEK_END);
	long long int length = ftell(fptr);
	fseek(fptr, 0, SEEK_SET); 

    char *input = (char *) malloc(length * sizeof(char));

    // update length to new filtered code
    length = parse_file(fptr, length, input);
    printf("parsed: %s\n", input);

    Node *head = (Node *) malloc(sizeof(Node));

    optimizer(length, input, head);

    for_each_node_ref(head, it)
    {
        printf("%lld%c", it->count, it->ins);
    } printf("\n");

    fclose(fptr);

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

long long int parse_file(FILE *fp, long long int length, char *input)
{
    char tmp = '_';
    long long int read_ind = 0;

    // warning: read_ind != i
    for (long long int i = 0; i < length; i++)
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
            case '[': // begin loop
            case ']': // end loop
                input[read_ind++] = tmp;
        }
    }
    input[read_ind] = '\0'; // add end-of-string char
    return length;
}

void optimizer(long long int length, char *input, Node *head) 
{
    char prev_ins = '_';
    long long int prev_count = 0;

    Node *cur = head;

    for (int i = 0; i < length; i++) 
    {
        if (prev_ins != input[i])
        {
            add_node(&cur, prev_ins, prev_count);

            prev_count = 0;
        }

        prev_count++;
        prev_ins = input[i];
    }
}

void write_file(Node *head)
{
    
}

void free_list(Node *head) 
{
    Node *cur = head;

    // todo: is it cur != NULL or cur->next != NULL 
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

    // create next node
    tmp->count = _count;
    tmp->ins = _ins;
    tmp->next = NULL;

    // attach node to list
    (*_cur)->next = tmp;

    // move cur to next in list
    *_cur = (*_cur)->next;
}
