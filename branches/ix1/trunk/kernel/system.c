/*
 * TODO
 * 1. enable sig_procs in proc.c 
 */

#include "proc.h"

/*===========================================================================*
 *                              umap                                         * 
 *===========================================================================*/
phys_bytes umap(rp, seg, vir_addr, bytes)
register struct proc *rp;       /* pointer to proc table entry for process */
int seg;                        /* T, D, or S segment */
vir_bytes vir_addr;             /* virtual address in bytes within the seg */
vir_bytes bytes;                /* # of bytes to be copied */
{
/* Calculate the physical memory address for a given virtual address. */
  vir_clicks vc;                /* the virtual address in clicks */
  phys_bytes seg_base, pa;      /* intermediate variables as phys_bytes */

  /* If 'seg' is D it could really be S and vice versa.  T really means T.
   * If the virtual address falls in the gap,  it causes a problem. On the
   * 8088 it is probably a legal stack reference, since "stackfaults" are
   * not detected by the hardware.  On 8088s, the gap is called S and
   * accepted, but on other machines it is called D and rejected.
   */
  if (bytes <= 0) return( (phys_bytes) 0);
  vc = (vir_addr + bytes - 1) >> CLICK_SHIFT;   /* last click of data */

  /* we assume 8088 cpu */
  if (seg != T)
        seg = (vc < rp->p_map[D].mem_vir + rp->p_map[D].mem_len ? D : S);

  if((vir_addr>>CLICK_SHIFT) >= rp->p_map[seg].mem_vir + rp->p_map[seg].mem_len)
        return( (phys_bytes) 0 );
  seg_base = (phys_bytes) rp->p_map[seg].mem_phys;
  seg_base = seg_base << CLICK_SHIFT;   /* segment orgin in bytes */
  pa = (phys_bytes) vir_addr;
  pa -= rp->p_map[seg].mem_vir << CLICK_SHIFT;
  return(seg_base + pa);
}

/*===========================================================================*
 *                              cause_sig                                    * 
 *===========================================================================*/
cause_sig(proc_nr, sig_nr)
int proc_nr;                    /* process to be signalled */
int sig_nr;                     /* signal to be sent in range 1 - 16 */
{
 	/* TODO: implementation */ 
}

