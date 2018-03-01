#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int main(int argc, char *argv[], char *envp[]) {

  int pid = 0;
  char *cpid = argv[2];
  int i = 0;
  char *str = "sbush: kill: Operation not permitted\n";

  if (strcmp(argv[1], "-9") == 0) {

    while(cpid[i] != '\0') {
      pid = pid*10 + cpid[i] - '0';
      i++;
    }

    if(pid == 0 || pid == 1 || pid == 2) {
      write(1, str, strlen(str)); 
      exit(1);
    }

    kill((pid_t)pid, 9);
    exit(0);
  }

  exit(1);
}

