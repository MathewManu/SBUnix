#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/tarfs.h>

#define CMD_UNKNOWN      0
#define CMD_CD           1
#define CMD_CWD          2
#define CMD_LS           3
#define CMD_EXIT         4

#define CMD_LEN          1024
#define SBUSH_HASHBANG   "#!/bin/sbush"

typedef struct piped_commands {
  char *commands[50];
} piped_commands;

struct linux_dirent64 {
  unsigned long  d_ino;    /* 64-bit inode number */
  unsigned long  d_off;    /* 64-bit offset to next structure */
  unsigned short d_reclen; /* Size of this dirent */
  unsigned char  d_type;   /* File type */
  char           d_name[]; /* Filename (null-terminated) */
};

char buff[CMD_LEN] = {0};
char **m_environ;
//int is_bg;

int do_execute(char *cmd, char *cmd_path[], char *env[]); 


char *getenv(const char *name);
int setenv(char *name, char *value, int overwrite);
char *getcwd(char *buf, size_t size);
int  puts_nonewline(const char *s);

int  get_command(char *cmd);
void handle_cd(char *path);
void handle_cwd();
void handle_ls();
void execute_non_builtin(char *cmd, char *cmd_arg);
void execute_command_line(char *cmd);
void execute_commands(char *cmd, char *cmd_arg);
void read_and_exec_from_file(char *filename, int type);
void execute_command_sbunix(char *buff);

char ps1_variable[256] = "sbush> ";
char path_variable[256] = "/rootfs/bin";

int execute_piped_commands(int num_pipes, piped_commands *cmds);

int bg_proc;
char glob_cmd[64];
char *arg_vv[6]; 

/*
 * if & is found, replace with '\0' and return true
 */ 
int update_if_bg_cmdarg(char *cmd_arg) {
  char *amp;
  int ret = 0;
  if ((amp = strstr(cmd_arg, "&")) != NULL) {
    *amp = '\0'; 
    ret = 1;
  }

  return ret; 
}

void update_cmd(char *buff, char *cmd, char *arg[]) {

  char *sep = " ";
  int i = 0, j = 0;
  char *saveptr;

  char arr[255] = {0};
  strcpy(arr, buff);

  char *token = strtok_r(arr, sep, &saveptr);
  while (token != NULL && i < 11) {

    if(i == 0) {
      strcpy(cmd, token);
      token = strtok_r(NULL, sep, &saveptr);
      i++;
      continue;
    }
    strcpy(arg[j], token);
    token = strtok_r(NULL, sep, &saveptr);
    i++;
    j++;
  }
 // if (update_if_bg_cmdarg(buff))
  //  is_bg = 1;
}

void print_prompt() {
  puts(ps1_variable);
}

int getdents64(int fd, struct linux_dirent64 *dirp, int count) {
  return syscall(__NR_getdents64, fd, dirp, count);
}

int puts_nonewline(const char *s) {
  int ret;
  while (*s) {
    if ((ret = putchar(*s)) != *s)
      return EOF;
    s++;
  } 
  return 0;
}

int tokenize(char *arg, char *argv[], int max_tokens, char *sep); 

int find_path_and_exe(char *cmd, char *argv[], char *env[]) {

  char *slash = strstr(cmd, "/");
  /* commands like ls */
  if (slash == 0) {

    return do_execute(cmd, argv, env);
  }

  return -1;
}

int do_execute(char *cmd, char *argv[], char *env[]) {

  char *path_env = getenv("PATH");
  char *paths[50] = {0};
  char exe_path[255] = {0};

  int i, len, ret = 1;
  int num_paths  = tokenize(path_env + 5, paths, 50, ":");
  /* 
   * iterate and find out the path of cmd
   */
  for (i = 0; i < num_paths; i++) {
    len = strlen(paths[i]);

    strncpy(exe_path, paths[i], len);    
    strncpy(exe_path + len, "/", 1);
    strcpy(exe_path + len + 1, cmd);
  }

  if (i == num_paths)
    ret = -1; 
  return ret;
}

