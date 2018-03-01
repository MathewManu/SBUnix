#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]) {
	ps();
  write(1, "\n", 1);
	exit(0);
}

