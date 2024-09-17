#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
  assert(s != NULL);
  size_t length = 0;
  while (s[length] != '\0')
  {
    length++;
  }
  return length;
}

char *strcpy(char *dst, const char *src)
{
  assert(dst != NULL && src != NULL);
  char *original_dst = dst; // 这里可以修改 dst 的指向，因为c语言是值传递，即这里传入的 dst 只是 dst 的一个副本，我们只修改了指针指向的内容，而没有修改原本的 dst，即原本的 dst 仍指向字符串首字符
  while (*src != '\0')
  {
    *original_dst = *src;
    original_dst++;
    src++;
  }
  *original_dst = '\0'; // 重要
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
  assert(dst != NULL && src != NULL);
  size_t i;

  // 逐字符复制
  for (i = 0; i < n && src[i] != '\0'; i++) // 不额外添加 '\0'
  {
    dst[i] = src[i];
  }

  // 如果 src 长度小于 n，填充剩余部分为 '\0'
  for (; i < n; i++)
  {
    dst[i] = '\0';
  }

  // 返回目标字符串的指针
  return dst;
}

char *strcat(char *dst, const char *src)
{
  assert(dst != NULL && src != NULL);
  char *original_dst = dst;
  while (*original_dst != '\0')
  {
    original_dst++;
  }
  while (*src != '\0')
  {
    *original_dst = *src;
    original_dst++;
    src++;
  }
  *original_dst = '\0'; // 重要
  return dst;
}

int strcmp(const char *s1, const char *s2)
{
  if (s1 == NULL && s2 == NULL)
  {
    return 0;
  }
  else if (s1 == NULL && s2 != NULL)
  {
    return -1;
  }
  else if (s1 != NULL && s2 == NULL)
  {
    return 1;
  }
  else
  {
    while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2)
    {
      s1++;
      s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
  }
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  if (s1 == NULL && s2 == NULL)
  {
    return 0;
  }
  else if (s1 == NULL && s2 != NULL)
  {
    return -1;
  }
  else if (s1 != NULL && s2 == NULL)
  {
    return 1;
  }
  else
  {
    // 逐字符比较
    for (size_t i = 0; i < n; i++)
    {
      // 如果到达字符串末尾
      if (s1[i] == '\0' || s2[i] == '\0')
      {
        return (unsigned char)s1[i] - (unsigned char)s2[i];
      }

      // 比较字符
      if (s1[i] != s2[i])
      {
        return (unsigned char)s1[i] - (unsigned char)s2[i];
      }
    }

    // 如果前 n 个字符相等，返回 0
    return 0;
  }
}

void *memset(void *s, int c, size_t n)
{
  assert(s != NULL);
  unsigned char *p = (unsigned char *)s; // 内存是字节数组 uint8_t
  unsigned char value = (unsigned char)c;
  for (size_t i = 0; i < n; i++)
  {
    p[i] = value;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n)
{
  assert(dst != NULL && src != NULL);
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;

  // 判断内存是否重叠
  if (d < s || d >= s + n) // 指针就是地址，根据首地址判断内存区域是否重叠
  {
    // 没有重叠，从前往后复制
    for (size_t i = 0; i < n; i++)
    {
      d[i] = s[i];
    }
  }
  else
  {
    // 有重叠，从后往前复制
    for (size_t i = n; i > 0; i--)
    {
      d[i - 1] = s[i - 1];
    }
  }

  // 返回目标内存块的指针
  return dst;
}

void *memcpy(void *out, const void *in, size_t n)
{
  assert(out != NULL && in != NULL);
  unsigned char *dst = (unsigned char *)out;
  const unsigned char *src = (const unsigned char *)in;
  for (size_t i = 0; i < n; i++)
  {
    dst[i] = src[i];
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  if (s1 == NULL && s2 == NULL)
  {
    return 0;
  }
  else if (s1 == NULL && s2 != NULL)
  {
    return -1;
  }
  else if (s1 != NULL && s2 == NULL)
  {
    return 1;
  }
  else
  {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    for (size_t i = 0; i < n; i++) // 调用者应该确保在调用 memcmp 时提供的 n 不超过两个指针所指向的内存区域的大小。如果提供的 n 超过了实际可用的内存大小，函数的行为是未定义的
    {
      if (p1[i] != p2[i])
      {
        return (int)(p1[i] - p2[i]);
      }
    }
    return 0; // 如果前 n 个相等，返回 0
  }
}

#endif