int tokenize(char *arg, char *argv[], int max_tokens, char *sep) {
  int i = 0;
  char *saveptr;
  char arr[255] = {0};
  strcpy(arr, arg);
  char *token = strtok_r(arr, sep, &saveptr);
  while (token != NULL && i < max_tokens - 1) {
    argv[i] = malloc(strlen(token) + 1);
    strcpy(argv[i], token);
    token = strtok_r(NULL, sep, &saveptr);
    i++;
  }

  argv[i] = NULL;
  return i;
}

void build_argv(char *input, char *arg, char *argv[]) {
  argv[0] = malloc(strlen(input) + 1);
  strcpy(argv[0], input);
  tokenize(arg, &argv[1], 49, " ");
}

void handle_cd(char *path) {
  
  int ret;
  ret = chdir(path);
  if (ret != 0) {
    puts_nonewline("sbush: cd: ");
    puts_nonewline(path); 
    puts(": No such file or directory\n"); 
  }
}

void handle_cwd() {
  char buff[CMD_LEN] = {0};
  if (getcwd(buff, sizeof(buff)) != NULL)
    puts(buff);
}

void handle_ls() {

	char buff[CMD_LEN] = {0};
	if (getcwd(buff, sizeof(buff)) == NULL)
		return;

	int fd;
	int ret;
	int i = 0;
	char buf[CMD_LEN];
	struct linux_dirent64 *d_ent;
	fd = open(buff, O_RDONLY);
	ret = getdents64(fd, (struct linux_dirent64 *)buf, CMD_LEN);

	while (i < ret) {
		d_ent = (struct linux_dirent64 *) (buf + i);
		if ((d_ent->d_name)[0] != '.')
			puts(d_ent->d_name);

		i += d_ent->d_reclen;
	}

}

int get_command(char *cmd) {

  /* built-in */
  if (strcmp(cmd, "cd") == 0)
    return CMD_CD;

  /* built-in */
  else if (strcmp(cmd, "exit") == 0)
    return CMD_EXIT;

  else
    return CMD_UNKNOWN;
}

void get_path_string(char *cmd, char *path_value) {

  char *ptr = NULL;
  if ((strstr(cmd, "$PATH")) != NULL) {
    /*
     * 2 cases. eg: PATH=$PATH:/bin:/usr/bin 
     * $PATH anywhere else. beginning or somewhere else
     */ 
    char *path = strstr(cmd, "=");
    path++; 
    char *temp = path;
    int len = strlen(path);

    ptr = strstr(path, "$PATH");

    char *sys_env = getenv("PATH");

    /* $PATH in the beginning */
    if (temp == ptr) {
      strncpy(path_value, sys_env, strlen(sys_env));
      if (len - 6 > 0) { //6 len of $PATH:
        strcpy(path_value+strlen(sys_env), ptr+5);
      } 
    } else {
      /* $PATH anywhere else */
      strncpy(path_value, temp, ptr-temp); 
      strncpy(path_value + (ptr - temp), sys_env, strlen(sys_env));
      strcpy(path_value + (ptr - temp) + strlen(sys_env), ptr + 5);
    }

  } else {
    /* eg: PATH=/usr/bin */ 
    ptr = strstr(cmd, "=");
    strcpy(path_value, ptr + 1); 
  }
}

int check_if_path_cmd(char *cmd) {
  return strncmp(cmd, "PATH=", 5) == 0 ? 1 : 0; 
}

int check_if_ps1_cmd(char *cmd) {
  return strncmp(cmd, "PS1=", 4) == 0 ? 1 : 0; 
}


/* Not shell built-in commands, call exec */
void execute_non_builtin(char *cmd, char *cmd_arg) {
  pid_t pid;
  int i, status, bg_process = 0;
  char *argv[50] = {0};
  char path_value[CMD_LEN] = {0} ; 
  
  /* PATH variable set */
  if (check_if_path_cmd(cmd)) {
    get_path_string(cmd, path_value); 
    setenv("PATH", path_value, 1);
    return;
  }
  /* PS1 variable set */ 
  else if (check_if_ps1_cmd(cmd)) {
    strcpy(ps1_variable, strstr(cmd, "=") + 1);
    return;
  }
  /* command & handling, true if & is found in the command */
  else if (update_if_bg_cmdarg(cmd_arg)) {
    bg_process = 1;
  }
 
  build_argv(cmd, cmd_arg, argv);

  pid = fork();
  if (pid == 0) {
     if (find_path_and_exe(cmd, argv, m_environ) < 0) {
      puts_nonewline(cmd);
      puts(": command not found");
      exit(1);
    }
  } else {
    if (pid < 0) {
      exit(1);
    }
    else {
      if (!bg_process)
        waitpid(-1, &status);
    }
  }

  /* Freeing */
  i = 0;
  while (argv[i]) {
    free(argv[i]);
    argv[i] = NULL;
    i++;
  }
}

