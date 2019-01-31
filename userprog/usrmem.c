#include "userprog/usrmem.h"
#include "threads/vaddr.h"


/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
int get_user (const uint8_t *uaddr) {
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
bool put_user (uint8_t *udst, uint8_t byte) {
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

/* Reads a byte sequence of length 'length' at user virtual address src and stores it at dst.
   Returns true successful, false if an error occurs.
   occurred.
*/
bool read_user(const void * src, const int length, const void * dst) {
  if (src < PHYS_BASE && (src + length) < PHYS_BASE) {
    int i = 0;
    int temp;
    for (i = 0; i < length; i++) {
      temp = get_user((uint8_t *) src + i);
      if (temp == -1) {
        return false;
      }
      *((uint8_t *) dst + i) = (uint8_t)temp;
    }
    return true;
  } else {
    return false;
  }
}


/* Writes a byte sequence of length 'length' at user virtual address dst from src.
   Returns true successful, false if an error occurs.
   occurred.
*/
bool write_user (const void * dst, const int length, const void * src) {
  if (dst < PHYS_BASE && (dst + length) < PHYS_BASE) {
    int i = 0;
    int temp;
    for (i = 0; i < length; i++) {
      temp = put_user((uint8_t *) dst + i, *((uint8_t *) src + i));
      if (temp == -1) {
        return false;
      }
    }
    return true;
  } else {
    return false;
  }
}

bool check_user_string (const void * src) {
  if (src < PHYS_BASE) {
    int i = 0;
    int temp;
    char c  = 'a';
    while (c != '\0') {
      if ((src + i) >= PHYS_BASE) {
        return false;
      }
      temp = get_user((uint8_t *) src + i);
      if (temp == -1) {
        return false;
      }
      c = temp;
      i++;
    }
    return true;
  } else {
    return false;
  }
}