
// Developed MYSHELL: A linux shell like script
//Assignment 3
//MOHAMMAD ABU MUSA RABIULID: 220201072
//YiGIT CAN TURK ID: 230201011


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "myscript.h"
#include <dirent.h>
#include <syscall.h>
#include <sys/wait.h>
#include <sys/stat.h>

/*global declarations */
const char* build_in_operation[5] = {"dir", "cd", "history", "findloc","bye"};
int nodeCounter = 0;
Node *head = NULL;


int main (int argc, char *argv[]){

    while(1) {


        int background=0;
        printf("%s", "myshell>");
        char userInput[100]; // store input
        char storeInput[100];
        char* arg[10];// arg of strings and arg size is 10

        for(int i=0;i<10;i++)	arg[i] = NULL; // initializing the argument vector

        gets(userInput); // get input from the console

        strcpy(storeInput,userInput); // store the user input to add into history

        if(!strcmp(userInput,"")) continue;		//if user does not type anything, loop restarts

        int size= parser(userInput,arg," ");  // parsing of command input using delimiter " "

        addToHistory(storeInput);	// store given command in history


        int flag = buildinCommand(arg); //fuction returns three outputs: 0---> buildin operation except "bye",
                                        // 1---> system commad
                                        // and -1 ---> bye command
        if (flag == -1){ //exit the myshell
            exit(1);
        } else if (flag){ // if return 1 then child process wil handle the shell command


            //PIPE START
            int pipe_op = in(arg,10,"|"); // if pipe operation found then return 1 else return 0
            if(pipe_op == 1){
                char firstOp[5][10]={"","","","",""};
                char secondOp[5][10]={"","","","",""};

                int n=0;
                int m=0;
                int index= findIndex(arg,10,"|"); // FIND INDEX OF THE "|"

                if (index == 0) continue; // if starting is | then continue.
                else if(index >0){
                    for (; n<index; n++){
                        strcpy(firstOp[n],arg[n]);

                    }
                    m= index+1;
                    int j=0;
                    for(;m<size;m++){
                        strcpy(secondOp[j],arg[m]);

                        j++;
                    }

                    pipeOperation(firstOp,secondOp); // PERFORM PIPE OPERATION
                    continue; // after performing pipe control goes to user
                }
            }


            /*foreground and background operations */

            background = in(arg,10,"&"); // return 1 when ampersand found else 0;
            int amper_index = findIndex(arg,10,"&");
            if(amper_index > -1)	arg[amper_index] = NULL;
            if(amper_index == 0){	printf("Can't Start with '&'\n"); continue;}
            pid_t child_id;
            // Create child process.
            child_id = fork();

            if (child_id < 0){
                printf("FORK failed\n");
            }
            else if (child_id == 0){

                if(execvp(arg[0],arg) == -1)	printf("ERROR: Command doesn't exist\n");

            }
            else{

                if(background == 1){	//BACKGROUND process

                    continue;

                }
                else{	//FOREGROUND process
                    wait(NULL);

                }

            }

        }
    }
    return 0;


}

Node* createListNode(char* value){ // create node
    Node *newNode = malloc(sizeof(Node));
    strcpy(newNode->data,value);
    newNode->next = NULL;

    return newNode;
}


int in(char **arr, int len, char *target) { // check argument vector to find a token
    int i;
    for(i = 0; i < len; i++) {
        if (arr[i] == NULL) {return -1;}
        if(strncmp(arr[i], target, strlen(target)) == 0) {
            return 1;
        }
    }
    return 0;
}

int findIndex(char **arr, int len, char *target){ // find index of the token
    int i;
    for(i = 0; i < len; i++) {
        if (arr[i] == NULL) {return -1;}
        if(strncmp(arr[i], target, strlen(target)) == 0) {
            return i;
        }
    }
    return -1;
}

void addToHistory(char *argument){		// add given command into a linked list that store user 10 most recent query

    if(head == NULL){
        head = createListNode(argument);
        nodeCounter++;
        return;
    }

    Node* current = head;

    while(current != NULL && current->next != NULL){
        current = current->next;
    }
    current->next = createListNode(argument);
    nodeCounter++;


    int step = nodeCounter - 10;
    if(step > 0){
        for(int i=0; i<step;i++){
            Node * temp= head;
            head = head->next;	// If nodeCounter greater than 10, head shifts to right and ex-head should be dynamically deallocated.
            free(temp); // deallocaing
        }
        nodeCounter = 10;
    }

}


void printHistory(){				// prints most recent 10 commands from linked list
    Node* current = head;
    int counter = 0;

    if(head == NULL){
        printf("You have not called any operation!\n");
        return;
    }

    while(current != NULL){
        printf("[%d] %s\n",counter+1,current->data);
        counter++;
        current = current->next;
    }
}


