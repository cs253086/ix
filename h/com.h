/* System calls. */
#define SEND               1    /* function code for sending messages */
#define RECEIVE            2    /* function code for receiving messages */
#define BOTH               3    /* function code for SEND + RECEIVE */
#define ANY   (NR_PROCS+100)    /* receive(ANY, buf) accepts from any source */

/* Task numbers, function codes and reply codes. */
#define HARDWARE          -1    /* used as source on interrupt generated msgs */

#define CLOCK             -3    /* clock class */
#       define SET_ALARM   1    /* fcn code to CLOCK, set up alarm */
#       define CLOCK_TICK  2    /* fcn code for clock tick */
#       define GET_TIME    3    /* fcn code to CLOCK, get real time */
#       define SET_TIME    4    /* fcn code to CLOCK, set real time */
#       define REAL_TIME   1    /* reply from CLOCK: here is real time */

#define MEM               -4    /* /dev/ram, /dev/(k)mem and /dev/null class */
#       define RAM_DEV     0    /* minor device for /dev/ram */
#       define MEM_DEV     1    /* minor device for /dev/mem */
#       define KMEM_DEV    2    /* minor device for /dev/kmem */
#       define NULL_DEV    3    /* minor device for /dev/null */

#define FLOPPY            -5    /* floppy disk class */
#define WINCHESTER        -6    /* winchester (hard) disk class */
#       define DISKINT     1    /* fcn code for disk interupt */
#       define DISK_READ   3    /* fcn code to DISK (must equal TTY_READ) */
#       define DISK_WRITE  4    /* fcn code to DISK (must equal TTY_WRITE) */
#       define DISK_IOCTL  5    /* fcn code for setting up RAM disk */

/* Names of message fields used for messages to block and character devices(tasks). */
#define DEVICE         m2_i1    /* major-minor device */
#define PROC_NR        m2_i2    /* which (proc) wants I/O? */
#define COUNT          m2_i3    /* how many bytes to transfer */
#define POSITION       m2_l1    /* file offset */
#define ADDRESS        m2_p1    /* core buffer address */

/* Names of message fields for messages to CLOCK task. */
#define DELTA_TICKS    m6_l1    /* alarm interval in clock ticks */
#define FUNC_TO_CALL   m6_f1    /* pointer to function to call */
#define NEW_TIME       m6_l1    /* value to set clock to (SET_TIME) */
#define CLOCK_PROC_NR  m6_i1    /* which proc (or task) wants the alarm? */
#define SECONDS_LEFT   m6_l1    /* how many seconds were remaining */

/* Names of messages fields used in reply messages from tasks. */
#define REP_PROC_NR    m2_i1    /* # of proc on whose behalf I/O was done */
#define REP_STATUS     m2_i2    /* bytes transferred or error number */

