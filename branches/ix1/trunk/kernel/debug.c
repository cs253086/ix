#include "debug.h"

/* print function for debugging */
debug_print(var_name, type, var)
char *var_name;	/* variable name */
char type;	/* data type: int, long, etc. */
void *var;	/* actual variable */
{
	debug_title();
	/* Determine which type it is, and printf */
	switch (type) {
		case 'c':
			printf("%s: %c\n", *((char *) var));
			break;
		case 'd':
			printf("%s: %d\n", *((int *) var));
			break;
		case 'D':
			printf("%s: %D\n", *((long *) var));	
			break;
		case 'u':
			printf("%s: %u\n", *((unsigned int *) var));
			break;
		case 'U':
			printf("%s: %U\n", *((unsigned long *) var));
			break;
		case 's':
			printf("%s: %s\n", *((char **) var));
			break;
	}
}

debug_a_procs()
{
	int i;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
		debug_title();
		debug_item(proc[i]);
	}
	
}

debug_item(p)
struct proc p;
{
	int i;
	printf("p_regs: ");
	for (i = 0; i < NR_REGS; i++) {
		printf("%d, ", p.p_reg[i]);
	}
	printf("\n");
	/*
	int p_reg[NR_REGS];     
        int *p_sp;      
        struct pc_psw p_pcpsw;  
        int p_flags;    
        struct mem_map p_map[NR_SEGS];  
        int *p_splimit; 
        int p_pid;      

        real_time user_time;    
        real_time sys_time;     
        real_time child_utime; 
        real_time child_stime;  
        real_time p_alarm;      

        struct proc *p_callerq; 
        struct proc *p_sendlink;      
        message *p_messbuf;     
        int p_getfrom;  

        struct proc *p_nextready;     
        int p_pending; 
	*/

}
debug_title()
{
	printf("[DEBUG] ");
}
