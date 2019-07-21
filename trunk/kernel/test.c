#include <stdio.h>
#include "common.h"
#include "../h/test_print.h"

/* Test1: kernel flow */

/* Test2: ready queue */

/* Test3: current_proc_id */
void test_current_proc_id(int exp)
{
#ifdef TEST
	if (cur_proc_id == exp)
		test_print_pass(__FUNCTION__);
	else
		test_print_fail(exp, cur_proc_id, __FUNCTION__);
#endif 
}

