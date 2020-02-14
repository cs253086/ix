/* Test purpose dummy task */
#include "proc.h"
#include "glo.h"

dummy_task()
{
        printf("Dummy task is working...: %d\n", cur_proc);
        unready(proc_ptr);
        restart();
}

dummy()
{
        printf("dummy is running called from dummy_int in mpx88.s\n");
}
  
