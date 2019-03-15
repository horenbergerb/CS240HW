#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(){
  clock_t begin = clock();
  FILE *fp;

  while((double)(clock()-begin)/CLOCKS_PER_SEC < (60*5)){
    fp = fopen("test.txt", "w+");
    fputs("z", fp);
    fclose(fp);
  }
  printf("Done executing\n");
}