void findloc(char* argument){  // findlocation of a system command in the system path directory.
    // getting environment variable path

    char file[50];
    char* directories[20];
    for(int i=0;i<20;i++)	directories[i] = NULL; // initializing
    int size = 0;
    char  path[100];
    strcpy(path,getenv("PATH")); // get Path from the system file.
    if (path == NULL){
        perror("PATH variable is Empty");
    } else{
        // parsing
        size= parser(path, directories,":"); // parsing the path variable
    }

    int flagbit=1;
    DIR *d;
    while (size > 0){ // iteratively check each directory in the path

        struct dirent *dir;
        struct stat sb;
        d = opendir(directories[size-1]);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (!strcmp(dir->d_name, argument)){ // if command exist in the system directory
                    flagbit= 0;

                    strcpy(file,directories[size-1]);
                    strcat(file,"/");
                    strcat(file,argument);


                    if (stat(file, &sb) == 0 &&  sb.st_mode & S_IXUSR){ // check if the file executable

                        printf("%s/%s\n",directories[size-1],argument);
                    }
                    else{
                        printf("ERROR: file Not Executable!");

                    }
                }
            }

        }
        size --;
        closedir(d);
    }

    if (flagbit){
        printf("File doesn't exist\n");
    }

}


void deallocate() // dynamically deallocate the history upon exiting from the system.
{

    Node* current = head;

    Node* next;
    if (head == NULL){

        printf("%s\n", "Can't delete empty list");
        return;
    }
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    head=NULL; //making head value null

}

int parser(char* str, char** list, char* delimiter){ // parse the string (Path, user input) and return size of the argument vector

    int i=0;
    int size=0;
    if (strcmp(str,"")== 0) printf("ERROR: input string is empty!");
    list[i] = strtok(str,delimiter);

    while(list[i]!=NULL)
    {
        size++;
        i++;
        list[i] =strtok(NULL,delimiter);




    }
    return size;


}


/* perform buildin operations*/
// 0="dir", 1= "cd", 2= "history", 3= "findloc", 4 ="bye"};
int buildinCommand(char* arg[]){ // if return 1 then command is not build_in
                                 // if return -1 then sys terminate.
    // if command is built-in
    if(strcmp(arg[0], build_in_operation[2]) == 0){ // history
        if(arg[1] != NULL) printf("<---Two Many Arguments!--->\n"); 	// shows error if user types more than one argument
        else printHistory();
        return 0;
        
    }else if(strcmp(arg[0], build_in_operation[0]) == 0){
        if(arg[1] != NULL) printf("<---Two Many Arguments!--->\n"); 	// shows error if user types more than one argument
        else printf("%s\n",getcwd(0,0)); // we have handle system error
        return 0;
        
    }else if(strcmp(arg[0],build_in_operation[4]) == 0){
        deallocate();							// deallocating history for memory leaking
        return -1;
        
    }else if(strcmp(arg[0], build_in_operation[3]) == 0){
        if(arg[1] == NULL)		printf("<---No Argument Given!--->\n");		// shows error if user does not type any argument
        else if(arg[2] != NULL)	printf("<---Two Many Arguments!--->\n"); 	// shows error if user types more than two arguments
        else{
            findloc(arg[1]);
        }
        return 0;

    }else if(strcmp(arg[0], build_in_operation[1]) == 0){
        if(arg[1] == NULL)		printf("<---No Argument Given!--->\n");		// shows error if user does not type any argument
        else if(arg[2] != NULL)	printf("<---Two Many Arguments!--->\n"); 	// shows error if user types more than two arguments
        else{
            if(chdir(arg[1]) == -1){
                printf("<---Path Does not Exist!--->\n<---Redirecting to HOME path!--->\n");
                chdir(getenv("HOME"));
            }
        }
        return 0;
    }

    return 1;


}

void pipeOperation(char firstOp[5][10], char secondOp[5][10]){ //performs pipe operation
    pid_t pid1, pid2;
    int status;
    int fd[2];
    char* first= firstOp[0];
    char* second= secondOp[0];
    // The two commands we'll execute.
    /* create the pipe */
    if (pipe(fd) == -1) {
        fprintf(stderr,"Pipe failed\n");
        return;
    }
    // Create first process.
    pid1 = fork();
    if (pid1 < 0) {
        perror("Could not create process\n");
        return;
    }
    if (pid1 == 0) {
        // Hook stdout up to the write end of the pipe and close the read end of
        // the pipe which is no longer needed by this process.
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        // execute the first command
        if(execlp(first, first,NULL) == -1) printf("ERROR: Command doesn't exist\n");


    }
    // Create second process.
    pid2 = fork();
    if (pid2 < 0) {
        perror("Could not create process\n");
        return;

    }
    if (pid2 == 0) {
        // Hook stdin up to the read end of the pipe and close the write end of
        // the pipe which is no longer needed by this process.
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        // executing the second command
        if(execlp(second, second,NULL) == -1)  printf("ERROR: Command doesn't exist\n");
    }

// Closing both ends of the pipe.
    close(fd[0]);
    close(fd[1]);

// Wait for everything to finish and exit.
    waitpid(pid1,&status,0);
    waitpid(pid2,&status,0);
}
