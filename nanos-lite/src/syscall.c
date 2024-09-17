#include <common.h>

#include <fs.h>
#include "syscall.h"

#include <sys/time.h>

#define MAX_FILES 128

static char *fd_to_filename[MAX_FILES] = {NULL}; // 定义文件描述符和文件名的映射

static bool init_flag = true;
void init_fd_to_filename();

int sys_exit(int status);
void sys_yield();
int sys_open(const char *pathname, int flags, int mode);
size_t sys_read(int fd, void *buf, size_t len);
size_t sys_write(int fd, const void *buf, size_t len);
int sys_close(int fd);
size_t sys_lseek(int fd, size_t offset, int whence);
int sys_brk(uintptr_t new_brk);
int sys_execve(const char *fname, char *const argv[], char *const envp[]);
int sys_gettimeofday(struct timeval *tv, struct timezone *tz);

void do_syscall(Context *c)
{
  if (init_flag)
  {
    init_fd_to_filename();
    init_flag = false;
  }
  uintptr_t a[4];
  a[0] = c->GPR1; // a7
  a[1] = c->GPR2; // a0
  a[2] = c->GPR3; // a1
  a[3] = c->GPR4; // a2

  switch (a[0])
  {
  case SYS_exit:
#ifdef HAS_STRACE
    printf("sys_exit(%d)\n", a[1]);
#endif
    c->GPRx = sys_exit(a[0]);
    break;

  case SYS_yield:
#ifdef HAS_STRACE
    printf("sys_yield()\n");
#endif
    sys_yield();
    c->GPRx = 0;
    break;

  case SYS_open:
#ifdef HAS_STRACE
    printf("sys_open(%s, %d, %d)\n", (char *)a[1], a[2], a[3]);
#endif
    c->GPRx = sys_open((char *)a[1], a[2], a[3]);
    break;

  case SYS_read:
#ifdef HAS_STRACE
    printf("sys_read(%s, %p, %d)\n", fd_to_filename[a[1]], (void *)a[2], a[3]);
#endif
    c->GPRx = sys_read(a[1], (void *)a[2], a[3]);
    break;

  case SYS_write:
#ifdef HAS_STRACE
    printf("sys_write(%s, \"", fd_to_filename[a[1]]); // 如果 stdout 没有初始化，即 fd_to_filename[1] = NULL，则 %s 会有问题（NULL 的 address 为0）  address (0x00000000) is out of bound at pc = 0x800013e0

    for (size_t i = 0; i < a[3]; i++)
    {
      char ch = ((char *)a[2])[i];
      if (ch == '\n')
      {
        printf("\\n");
      }
      else
      {
        putch(ch);
      }
    }
    printf("\", %d)\n", a[3]);
#endif
    c->GPRx = sys_write(a[1], (void *)a[2], a[3]); // 将 a[2] 转换为 void*
    break;

  case SYS_close:
#ifdef HAS_STRACE
    printf("sys_close(%s)\n", fd_to_filename[a[1]]);
#endif
    c->GPRx = sys_close(a[1]);
    break;

  case SYS_lseek:
#ifdef HAS_STRACE
    printf("sys_lseek(%s, %d, %d)\n", fd_to_filename[a[1]], a[2], a[3]);
#endif
    c->GPRx = sys_lseek(a[1], a[2], a[3]);
    break;

  case SYS_brk:
#ifdef HAS_STRACE
    printf("sys_brk(%p)\n", (void *)a[1]);
#endif
    c->GPRx = sys_brk(a[1]);
    break;

  case SYS_execve:
#ifdef HAS_STRACE
    printf("sys_execve(%s)\n", (const char *)a[1]);
#endif
    c->GPRx = sys_execve((const char *)a[1], (char **)a[2], (char **)a[3]);
    break;

  case SYS_gettimeofday:
#ifdef HAS_STRACE
    printf("sys_gettimeofday(%p, %p)\n", (void *)a[1], (void *)a[2]);
#endif
    c->GPRx = sys_gettimeofday((struct timeval *)a[1], (struct timezone *)a[2]);
    break;

  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}

void init_fd_to_filename()
{
  fd_to_filename[0] = strcpy(malloc(strlen("stdin") + 1), "stdin"); // malloc(strlen(string) + 1) '\0'
  fd_to_filename[1] = strcpy(malloc(strlen("stdout") + 1), "stdout");
  fd_to_filename[2] = strcpy(malloc(strlen("stderr") + 1), "stderr");
}

int sys_exit(int status)
{
  halt(status);
  // 重新加载 nterm
  // return sys_execve("/bin/menu", NULL, NULL);
}

void sys_yield()
{
  yield();
}

int sys_open(const char *pathname, int flags, int mode)
{
  int fd = fs_open(pathname, flags, mode);
  if (fd >= 0 && fd < MAX_FILES)
  {
    fd_to_filename[fd] = strcpy(malloc(strlen(pathname) + 1), pathname); // 先分配空间，再 strcpy
  }
  return fd;
}

size_t sys_read(int fd, void *buf, size_t len)
{
  return fs_read(fd, buf, len);
}

size_t sys_write(int fd, const void *buf, size_t len)
{
  return fs_write(fd, buf, len);
}

int sys_close(int fd)
{
  if (fd >= 0 && fd < MAX_FILES)
  {
    free(fd_to_filename[fd]); // 释放文件名
    fd_to_filename[fd] = NULL;
  }
  return fs_close(fd);
}

size_t sys_lseek(int fd, size_t offset, int whence)
{
  return fs_lseek(fd, offset, whence);
}

int sys_brk(uintptr_t new_brk)
{
  return 0;
}

int sys_execve(const char *fname, char *const argv[], char *const envp[])
{
  naive_uload(NULL, fname);
  return 0;
}

int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
  uint64_t us = io_read(AM_TIMER_UPTIME).us; // 在AM实现了这个关于获取时间的ioe 在 AM 眼中，nanos-lite 和 tests/rtc.c 是一样的
  if (tv)
  {
    // us 当前时间的微秒数
    tv->tv_sec = us / (1000 * 1000);
    tv->tv_usec = us % (1000 * 1000);
  }

  if (tz)
  {
    // 对于这个简单实现，tz可以忽略或设置为0
    tz->tz_minuteswest = 0;
    tz->tz_dsttime = 0;
  }

  return 0;
}