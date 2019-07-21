void system_call(int function, int caller_pid, int src_dst, message_t *msg);
void ready(proc_t *proc_p);
void block(proc_t *proc_p);
void pick_proc();
proc_t * get_proc_addr(int proc_id);
void cp_msg(message_t *src, message_t *dst);
int kernel_send_msg(int caller_pid, int dst, message_t *msg);
int kernel_receive_msg(int caller, int src, message_t *msg);

