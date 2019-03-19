#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

//NOTE: certain cases of ';' cause crashes: 'ls;' for example
//NOTE: alias edge cases and parsing are ugly
//NOTE: reading in a file has weird spacing!
//NOTE: parsing the export path command is super janky
//NOTE: history is also janky. Weird spacing!
/*TODO:
Font Change
Piping
 */

/*
Color change:
catch color requests and pass to setColor()
 */

//Struct for linked list containing many commands
struct comNode{
  struct comNode* next;
  char command[100];
};

struct histNode{
  struct histNode* next;
  char command[100];
  int ID;
};

struct aliasNode{
  struct aliasNode* next;
  char alias[100];
  char actualCom[100];
};

int curID = 0;

struct histNode* histHead = NULL;

//The head for the alias chain
struct aliasNode* aliasHead = NULL;

//Max arguments in a single command
const int MAX_ARGS = 10;
//Max line size
const int MAX_SIZE = 100;

//Parses one line of text into separate commands
int getInput(struct comNode** head);
//Runs a parsed command
int forkCommand(char** commands);
//Parses a command into arguments
char** parseCommand(char* commands);

int implementAlias(char** commands);
char* searchAlias(char* name);
int unalias(char* name);
int changePath(char** commands);

int addToHistory(char* commands);
void printHistory();
char* runByID(int userID);

int changeColor();

