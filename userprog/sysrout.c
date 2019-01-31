#include <stdbool.h>
#include "userprog/sysrout.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/off_t.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "userprog/process.h"
#include "userprog/usrmem.h"
#include <stdio.h>

static struct lock file_lock;  /* Synchronizes file system access. */

void init_routine (void) {
	lock_init (&file_lock);
}

void halt_routine (void) {
    shutdown_power_off();
}

void exit_routine (int status) {
    struct thread *t = thread_current ();
    t->ret_status = status;
    thread_exit();
}

pid_t exec_routine (const char *file) {
	if (!check_user_string(file)) {
		exit_routine(-1);
	}
	lock_acquire(&file_lock);
	pid_t ret = process_execute(file);
    lock_release(&file_lock);
    return ret;
}

int wait_routine (pid_t id) {
    return process_wait(id);
}

bool create_routine (const char *file, unsigned initial_size) {
	if (!check_user_string(file)) {
		exit_routine(-1);
	}
    lock_acquire(&file_lock);
    bool create = filesys_create(file, initial_size);
    lock_release(&file_lock);
    return create;
}

bool remove_routine (const char *file) {
	if (!check_user_string(file)) {
		exit_routine(-1);
	}
    lock_acquire(&file_lock);
	bool remove = filesys_remove(file);
    lock_release(&file_lock);
    return remove;
}

int open_routine (const char *file) {
	if (!check_user_string(file)) {
		exit_routine(-1);
	}
    struct thread *t = thread_current ();
    struct file_elem *e = malloc(sizeof(struct file_elem));
    lock_acquire(&file_lock);
    struct file *f = filesys_open(file);
    lock_release(&file_lock);
    if (f == NULL) {
        free(e);
        return -1;
    }
    e->file = f;
       struct list_elem *e2;
       int prev=1;
           struct file_elem * f2;

    for (e2 = list_begin (&t->file_elems); !list_empty (&t->file_elems) && e2 != list_end (&t->file_elems); e2 = list_next (e2)) {
        f2 = list_entry(e2, struct file_elem, elem);
        if(f2->fd != prev+1){
            e->fd = prev+1;
            list_insert(&f2->elem, &e->elem);
            return e->fd;
        }
        prev = f2->fd;
    }
    e->fd = t->fd;
    t->fd = t->fd + 1;
    list_push_back (&t->file_elems, &e->elem);
    return e->fd;
}

int filesize_routine (int fd) {
    struct thread *t = thread_current ();
    struct file_elem * f;
    struct list_elem *e;
    for (e = list_begin (&t->file_elems); !list_empty (&t->file_elems) && e != list_end (&t->file_elems); e = list_next (e)) {
        f = list_entry(e, struct file_elem, elem);
        if(f->fd == fd){
    	  lock_acquire(&file_lock);
          int length = file_length(f->file);
          lock_release(&file_lock);
          return length;
        }
    }
    return -1;
}

int read_routine (int fd, void *buffer, unsigned length) {
    if (fd == STDOUT_FILENO) {
    	return -1;
    }
    char * temp_buffer = malloc(length + 1);
	if (fd == STDIN_FILENO){
		lock_acquire(&file_lock);
        unsigned i = 0;
        while (i < length) {
        	temp_buffer[i] = input_getc();
        	if(!write_user(buffer +i, 1, temp_buffer + i)) {
        		lock_release(&file_lock);
				free(temp_buffer);
				exit_routine(-1);
        	}
        	i++;
    	}
	    lock_release(&file_lock);
		free(temp_buffer);
        return length;
	}
    struct thread *t = thread_current ();
    struct file_elem * f;
    struct list_elem * e;
    for (e = list_begin (&t->file_elems); !list_empty (&t->file_elems) && e != list_end (&t->file_elems); e = list_next (e)) {
        f = list_entry(e, struct file_elem, elem);
        if(f->fd == fd){
        	lock_acquire(&file_lock);
            int bytes = file_read(f->file, temp_buffer, length);
            lock_release(&file_lock);
            if (!write_user(buffer, length, temp_buffer)) {
            	free(temp_buffer);
            	exit_routine(-1);
            }
            free(temp_buffer);
            return bytes;
        }
    }
	free(temp_buffer);
	return -1;
}

int write_routine (int fd, const void *buffer, unsigned length) {
	if (fd == STDIN_FILENO) {
		return 0;
	}
	if (fd == STDOUT_FILENO){
		lock_acquire(&file_lock);
        putbuf (buffer, length);
        lock_release(&file_lock);
        return length;
	} else {
		struct thread *t = thread_current ();
		struct file_elem * f;
		struct list_elem * e;
		// User memory access begins
		char * temp_buffer = malloc(length + 1);
		if (!read_user(buffer, length, temp_buffer)) {
			free(temp_buffer);
			exit_routine(-1);
		}
		free(temp_buffer);
		// User memory access ends
		for (e = list_begin (&t->file_elems); !list_empty (&t->file_elems) && e != list_end (&t->file_elems); e = list_next (e)) {
        	f = list_entry(e, struct file_elem, elem);
        	if(f->fd == fd){
        		lock_acquire(&file_lock);
          		int bytes = file_write(f->file, buffer, length);
          		lock_release(&file_lock);
          		return bytes;
        	}
    	}
	}
	return 0;
}

void seek_routine (int fd, unsigned position) {
	struct thread *t = thread_current();
    struct file_elem * f;
    struct list_elem * e;
    for (e = list_begin (&t->file_elems); !list_empty (&t->file_elems) && e != list_end (&t->file_elems); e = list_next (e)) {
        f = list_entry(e, struct file_elem, elem);
        if(f->fd == fd){
        	lock_acquire(&file_lock);
         	file_seek(f->file , position);
         	lock_release(&file_lock);
        }
    }

}

unsigned tell_routine (int fd) {

    struct thread *t = thread_current();
    struct file_elem * f;
    struct list_elem * e;
    for (e = list_begin (&t->file_elems); !list_empty (&t->file_elems) && e != list_end (&t->file_elems); e = list_next (e)) {
        f = list_entry(e, struct file_elem, elem);
        if(f->fd == fd){
        	lock_acquire(&file_lock);
         	int offset = file_tell(f->file);
          	lock_release(&file_lock);
         	return offset;
        }
    }
	return 0;
}
void close_routine (int fd) {
	if (fd == STDIN_FILENO || fd == STDOUT_FILENO) {
		exit_routine(-1);
	}

    struct thread *t = thread_current();
    struct file_elem * f;
    struct list_elem * e;
    for (e = list_begin (&t->file_elems); !list_empty (&t->file_elems) && e != list_end (&t->file_elems); e = list_next (e)) {
        f = list_entry(e, struct file_elem, elem);
        if(f->fd == fd){
        	lock_acquire(&file_lock);
         	file_close(f->file);
         	lock_release(&file_lock);
         	list_remove(&(f->elem));
         	free(f);
         	return;
        }
    }
}

