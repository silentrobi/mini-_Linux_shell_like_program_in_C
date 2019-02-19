#include<stdio.h>
#include<stdlib.h>


struct lst_node{
    char data[100];
    struct lst_node* next;
};

typedef struct lst_node Node;


Node* createListNode(char* value);
void addToHistory(char * argument);
void printHistory();
void findloc(char* argument);
int in(char **arr, int len, char *target);
int findIndex(char **arr, int len, char *target);
void pipeOperation(char firstOp[5][10], char secondOp[5][10]);
void deallocate();
int parser(char* str, char** list, char* delimiter);
int buildinCommand(char* arg[]);
