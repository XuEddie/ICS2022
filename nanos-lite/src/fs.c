#include <fs.h>

#include "ramdisk.h"

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

typedef struct
{
  char *name;         // 由于sfs没有目录, 我们把目录分隔符/也认为是文件名的一部分, 例如/bin/hello是一个完整的文件名
  size_t size;        // 文件大小
  size_t disk_offset; // 文件在ramdisk中的偏移
  size_t open_offset; // 每次对文件读写了多少个字节, 偏移量就前进多少
  ReadFn read;        // 读函数指针
  WriteFn write;      // 写函数指针
} Finfo;

enum
{
  FD_STDIN,
  FD_STDOUT,
  FD_STDERR,
  FD_FB
};

size_t invalid_read(void *buf, size_t offset, size_t len)
{
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len)
{
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, serial_write},
    [FD_FB] = {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
    {"/dev/events", 0, 0, 0, events_read, invalid_write},
    {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
#include "files.h" // 普通文件的 read = write = NULL
};

#define NR_FILES (sizeof(file_table) / sizeof(Finfo))

void init_fs()
{
  // TODO: initialize the size of /dev/fb
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;
  file_table[FD_FB].size = w * h * sizeof(uint32_t);
}

int fs_open(const char *pathname, int flags, int mode) // 由于sfs的文件数目是固定的, 我们可以简单地把文件记录表的下标作为相应文件的文件描述符返回给用户程序
{
  for (int i = 0; i < NR_FILES; i++)
  {
    if (strcmp(pathname, file_table[i].name) == 0)
    {
      return i;
    }
  }
  assert(0); // 文件不存在时终止程序
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len)
{
  assert(fd >= 0 && fd < NR_FILES);
  Finfo *f = &file_table[fd];
  size_t real_len;
  if (f->read)
  {
    real_len = f->read(buf, f->open_offset, len);
    f->open_offset += real_len;
  }
  else
  {
    if (f->open_offset + len > f->size)
    {
      len = f->size - f->open_offset; // 避免越界读取
    }
    real_len = ramdisk_read(buf, f->disk_offset + f->open_offset, len);
    f->open_offset += real_len;
  }
  return real_len;
}

size_t fs_write(int fd, const void *buf, size_t len)
{
  assert(fd >= 0 && fd < NR_FILES);
  Finfo *f = &file_table[fd];
  size_t real_len;
  if (f->write)
  {
    real_len = f->write(buf, f->open_offset, len); // 对于串口输出，stdout 的 size 是0，如果对 len 做了更新，则 len = size - offset = 0，导致没有输出
    f->open_offset += real_len;
  }
  else // 默认是普通文件
  {
    assert(f->open_offset + len <= f->size);
    real_len = ramdisk_write(buf, f->disk_offset + f->open_offset, len);
    f->open_offset += real_len;
  }
  return real_len;
}

size_t fs_lseek(int fd, size_t offset, int whence)
{
  assert(fd >= 0 && fd < NR_FILES);
  Finfo *f = &file_table[fd];
  size_t file_size = f->size;
  size_t open_offset = f->open_offset;
  size_t new_offset;

  switch (whence)
  {
  case SEEK_SET:
    new_offset = offset;
    break;
  case SEEK_CUR:
    new_offset = open_offset + offset;
    break;
  case SEEK_END:
    new_offset = file_size + offset;
    break;
  default:
    panic("Invalid whence value!");
  }

  assert(new_offset >= 0 && new_offset <= file_size); // 防止越界
  f->open_offset = new_offset;
  return new_offset;
}

int fs_close(int fd)
{
  assert(fd >= 0 && fd < NR_FILES);
  return 0;
}