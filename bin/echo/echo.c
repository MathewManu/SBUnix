#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
	char *space = " ";
	char *nwline = "\n";
//	char *ptr1 = "hello";
	int i = 1;
	while (i < argc) {
          write(1, argv[i], strlen(argv[i]));
          write(1, space, 1);
	  i++;
	}
        write(1, nwline, 1);
	exit(0);
}

