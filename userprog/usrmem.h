#ifndef __LIB_USER_USRMEM_H
#define __LIB_USER_USRMEM_H

#include <stdint.h>
#include <stdbool.h>

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
int get_user (const uint8_t *uaddr);

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
bool put_user (uint8_t *udst, uint8_t byte);

/* Reads a byte sequence of length 'length' at user virtual address src and stores it at dst.
   Returns true successful, false if an error occurs.
   occurred.
*/
bool read_user(const void * src, const int length, const void * dst);


/* Writes a byte sequence of length 'length' at user virtual address dst from src.
   Returns true successful, false if an error occurs.
   occurred.
*/
bool write_user(const void * dst, const int length, const void * src);
/* Checks if a valid string is readable from user memory address.
*/
bool check_user_string(const void * src);

#endif /* lib/user/usrmem.h */