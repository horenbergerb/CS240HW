#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

const int MAX_ARGS = 10;
const int MAX_SIZE = 20;

char** getInput();

int main(){

  //The main loop for the shell
  while(1){

    //The function for parsing the input
    char** commands = getInput();

    //Skip this iteration if the command is blank
    if(commands[0] == NULL)
      continue;

    //If the command is "exit," then exit
    if(strcmp(commands[0], "exit") == 0)
      break;

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

    //Freeing the memory
    int initCount;
    for(initCount = MAX_ARGS-1; initCount >= 0; initCount--){
      free(commands[initCount]);
    }
    free(commands);
  }
}

char** getInput(){
  //Setting up the input using strtok
  char input[100];
  char* parser;
  printf(">:");
  fgets(input, 100, stdin);
  char** commands = malloc(sizeof(char*)*MAX_ARGS);
  int initCount;
  for(initCount = 0; initCount < MAX_ARGS; initCount++){
    commands[initCount] = malloc(sizeof(char)*MAX_SIZE);
  }
  memset(commands, 0, MAX_ARGS);

  //The while command which parses the input
  parser = strtok(input, " \n");
  int counter = 0;
  while(counter < MAX_SIZE && parser != NULL){
    if(strcmp(parser, "") != 0){
      //Initializes each part of the array, then copys in
      commands[counter] = malloc(sizeof(char)*100);
      strcpy(commands[counter], parser);
      parser = strtok(NULL, " \n");
      counter++;
    }
  }

  //A null value so the exec knows when to end
  commands[counter] = NULL;

  free(parser);

  return commands;
}