void execute_command_line(char *cmd) {
  int i;
  char *str, *token, *saveptr;
  for (i = 1, str = cmd; ; i++, str = NULL) {

    token = strtok_r(str, " ", &saveptr);
    if (token == NULL)
      break;

    if (i == 1 && token[0] != '#')
      execute_commands(token, saveptr);
  }
}

void execute_commands(char *cmd, char *cmd_arg) {

  int cmd_id = get_command(cmd);
  switch (cmd_id) {

    case CMD_CD:
      handle_cd(cmd_arg);
      break;

    case CMD_EXIT:
      exit(0);

    case CMD_UNKNOWN:
      execute_non_builtin(cmd, cmd_arg);
      break;

    default:
      puts_nonewline(cmd);
      puts(": command not found");
      break;
  }
}

int valid_command(char *buff, int size) {

  char cmd[CMD_LEN] = {0};
  char cwd[CWD_LEN] = {0};
  char tmp[CMD_LEN] = {0};
  char *bin_dir = "bin/";

  memset(cwd, 0, sizeof(cwd));
  getcwd(cwd, sizeof(cwd));
  //kprintf("cwd = [%s]\n", cwd);

  //puts("BUFF :");
  //puts(buff);
  //puts("\n");
  if (buff[0] == '.' && buff[1] == '/') {
    puts("sbush : not a valid sbush script\n");
    return 0;

  } else if (buff[0] != '/') {

    memset(tmp, 0, sizeof(tmp));
    strcpy(tmp, &cwd[8]);
    strcpy(tmp + strlen(tmp), buff);

    //puts("TT2\n");
    //puts(tmp);
    //puts("\n");
    //kprintf("TT = [%s]\n", tmp);
    char *sep = " ";
    char *saveptr;
    char arr[255] = {0};
    strcpy(arr, tmp);
    char *token = strtok_r(arr, sep, &saveptr);
    if (token && !validexe(token)) {
      memset(tmp, 0, sizeof(tmp));
      strcpy(tmp, &path_variable[8]);
      strcpy(tmp + strlen(tmp), "/");
      strcpy(tmp + strlen(tmp), buff);
      //puts("TT3\n");
      //puts(tmp);
      //puts("\n");
      char *sep = " ";
      char *saveptr;
      char arr[255] = {0};
      strcpy(arr, tmp);
      char *token = strtok_r(arr, sep, &saveptr);
      if (token && validexe(token)) {
        memset(cmd, 0, sizeof(cmd));
        strcpy(cmd, tmp);
        //puts("CMD\n");
        //puts(cmd);
        //puts("\n");

      } else {

        memset(cmd, 0, sizeof(cmd));
        strcpy(cmd, bin_dir);
        strcpy(cmd + strlen(bin_dir), buff);
      }
      //puts("AA\n");
      //puts(cmd);
      //puts("\n");
    } else {

      memset(cmd, 0, sizeof(cmd));
      strcpy(cmd, tmp);
      //puts("BB\n");
      //puts(cmd);
      //puts("\n");
    }

  } else {

    memset(cmd, 0, sizeof(cmd));
    strcpy(cmd, &buff[8]);
    //puts("CC\n");
    //puts(cmd);
    //puts("\n");
  }

  memset(buff, 0, size);
  strcpy(buff, cmd);

  char *sep = " ";
  char *saveptr;
  char arr[255] = {0};
  strcpy(arr, cmd);
  char *token = strtok_r(arr, sep, &saveptr);
  if (token && validexe(token)) {
    //puts("VALID FILE\n");
    return 1;
  }

 return 0;
}

