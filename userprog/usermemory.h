
#ifndef PINTOS_USERMEMORY_H
#define PINTOS_USERMEMORY_H

#include <stdint.h>
#include <stdbool.h>

int get_user (const uint8_t *uaddr);
bool put_user (uint8_t *udst, uint8_t byte);
bool read_user(const void * src, const int length, const void * dst);
bool check_user_string(const void * src);

#endif //PINTOS_USERMEMORY_H
