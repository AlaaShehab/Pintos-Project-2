#include "userprog/syscall.h"
#include "userprog/sysrout.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/usrmem.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  init_routine();
}

static void
syscall_handler (struct intr_frame *f)
{
  int32_t * sp = f->esp;
  uint32_t sys_id;
  if (!read_user((void *)sp, 4, (void *)&sys_id)) {
  	exit_routine(-1);
  }
  uint32_t arg1, arg2, arg3;
  if (sys_id != SYS_HALT) {
  	if (!read_user((void *)(sp + 1), 4, (void *) &arg1)) {
  		exit_routine(-1);
  	}
  	if (sys_id == SYS_CREATE || sys_id == SYS_SEEK || sys_id == SYS_READ || sys_id == SYS_WRITE) {
  		if (!read_user((void *)(sp + 2), 4, (void *) &arg2)) {
  			exit_routine(-1);
  		}
  		if (sys_id == SYS_READ || sys_id == SYS_WRITE) {
  			if (!read_user((void *)(sp + 3), 4, (void *) &arg3)) {
  				exit_routine(-1);
  			}
  		}
  	}
  }
  bool no_ret = false;
  int32_t ret;
  switch (sys_id) {
  	case  SYS_HALT:                   /* Halt the operating system. */
  		halt_routine();
  		no_ret = true;
  		break;
    case SYS_EXIT:                   /* Terminate this process. */
    	exit_routine((int)arg1);
    	no_ret = true;
    	break;
    case SYS_EXEC:                   /* Start another process. */
    	ret = exec_routine((const char *)arg1);
    	break;
    case SYS_WAIT:                   /* Wait for a child process to die. */
    	ret = wait_routine((pid_t)arg1);
    	break;
    case SYS_CREATE:                 /* Create a file. */
    	ret = create_routine((const char *)arg1, (unsigned)arg2);
    	break;
    case SYS_REMOVE:                 /* Delete a file. */
    	ret = remove_routine((const char *)arg1);
    	break;
    case SYS_OPEN:                   /* Open a file. */
    	ret = open_routine((const char *)arg1);
    	break;
    case SYS_FILESIZE:               /* Obtain a file's size. */
    	ret = filesize_routine((int)arg1);
    	break;
    case SYS_READ:                   /* Read from a file. */
    	ret = read_routine((int)arg1, (void *)arg2, (unsigned)arg3);
    	break;
    case SYS_WRITE:                  /* Write to a file. */
    	ret = write_routine((int)arg1, (const void *)arg2, (unsigned)arg3);
    	break;
    case SYS_SEEK:                   /* Change position in a file. */
    	seek_routine((int) arg1, (unsigned) arg2);
    	no_ret = true;
    	break;
    case SYS_TELL:                   /* Report current position in a file. */
    	ret = tell_routine((int)arg1);
    	break;
    case SYS_CLOSE:                  /* Close a file. */
		close_routine((int)arg1);
		no_ret = true;
		break;
  }
  if (!no_ret) {
  	f->eax = ret;
  }
}
