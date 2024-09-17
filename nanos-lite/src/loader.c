#include <proc.h>
#include <elf.h>

#include <fs.h>
#include "loader.h"
#include "ramdisk.h"

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

// 可以在loader中使用文件名来指定加载的程序，而无需关注文件在ramdisk上的位置
static uintptr_t loader(PCB *pcb, const char *filename)
{
  // 1. 打开文件
  int fd = fs_open(filename, 0, 0);
  assert(fd >= 0); // 确保文件成功打开

  // 2. 读取ELF文件头
  Elf_Ehdr elf_header;

  fs_lseek(fd, 0, SEEK_SET); // 确保文件指针在文件头部 多次重启

  fs_read(fd, &elf_header, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)elf_header.e_ident == 0x464C457F); // ELF文件的魔数校验
  // 3. 遍历Program Header Table
  Elf_Phdr phdr;
  for (int i = 0; i < elf_header.e_phnum; i++)
  {
    // 读取每个Program Header
    size_t phdr_offset = elf_header.e_phoff + i * sizeof(Elf_Phdr); // 文件内偏移量
    fs_lseek(fd, phdr_offset, SEEK_SET);                            // 重置偏移量
    fs_read(fd, &phdr, sizeof(Elf_Phdr));

    // 4. 如果是PT_LOAD类型的segment，则加载到内存中
    if (phdr.p_type == PT_LOAD)
    {
      // 加载segment内容到VirtAddr
      fs_lseek(fd, phdr.p_offset, SEEK_SET);
      fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);

      // 对内存中多出的部分清零 (BSS 段)
      if (phdr.p_memsz > phdr.p_filesz)
      {
        memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
      }
    }
  }

  // 5. 关闭文件
  fs_close(fd);

  // 6. 返回入口地址
  return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename)
{
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  printf("filename:%s\n", filename);
  ((void (*)())entry)();
}
