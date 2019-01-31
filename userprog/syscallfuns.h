#ifndef PINTOS_SYSCALLFUNS_H
#define PINTOS_SYSCALLFUNS_H


#include <stdbool.h>
#include "lib/user/syscall.h"
#include <list.h>

struct file_elem {
    int fd;
    struct file* file;
    struct list_elem elem;
};

void init_lock(void);
void halt(void);
void exit (int status);
pid_t exec(const char *file);
int wait(pid_t id);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned length);
int write(int fd, const void *buffer, unsigned length);
void seek (int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);


#endif //PINTOS_SYSCALLFUNS_H
