#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
     int i = 0;
     int ret = 0;
     char **list = NULL;

     if (argc < 2) {
	  fprintf(stderr, "Usage: %s <program path and arguments>\n", argv[0]);
	  return -1;
     }

     // get elf section content
     // calculate elf data section by sha256
     // compare

     list = (char**)malloc(sizeof(char*)*argc);
     for (i = 1; i < argc; i++) {
	  list[i-1] = argv[i];
     }
     list[i] = NULL;
     ret = execvp(argv[1], list);
     free(list);
     if (ret != 0) {
	  fprintf(stderr, "Failed to execv: %s\n", strerror(errno));
     }
     return 0;
}
