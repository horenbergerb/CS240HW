#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(){
  clock_t begin = clock();

  while((double)(clock()-begin)/CLOCKS_PER_SEC < (60*5)){

  }
  printf("Done executing\n");
}