/* Read from the file one line at a time and execute */
void read_and_exec_from_file(char *filename, int type) {
  //puts("hhh\n");

  if ((type == 1 && filename[0] != '/') || (type == 2 && strncmp(filename, "/rootfs", strlen("/rootfs")))) {
    puts("sbush : give absolute path\n");
    return;
  } else {
    int file = open(filename, O_RDONLY);
    if (file == -1) {
      puts("sbush : No such file or directory\n"); 
      return;
    }
    char code[CMD_LEN] = {0};
    if (read(file, code, 12) > 0) {
      if (strncmp(code, "#!/bin/sbush", strlen("#!/bin/sbush"))) {
        puts("sbush : file not beginning with #!/bin/sbush\n");
        close(file);
        return;
      }
    }
    close(file);
  }

  int file = open(filename, O_RDONLY);
  char code[CMD_LEN] = {0};
  size_t n = 0;
  char c;
  uint8_t hash_read = 0;

  if (file == -1) {
    puts("sbush : No such file or directory\n"); 
    return;
  }

  while (read(file, &c, 1) > 0)
  {
    code[n++] = (char) c;
    if (c == '\n') {
      code[n - 1] = '\0';

      if (!hash_read) {
        if (!strncmp(code, SBUSH_HASHBANG, strlen(SBUSH_HASHBANG))) {
          hash_read = 1;
          n = 0;
        }
      } else {
        if (strlen(code)) {
          if (valid_command(code, strlen(code))) {
            execute_command_sbunix(code);
          } else {
            close(file);
            return;
          }
        }
        n = 0;
      }
    }
  }

  code[n] = '\0'; 
  if (strlen(code) && valid_command(code, strlen(code))) {
	  execute_command_sbunix(code);
  }

  close(file);
}

void handle_piped_commands(char *arg) {
  char *argv[50] = {0};
  int i = 0;
  int num_cmds  = tokenize(arg, &argv[0], 50, "|");
  int num_pipes = num_cmds - 1;
  piped_commands cmds[num_cmds];
  memset(cmds, 0, sizeof(cmds));

  while (i < num_cmds) {
    tokenize(argv[i], cmds[i].commands, 50, " ");
    i++;
  }
 
  execute_piped_commands(num_pipes, cmds);

  /* Freeing */
  i = 0;
  while (i < num_cmds) {
    int j = 0;
    while(cmds[i].commands[j]) {
      free(cmds[i].commands[j]);
      cmds[i].commands[j] = NULL;
      j++;
    }
    i++;
  }
  i = 0;
  while(argv[i]) {
    free(argv[i]);
    argv[i] = NULL;
    i++;
  }
}

void read_from_stdin() {
  int cnt;
  char *str, *saveptr, *token;
  char buff[CMD_LEN] = {0};
  while (read(0, buff, sizeof(buff)) > 0) {

    cnt = 1;
    size_t buff_length = strlen(buff);
    if (buff_length != 0 && buff[buff_length - 1] == '\n') {

      buff_length--;
      buff[buff_length] = '\0';
    }

    if (strstr(buff, "|")) {
      handle_piped_commands(buff);
    }  else {
      str = buff;
      while (1) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
          break;

        if (cnt == 1)
          execute_commands(token, saveptr);

        cnt++;
        str = NULL;
      }
    }

    print_prompt();

    memset(buff, 0, sizeof(buff));
  }
}

int process_start(int input_fd, int output_fd, piped_commands *cmds) {
  int status;
  pid_t pid = fork();
  if (pid == 0) {

    if (input_fd != 0) {
      dup2(input_fd, 0);
      close(input_fd);
    }

    if (output_fd != 1) {
      dup2(output_fd, 1);
      close(output_fd);
    }
    return find_path_and_exe(cmds->commands[0], cmds->commands, m_environ);

  } else {
    waitpid(-1, &status);
  }

  return pid;
}

int execute_piped_commands(int num_pipes, piped_commands *cmds) {
  int i = 0;
  int input_fd = 0; /* stdin */
  int fds[2];
  int status;
  pid_t pid;

  while (i < num_pipes) {
    if (pipe(fds) != 0)
      return 1;

    process_start(input_fd, fds[1], cmds + i);
    close(fds[1]);

    input_fd = fds[0];
    i++;
  }

  /* last command */
  pid = fork();
  if (pid == 0) {

    dup2(input_fd, 0);
    close(input_fd);

    if (find_path_and_exe(cmds[i].commands[0], cmds[i].commands, m_environ)) {
      exit(1);
    }

  } else {
    waitpid(-1, &status);
  }

  return 0;
}

