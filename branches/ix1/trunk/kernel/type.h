/* TODO
 * Usee 'include guard' when developing 
 * 32-bit OS
 */
/* It must contain the informatioin pushed onto the stack by an interrupt */

struct pc_psw {
	int (*pc)();	/* storage for program counter */
			/* TOKNOW: why pc is function has 'int' return type
			 * and no argument 
			 */
	phys_clicks cs;	/* code segment register */
	unsigned psw;	/* program status word */
};
