/* This file contains the drivers for four special files:
 *     /dev/null	- null device (data sink)
 *     /dev/mem		- absolute memory
 *     /dev/kmem	- kernel virtual memory
 *     /dev/ram		- RAM disk
 * It accepts three messages, for reading, for writing, and for
 * control. All use message format m2 and with these parameters:
 *
 *    m_type      DEVICE    PROC_NR     COUNT    POSITION  ADRRESS
 * ----------------------------------------------------------------
 * |  DISK_READ | device  | proc nr |  bytes  |  offset | buf ptr |
 * |------------+---------+---------+---------+---------+---------|
 * | DISK_WRITE | device  | proc nr |  bytes  |  offset | buf ptr |
 * |------------+---------+---------+---------+---------+---------|
 * | DISK_IOCTL | device  |         |  blocks | ram org |         |
 * ----------------------------------------------------------------
 *  
 *
 * The file contains one entry point:
 *
 *   mem_task:	main entry when system is brought up
 *
 */

#include "../h/syscall_nr.h"
#include "../h/com.h"
#include "../h/error.h"
#include "proc.h"

#define NR_RAM_TYPES            4	/* number of RAM-type devices */

static message mess;		/* message buffer */
static phys_bytes ram_origin[NR_RAM_TYPES];	/* origin of each RAM disk  */
static phys_bytes ram_limit[NR_RAM_TYPES];	/* limit of RAM disk per minor dev. */

/*===========================================================================*
 *				mem_task				     * 
 *===========================================================================*/
mem_task()
{
/* Main program of the RAM driver task. */

  int r, caller, proc_nr;
  extern unsigned sizes[8];
  extern phys_clicks get_base(); /* This function is defined in /lib/libsrc/getutil.s and it returns click at which prog starts */


  /* Initialize this task. */
  ram_origin[KMEM_DEV] = (phys_bytes) get_base() << CLICK_SHIFT;	/* ram_origin[KMEM_DEV] = 1536 */
  ram_limit[KMEM_DEV] = (sizes[0] + sizes[1]) << CLICK_SHIFT;	/* total size of kernel(text and data) Bug21*/
  ram_limit[MEM_DEV] = MEM_BYTES;	/* 0~640K */

  /* Here is the main loop of the memory task.  It waits for a message, carries
   * it out, and sends a reply.
   */

#ifdef DEBUG
	printf("mem_task initialized...\n");
#endif

  while (TRUE) {
	/* First wait for a request to read or write. */
	receive(ANY, &mess);
	if (mess.m_source < 0)	/* m_source should be FS */
		panic("mem task got message from ", mess.m_source);
	caller = mess.m_source;	/* actual caller, possibly, system calls(user program) */
	proc_nr = mess.PROC_NR;	/* who wants this I/O. It can be different from caller */

	/* Now carry out the work.  It depends on the opcode. */
	switch(mess.m_type) {
	    case DISK_READ:	r = do_mem(&mess);	break;
	    case DISK_WRITE:	r = do_mem(&mess);	break;
	    case DISK_IOCTL:	r = do_setup(&mess);	break;
	    default:		r = EINVAL;		break;
	}

	/* Finally, prepare and send the reply message. */
	mess.m_type = TASK_REPLY;	/* Reply o FS */
	mess.REP_PROC_NR = proc_nr;
	mess.REP_STATUS = r;
	send(caller, &mess);
  }
}

/*===========================================================================*
 *                              do_mem                                       * 
 *===========================================================================*/
static int do_mem(m_ptr)
register message *m_ptr;        /* pointer to read or write message */
{
/* Read or write /dev/null, /dev/mem, /dev/kmem, or /dev/ram. */

  int device, count;
  phys_bytes mem_phys, user_phys;
  struct proc *rp;
  extern phys_clicks get_base();
  extern phys_bytes umap();

  /* which RAM type is it? */
  device = m_ptr->DEVICE;
  if (device < 0 || device >= NR_RAM_TYPES) return(ENXIO);   /* ENXIO: IO ERROR */
  if (device==NULL_DEV) return(m_ptr->m_type == DISK_READ ? EOF : m_ptr->COUNT);	/* /dev/null */

  /* Set up 'mem_phys' for /dev/mem, /dev/kmem, or /dev/ram. */
  if (m_ptr->POSITION < 0) return(ENXIO);	
  mem_phys = ram_origin[device] + m_ptr->POSITION;
  if (mem_phys >= ram_limit[device]) return(EOF);
  /* how many bytes to transfer */
  count = m_ptr->COUNT;
  if(mem_phys + count > ram_limit[device]) count = ram_limit[device] - mem_phys;

  /* Determine address where data is to go or to come from. */
  rp = proc_addr(m_ptr->PROC_NR);
  user_phys = umap(rp, D, (vir_bytes) m_ptr->ADDRESS, (vir_bytes) count);
  if (user_phys == 0) return(E_BAD_ADDR);

  /* Copy the data. */
  if (m_ptr->m_type == DISK_READ)
        phys_copy(mem_phys, user_phys, (long) count);
  else	/* DISK_WRITE */
        phys_copy(user_phys, mem_phys, (long) count);
  return(count);
}

/*===========================================================================*
 *                              do_setup                                     * 
 *===========================================================================*/
static int do_setup(m_ptr)
message *m_ptr;                 /* pointer to read or write message */
{
/* Set parameters for one of the disk RAMs. */

  int device;

  device = m_ptr->DEVICE;
  if (device < 0 || device >= NR_RAM_TYPES) return(ENXIO);   /* bad minor device */
  ram_origin[device] = m_ptr->POSITION;
  ram_limit[device] = m_ptr->POSITION + (long) m_ptr->COUNT * BLOCK_SIZE;
  return(OK);
}

