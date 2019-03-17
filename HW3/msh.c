#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

struct comNode{
  struct comNode* next;
  char command[100];
};

const int MAX_ARGS = 10;
const int MAX_SIZE = 20;

void getInput(struct comNode** head);
int forkCommand();
char** parseCommand(char* commands);

int main(){

  //The main loop for the shell
  while(1){

    struct comNode* head = NULL;
    getInput(&head);
    struct comNode* nextNode = head;
    while(nextNode != NULL){
      char** commands = malloc(sizeof(char*)*MAX_ARGS);

      commands = parseCommand(nextNode->command);
      //Skip this iteration if the command is blank
      if(commands[0] == NULL)
	continue;
      
      //Return 2 if the command was "exit"
      if(forkCommand(commands)==2)
	break;
      
    //Freeing the memory
      int initCount;
      for(initCount = MAX_ARGS-1; initCount >= 0; initCount--){
	free(commands[initCount]);
      }
      nextNode = nextNode->next;
      free(commands);
    }
  }
}

void getInput(struct comNode** head){
  //Setting up the input using strtok
  char input[100];
  char* parser;
  printf(">:");
  fgets(input, 100, stdin);
  //Struct containing the separate command sections
  parser = strtok(input, ";\n");
  while (parser != NULL){
    if(*head == NULL){
      *head = (struct comNode*)malloc(sizeof(struct comNode));
      strcpy((*head)->command, parser);
      (*head)->next = NULL;
    }
    else{
      struct comNode* nextNode = *head;
      while(nextNode->next != NULL){
	nextNode = nextNode->next;
      }
      	nextNode->next = (struct comNode*)malloc(sizeof(struct comNode));
	strcpy(nextNode->next->command, parser);
	printf("%s\n", nextNode->next->command);
	nextNode->next->next = NULL;

    }
    parser = strtok(NULL, ";");
  }
  return;
}

char** parseCommand(char* command){

  char** newCommands = malloc(sizeof(char*)*MAX_ARGS);
  char* parser;
  
  //The while command which parses the input
  parser = strtok(command, " \n");
  int counter = 0;
  while(counter < MAX_SIZE && parser != NULL){
    if(strcmp(parser, "") != 0){
      //Initializes each part of the array, then copys in
      newCommands[counter] = malloc(sizeof(char)*100);
	strcpy(newCommands[counter], parser);
	parser = strtok(NULL, " \n");
	counter++;
    }
  }
  //A null value so the exec knows when to end
  newCommands[counter] = NULL;

  free(parser);

  return newCommands;
}

int forkCommand(char** commands){
    //If the command is "exit," then exit
    if(strcmp(commands[0], "exit") == 0)
      return 2;

    //The forking for running the command
    pid_t childpid;
    int status;

    //If the fork fails
    if((childpid = fork()) == -1){
      perror("Error while forking\n");
      exit(1);
    }

    //Attempting the command
    else if(childpid == 0){
      execvp(commands[0], commands);
      printf("Failed to run command\n");
      exit(1);
    }

    //If something causes the child to freak
    else if(childpid != wait(&status)){
      perror("Child interrupted somehow\n");
    }

}
