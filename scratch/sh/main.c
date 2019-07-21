//http://rik0.altervista.org/snippets/csimpleshell.html

#include <std.h>
/*
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
*/
#define BUFFER_SIZE	4096 
#define ARR_SIZE	16 

void parse_args(char *buffer, char** args, int args_size, int *nargs);

#if 0
int main()
{
    char buffer[BUFFER_SIZE];
    char *args[ARR_SIZE];
	
    int *ret_status;
    size_t nargs;
    pid_t pid;
    
    while(1){
        printf("$ ");
        fgets(buffer, BUFFER_SIZE, stdin);
        parse_args(buffer, args, ARR_SIZE, &nargs); 

        if (nargs==0) continue;
        if (!strcmp(args[0], "exit" )) exit(0);       
        pid = fork();
        if (pid){
            printf("Waiting for child (%d)\n", pid);
            pid = wait(ret_status);
            printf("Child (%d) finished\n", pid);
        } else {
            if( execvp(args[0], args)) {
                puts(strerror(errno));
                exit(127);
            }
        }
    }    
    return 0;
}
#endif

int main()
{

	char buffer[BUFFER_SIZE];
	char *args[ARR_SIZE];
	int nargs;
	int status;

	while (1)
	{
		printf("$ ");	

        	fgets(buffer, BUFFER_SIZE, stdin);
        	parse_args(buffer, args, ARR_SIZE, &nargs); 

		if (fork())
		{
			//printf("parent: waiting for child...\n");
			wait(status);
			//printf("parent: child exited...\n");
		}
		else
		{
			//printf("child: %s executed\n", args[0]);
			exec(args[0]);
			/* TODO: executing exit() automatically without calling it manually. One way is Global Constructor: http://wiki.osdev.org/Calling_Global_Constructors */
			exit(0);
		}
	}
}

void parse_args(char *buffer, char** args, int args_size, int *nargs)
{
#if 0
	char *buf_args[args_size]; /* You need C99 */
	char **cp;
	char *wbuf;
	int i, j;

	wbuf = buffer;
	buf_args[0] = buffer; 
	args[0] = buffer;

	for(cp = buf_args; (*cp = strsep(&wbuf, " \n\t")) != NULL ;)
	{
		if ((*cp != '\0') && (++cp >= &buf_args[args_size]))
	    		break;
	}

	for (j = i = 0; buf_args[i] != NULL; i++)
	{
		if(strlen(buf_args[i]) > 0)
	    		args[j++] = buf_args[i];
	}

	*nargs = j;
	args[j] = NULL;
#endif
	int i = 0;

	while (buffer[i] != '\n')
	{
		args[0][i] = buffer[i];
		i++;
	}
	args[0][i] = '\0';
}