char *getenv(const char *arg) {
  int i;
  for (i = 0; m_environ[i] !=0 ; i++) {
    if (strncmp(m_environ[i], "PATH=", 5) == 0) { 
      return m_environ[i];
    } 
  }
 return NULL;
}

int setenv(char *path_variable, char *value, int overwrite) {
  
  /* eg: setenv("PATH", path_value, 1); 
   * overwrite variable is not used now.*/

  int i;
  int var_len = strlen(path_variable);
  int value_len = strlen(value);

  for (i = 0; m_environ[i] !=0 ; i++) {

    if (strncmp(m_environ[i], "PATH=", 5) == 0) { 
      /*
       * first free the value. Then allocate for new value
       */
      free(m_environ[i]);
      m_environ[i]= malloc(value_len + var_len + 2);//include size of "=" as well.

      /*
       * copy the complete value in 3 steps. eg: PATH=/usr/bin
       */
      strncpy(m_environ[i], path_variable, var_len);	
      strncpy(m_environ[i] + var_len, "=", 1);	
      strcpy(m_environ[i] + var_len + 1, value);

      return 0;
    }
  }
  return 1;
}


void set_path_variable(char *newpath) {
  memset(path_variable, 0, sizeof(path_variable));
  if (strlen(newpath) >= strlen("/rootfs") && !strncmp(newpath, "/rootfs", strlen("/rootfs"))) {
    strcpy(path_variable, newpath);
  } else {
    puts("sbush : give absolute path\n");
  }
}

int process_script(char *buff, int size) {
  uint8_t script_handling_len = 0;

  if (!strncmp(buff, "sbush ", 6)) {
    script_handling_len = 6;
  } else if (buff[0] == '.' && buff[1] == '/') {
    script_handling_len = 1;
  } 
    
  if (script_handling_len) {

    char *sep = " ";
    char *saveptr;
    char arr[CMD_LEN] = {0};
    strcpy(arr, buff + script_handling_len);

    char *token = strtok_r(arr, sep, &saveptr);
    if (token) {
      if (script_handling_len == 6)
        read_and_exec_from_file(token, 1);
      else
        read_and_exec_from_file(token, 2);
    }

    return 1;
  }

  return 0;
}

void execute_command_sbunix(char *buff){

  int ret;
  memset(glob_cmd, 0, sizeof(glob_cmd));
  memset(arg_vv, 0, sizeof(arg_vv));

  arg_vv[0] = malloc(4096);
  arg_vv[1] = malloc(4096);
  arg_vv[2] = malloc(4096);
  arg_vv[3] = malloc(4096);
  arg_vv[4] = malloc(4096);
  arg_vv[5] = malloc(4096);

  update_cmd(buff, glob_cmd, arg_vv);

  ret = fork();
  if(ret == 0) {
#if 0
    if(strcmp(glob_cmd, "sleep") == 0)
      bg_proc = 0;
    else
      bg_proc = 0;
#endif
    execvpe(glob_cmd, &arg_vv[0], NULL);
    memset(buff, 0, sizeof(buff));
  }
  else {
   // if(!is_bg) {
      int st = 0;
      wait(&st);

   // }
    memset(buff, 0, sizeof(buff));

    free(arg_vv[0]);
    free(arg_vv[1]);
    free(arg_vv[2]);
    free(arg_vv[3]);
    free(arg_vv[4]);
    free(arg_vv[5]);
  }
}

int main(int argc, char *argv[], char *envp[]) {

  char path_value[CMD_LEN] = {0} ; 

  while(1) {
    memset(buff, 0, sizeof(buff));
    print_prompt();

    if (read(0, buff, CMD_LEN)) {
      puts(buff);
      puts("\n");
      if (check_if_ps1_cmd(buff)) {
        strcpy(ps1_variable, strstr(buff, "=") + 1);
      }
      else if (check_if_path_cmd(buff)) {
        memset(path_value, 0, sizeof(path_value));
        get_path_string(buff, path_value); 
        set_path_variable(path_value);
      }
      else {
        if(!process_script(buff, strlen(buff))) {
          //puts("Not Script\n");
          if (valid_command(buff, strlen(buff))) {
            //puts("EXE...\n");
            execute_command_sbunix(buff);
          }
        }
      }
    }
  }
  return 0;
}

