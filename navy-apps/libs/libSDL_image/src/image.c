#define SDL_malloc malloc
#define SDL_free free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface *IMG_Load_RW(SDL_RWops *src, int freesrc)
{
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface *IMG_Load(const char *filename)
{
  // 打开文件
  FILE *file = fopen(filename, "rb");
  if (!file)
  {
    perror("Failed to open file");
    return NULL;
  }

  // 获取文件大小
  if (fseek(file, 0, SEEK_END) != 0)
  {
    perror("Failed to seek to end of file");
    fclose(file);
    return NULL;
  }

  size_t size = ftell(file);
  if (size < 0)
  {
    perror("Failed to tell file size");
    fclose(file);
    return NULL;
  }

  if (fseek(file, 0, SEEK_SET) != 0)
  {
    perror("Failed to seek to beginning of file");
    fclose(file);
    return NULL;
  }

  // 申请一段内存来存储文件内容
  unsigned char *buf = (unsigned char *)malloc(size * sizeof(unsigned char));
  if (!buf)
  {
    perror("Failed to allocate memory");
    fclose(file);
    return NULL;
  }

  // 将文件读取到内存中
  size_t read_size = fread(buf, 1, size, file);
  if (read_size != size)
  {
    perror("Failed to read file content");
    free(buf);
    fclose(file);
    return NULL;
  }

  // 关闭文件
  fclose(file);

  // 调用 STBIMG_LoadFromMemory 进行图像解码
  SDL_Surface *surface = STBIMG_LoadFromMemory(buf, size);
  if (!surface)
  {
    printf("Failed to load image from memory\n");
    free(buf);
    return NULL;
  }

  // 释放文件缓冲区
  free(buf);

  return surface;
}

int IMG_isPNG(SDL_RWops *src)
{
  return 0;
}

SDL_Surface *IMG_LoadJPG_RW(SDL_RWops *src)
{
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError()
{
  return "Navy does not support IMG_GetError()";
}
