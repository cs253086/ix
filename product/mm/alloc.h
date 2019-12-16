void init_mem(uint32_t mem_size);
void debug_print_process_holes();
struct process_hole *del_first();
void add_last(struct process_hole *hole_p);
mm_proc_t *allocate_process_hole();