int main(){

  int running = 1;
  //The main loop for the shell
  while(running){

    //Linked list for parsing one line into separate commands
    struct comNode* head = NULL;
    //Grab the line of text and separate into commands
    if(getInput(&head) == -1)
      break;
    if(head == NULL)
      continue;
    struct comNode* nextNode = head;
    //Navigate the commands and parse them into arguments, then execute
    while(nextNode != NULL){
      char** commands = malloc(sizeof(char*)*MAX_ARGS);

      if(nextNode->command[0] != '!')
	addToHistory(nextNode->command);
      commands = parseCommand(nextNode->command);
      
      //Skip this iteration if the command is blank
      if(commands[0] == NULL)
	continue;

      if(strcmp(commands[0], "color") == 0){
	if(commands[1] == NULL)
	  printf("Please include an ASCII color. Example: color 196\n");
	else
	  changeColor(commands[1]);
	break;
      }
      
      //Return 2 if the command was "exit"
      if(forkCommand(commands)==2){
	running = 0;
	break;
      }
      
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

int getInput(struct comNode** head){
  //Setting up the input using strtok
  char input[100];
  char* parser;
  printf(">:");
  if(fgets(input, 100, stdin) == NULL && feof(stdin))
    return -1;
  //Creating a linked list breaking the line up along ';' delimiters
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
	nextNode->next->next = NULL;

    }
    parser = strtok(NULL, ";");
  }
  return 0;
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

    if(commands[0][0] == '!'){
      if(commands[0][1] == '!'){
	if(curID > 0)
	  commands = parseCommand(runByID(curID-1));
      }
      else{
	if(0 <= atoi(commands[0]+1) < curID)
	  commands = parseCommand(runByID(atoi(commands[0]+1)));
      }
    }
    
    //Catching alias requests
    if(strcmp(commands[0], "alias") == 0){
      implementAlias(commands);
      return 0;
    }

    if(strcmp(commands[0], "export") == 0){
      changePath(commands);
      return 0;
    }

    if(strcmp(commands[0], "history") == 0){
      printHistory();
      return 0;
    }

    
    //Catching unalias requests
    if(strcmp(commands[0], "unalias") == 0){
      unalias(commands[1]);
      return 0;
    }

    //Looking for aliases
    char* aliasCommand;
    if((aliasCommand = searchAlias(commands[0])) != NULL){
      commands = parseCommand(aliasCommand);
    }
    
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

//Parses the alias command, then searches the aliases and either modifies old alias or
//Adds a new alias
int implementAlias(char** commands){
  //breaks up the first part of the alias into the name and command
  char* parser;
  parser = strtok(commands[1], "=");
  char aliasName[100];
  char rawCom[100];
  if(parser != NULL)
      strcpy(aliasName, parser);
  else
    return -1;
  parser = strtok(NULL, "=\"");
  if(parser != NULL)
    strcpy(rawCom, parser);
  else
    return -1;
  //Breaks up the rest of the alias command and adds it to our command
  int counter = 2;
  for(counter = 2; commands[counter] != NULL; counter++){
    parser = strtok(commands[counter], "\"");
    strcat(rawCom, " ");
    while(parser != NULL){
      strcat(rawCom, parser);
      parser = strtok(NULL, "\"");
    }
  }

  //Adding the new alias and respective command to the linked list
  if(aliasHead == NULL){
    aliasHead = (struct aliasNode*)malloc(sizeof(struct aliasNode));
    strcpy((aliasHead->alias), aliasName);
    strcpy((aliasHead->actualCom), rawCom);
    aliasHead->next = NULL;
    printf("NAME: %s\n", aliasHead->alias);
    printf("COMMAND: %s\n", aliasHead->actualCom);
  }
  else{
    struct aliasNode* nextNode = aliasHead;
    while(nextNode->next != NULL && strcmp(nextNode->alias, aliasName) != 0){
      nextNode = nextNode->next;
      }
    nextNode->next = (struct aliasNode*)malloc(sizeof(struct aliasNode));
    strcpy((nextNode->alias), aliasName);
    strcpy((nextNode->actualCom), rawCom);
    printf("NAME: %s\n", nextNode->alias);
    printf("COMMAND: %s\n", nextNode->actualCom);
    nextNode->next->next = NULL;
  }
}

//Searches for an alias name, returns the respective command
char* searchAlias(char* name){
  if(aliasHead == NULL)
    return NULL;
  if(strcmp(aliasHead->alias, name) == 0){
    return aliasHead->actualCom;
  }
  else{
    struct aliasNode* nextNode = aliasHead;
    while(nextNode->next != NULL){
      nextNode = nextNode->next;
      if(strcmp(nextNode->alias, name) == 0){
	return nextNode->actualCom;
      }

    }
  }
  return NULL;
}

//Searches for an alias name, then removes that node from the linked list
int unalias(char* name){
  if(aliasHead==NULL)
    return 0;
  
  if(strcmp(aliasHead->alias, name) == 0){
    struct aliasNode* standIn = aliasHead;
    aliasHead = aliasHead->next;
    free(standIn);
    printf("Removed\n");
    return 1;
  }
  else{
    struct aliasNode* nextNode = aliasHead;
    while(nextNode->next != NULL){
      if(strcmp(nextNode->next->alias, name) == 0){
	struct aliasNode* standIn = nextNode->next;
	nextNode->next = nextNode->next->next;
	free(standIn);
	printf("Removed\n");
	return 1;
      }
      nextNode = nextNode->next;
    }
  }
  return 0;

}

int changePath(char** commands){
  char* newPath = malloc(sizeof(char)*1000);
  strcpy(newPath, getenv("PATH"));
  char* parser;
  if(commands[1] == NULL){
    printf("Invalid export command\n");
    return -1;
  }
  parser = strtok(commands[1], ":");
  parser = strtok(NULL, ":");
  strcat(newPath, ":");
  strcat(newPath, parser);
  setenv("PATH", newPath, 1);
  printf("%s\n", getenv("PATH"));
  free(newPath);
  //free(parser);
}

int addToHistory(char* commands){
  if(histHead == NULL){
    histHead = (struct histNode*)malloc(sizeof(struct histNode));
    strcpy((histHead->command), commands);
    histHead->ID = curID;
    histHead->next = NULL;
    curID++;
  }
  else{
    struct histNode* nextNode = histHead;
    while(nextNode->next != NULL){
      nextNode = nextNode->next;
      }
    nextNode->next = (struct histNode*)malloc(sizeof(struct histNode));
    strcpy((nextNode->next->command), commands);
    nextNode->next->ID = curID;
    nextNode->next->next = NULL;
    curID++;
  }

  if(curID > 20){
    struct histNode* tempNode = histHead;
    histHead = histHead->next;
    free(tempNode);
  }
  
  
  return 0;
}

void printHistory(){
  struct histNode* nextNode = histHead;
  while(nextNode != NULL){
    printf("%i:%s\n", nextNode->ID, nextNode->command);
    nextNode = nextNode->next;
  }
  return;
}

char* runByID(int userID){
  if(histHead == NULL)
    return NULL;
  if(histHead->ID == userID){
    return histHead->command;
  }
  else{
    struct histNode* nextNode = histHead;
    while(nextNode->next != NULL){
      nextNode = nextNode->next;
      if(nextNode->ID == userID){
	return nextNode->command;
      }

    }
  }
  return NULL;
}

int changeColor(char* colors){
  char** colorCommand = (char**)malloc(sizeof(char*)*3);
  colorCommand[0] = malloc(sizeof(char)*7);
  colorCommand[1] = malloc(sizeof(char)*255);
  strcpy(colorCommand[0], "echo");
  strcpy(colorCommand[1], "\e[38;5;");
  strcat(colorCommand[1], colors);
  strcat(colorCommand[1], "m");
  colorCommand[2] = NULL;
  forkCommand(colorCommand);

  return 0;
}
