#include <stdbool.h>
#include "userprog/syscallfuns.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/off_t.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "userprog/process.h"
#include <stdio.h>

static struct lock file_lock;

void init_lock(void){
    lock_init (&file_lock);
}
void halt (void){
    shutdown_power_off();
}

void exit (int status) {
    struct thread *t = thread_current();
    t -> return_status = status;
    thread_exit();
}

pid_t exec(const char *file){
   if(!check_user_string(file)){
       exit(-1);
   }
    lock_acquire(&file_lock);
    pid_t pid = process_execute(file);
    lock_release(&file_lock);
    return pid;
}

int wait(pid_t id){
    return process_wait(id);
}

bool create (const char *file, unsigned initial_size){
    if(!check_user_string(file)){
        exit(-1);
    }
    lock_acquire(&file_lock);
    bool create = filesys_create(file, initial_size);
    lock_release(&file_lock);
    return create;
}
bool remove (const char *file){
    if(!check_user_string(file)){
        exit(-1);
    }
    lock_acquire(&file_lock);
    bool remove = filesys_remove(file);
    lock_release(&file_lock);
    return remove;
}

int open (const char *file) {
    if (!check_user_string(file)) {
		exit(-1);
	}
    struct thread *t = thread_current ();
    struct file_elem *fileElem = malloc(sizeof(struct file_elem));
    lock_acquire(&file_lock);
    struct file *f = filesys_open(file);
    lock_release(&file_lock);
    if (f == NULL) {
        free(fileElem);
        return -1;
    }
    fileElem->file = f;
    fileElem->fd = t->fd++;
    list_push_back(&t->file_elems, &fileElem->elem);
    return fileElem->fd;
}

int filesize(int fd) {
   struct thread *t = thread_current();
   struct file_elem *fileElem;
   struct list_elem *listElement;
   
   for (listElement = list_begin(&t-> file_elems); !list_empty(&t->file_elems) && listElement != list_end(&t->file_elems); listElement = list_next(&t-> file_elems)) {
   	fileElem = list_entry(listElement, struct file_elem, elem);
	if(fileElem->fd == fd){
    	  lock_acquire(&file_lock);
          int length = file_length(fileElem->file);
          lock_release(&file_lock);
          return length;
        }
    }
    return -1;
}


int read(int fd, void *buffer, unsigned length) {
   if (fd == STDOUT_FILENO) {
       return -1;
   }
   uint8_t *temp_buffer = (uint8_t *) buffer;
   if (fd == STDIN_FILENO) {
        lock_acquire(&file_lock);
   	unsigned i = 0;
   	while (i < length) {
    	    temp_buffer[i] = input_getc();
    	    i ++;
   	}
   	lock_release(&file_lock);
   	return length;
   }
   
   struct thread *t = thread_current();
   struct file_elem *fileElem;
   struct list_elem *listElement;
   
   for (listElement = list_begin(&t-> file_elems); !list_empty(&t->file_elems) && listElement != list_end(&t->file_elems); listElement = list_next(&t-> file_elems)) {
   	fileElem = list_entry(listElement, struct file_elem, elem);
	if(fileElem->fd == fd){
    	  lock_acquire(&file_lock);
          int bytes = file_read(fileElem->file, buffer, length);
          lock_release(&file_lock);
	  return bytes;
        }
    }
   return -1;
}


int write(int fd,const void *buffer, unsigned length) {
   if (fd == STDIN_FILENO){
       return -1 ;   
   }

   if (fd == STDOUT_FILENO) {
      lock_acquire(&file_lock);
      putbuf(buffer , length);
      lock_release(&file_lock);
      return length ;
   }

   struct thread *t = thread_current();
   struct file_elem *fileElem;
   struct list_elem *listElement;
   
   for (listElement = list_begin(&t-> file_elems); !list_empty(&t->file_elems) && listElement != list_end(&t->file_elems); listElement = list_next(&t-> file_elems)) {
   	fileElem = list_entry(listElement, struct file_elem, elem);
	if(fileElem->fd == fd){
	   if(fileElem->file != NULL) {
    	      lock_acquire(&file_lock);
              int bytes = file_write(fileElem->file, buffer, length);
              lock_release(&file_lock);
	      return bytes;
           }
        }
    }
   return -1;
}

void seek (int fd, unsigned position) {
   struct thread *t = thread_current();
   struct file_elem *fileElem;
   struct list_elem *listElement;
   
   for (listElement = list_begin(&t-> file_elems); !list_empty(&t->file_elems) && listElement != list_end(&t->file_elems); listElement = list_next(&t-> file_elems)) {
   	fileElem = list_entry(listElement, struct file_elem, elem);
	if(fileElem->fd == fd){
    	  lock_acquire(&file_lock);
          file_seek(fileElem->file, position);
          lock_release(&file_lock);
        }
    }
}

unsigned tell(int fd) {
   struct thread *t = thread_current();
   struct file_elem *fileElem;
   struct list_elem *listElement;
   
   for (listElement = list_begin(&t-> file_elems); !list_empty(&t->file_elems) && listElement != list_end(&t->file_elems); listElement = list_next(&t-> file_elems)) {
   	fileElem = list_entry(listElement, struct file_elem, elem);
	if(fileElem->fd == fd){
    	  lock_acquire(&file_lock);
          int offset = file_tell(fileElem->file);
          lock_release(&file_lock);
          return offset;
        }
    }
    return 0;
}

void close(int fd) {
   if (fd == STDIN_FILENO || fd == STDOUT_FILENO) {
	exit(-1);
   }
   struct thread *t = thread_current();
   struct file_elem *fileElem;
   struct list_elem *listElement;
   
   for (listElement = list_begin(&t-> file_elems); !list_empty(&t->file_elems) && listElement != list_end(&t->file_elems); listElement = list_next(&t-> file_elems)) {
   	fileElem = list_entry(listElement, struct file_elem, elem);
	if(fileElem->fd == fd){
    	  lock_acquire(&file_lock);
          file_close(fileElem->file);
          lock_release(&file_lock);
	  list_remove(&(fileElem->elem));
          free(fileElem);
          return;
        }
    }
}
