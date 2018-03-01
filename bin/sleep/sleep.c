#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]) {

	int time = 0;

	char *ctime = argv[1];

	int i = 0;
	//char *str = "sbush: kill: Operation not permitted\n";

	while(ctime[i] != '\0') {
		time = time*10 + ctime[i] - '0';
		i++;
	}
  if (time != 0)
	  sleep(time);
	exit(0);

}


