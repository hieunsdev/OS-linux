#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* @@ Added and modified by student */
// Function to manipulate in scope of user space
static int32_t get_user(const uint8_t *uaddr);
static bool put_user(uint8_t *udst, uint8_t byte);
static int memread_user(void *src, void *des, size_t bytes);

// System call to exit terminated process
void syscall_init(void);
void sys_exit(int);
static void syscall_handler(struct intr_frame *f);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void sys_exit(int status)
{
  printf("\n[Message by student] %s: exit(%d)\n\n", thread_current()->name, status);
  thread_exit();
}

static void
syscall_handler(struct intr_frame *f)
{
  int syscall_number;
  ASSERT(sizeof(syscall_number) == 4);

  // The system call number is in the 32-bit word at the caller's stack pointer.
  memread_user(f->esp, &syscall_number, sizeof(syscall_number));

  // SYS_*** constants are defined in syscall-nr.h
  switch (syscall_number)
  {
  case SYS_EXIT: // 1
  {
    int exitcode;
    memread_user(f->esp + 4, &exitcode, sizeof(exitcode));

    sys_exit(exitcode);
    NOT_REACHED();
    break;
  }
  /* unhandled case */
  default:
    printf("[ERROR] system call EXIT is unimplemented!\n");
    sys_exit(-1);
    break;
  }
}

static int
memread_user(void *src, void *dst, size_t bytes)
{
  int32_t value;
  size_t i;
  for (i = 0; i < bytes; i++)
  {
    value = get_user(src + i);
    if (value == -1) // segfault or invalid memory access
    {
      sys_exit(-1);
      NOT_REACHED(); // PANIC kernel
    }

    *(char *)(dst + i) = value & 0xff;
  }
  return (int)bytes;
}

/**
 * @@ Follow the reference, added by student
 */

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user(const uint8_t *uaddr)
{
  int result;
  asm("movl $1f, %0; movzbl %1, %0; 1:"
      : "=&a"(result)
      : "m"(*uaddr));
  return result;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user(uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm("movl $1f, %0; movb %b2, %1; 1:"
      : "=&a"(error_code), "=m"(*udst)
      : "q"(byte));
  return error_code != -1;
}

/**
 *  End
 */