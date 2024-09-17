#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <limits.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int vsprintf(char *out, const char *fmt, va_list ap)
{
  char *ptr = out; // 指向输出缓冲区

  while (*fmt)
  {
    if (*fmt == '%')
    {
      fmt++; // 移动到格式指示符的下一个字符

      // 处理宽度说明符
      int width = 0;
      if (*fmt >= '0' && *fmt <= '9')
      {
        width = *fmt - '0';
        fmt++;
        if (*fmt >= '0' && *fmt <= '9')
        {
          width = width * 10 + (*fmt - '0');
          fmt++;
        }
      }

      switch (*fmt)
      {

      case 'c':
      {
        char c = va_arg(ap, int);
        *ptr++ = c;
        break;
      }

      case 'd':
      case 'i':
      {
        int num = va_arg(ap, int);

        // 处理负数
        if (num == INT_MIN)
        {
          *ptr++ = '-';
          *ptr++ = '2';
          num = -(num % 1000000000); // 去掉前导 '2'，保留后面的 147483648
        }

        if (num < 0)
        {
          *ptr++ = '-';
          num = -num;
        }

        // 将数字转换为字符串
        char buffer[12];
        char *buf_ptr = buffer + sizeof(buffer) - 1;
        *buf_ptr = '\0';
        do
        {
          *--buf_ptr = (num % 10) + '0';
          num /= 10;
        } while (num);

        // 计算要填充的前导零的个数
        int len = buffer + sizeof(buffer) - 1 - buf_ptr;
        while (width > len)
        {
          *ptr++ = '0';
          width--;
        }

        // 复制转换后的数字字符串
        while (*buf_ptr)
        {
          *ptr++ = *buf_ptr++;
        }
        break;
      }

      case 'u':
      {
        unsigned int num = va_arg(ap, unsigned int);

        // 将数字转换为字符串
        char buffer[11]; // 10 digits + null terminator
        char *buf_ptr = buffer + sizeof(buffer) - 1;
        *buf_ptr = '\0';
        do
        {
          *--buf_ptr = (num % 10) + '0';
          num /= 10;
        } while (num);

        // 计算要填充的前导零的个数
        int len = buffer + sizeof(buffer) - 1 - buf_ptr;
        while (width > len)
        {
          *ptr++ = '0';
          width--;
        }

        // 复制转换后的数字字符串
        while (*buf_ptr)
        {
          *ptr++ = *buf_ptr++;
        }
        break;
      }

      case 'x':
      { // 处理十六进制输出
        unsigned int num = va_arg(ap, unsigned int);
        char buffer[9]; // 最大为8位数加上一个空终止符
        char *buf_ptr = buffer + sizeof(buffer) - 1;
        *buf_ptr = '\0';
        do
        {
          unsigned int digit = num % 16;
          *--buf_ptr = (digit < 10) ? digit + '0' : digit - 10 + 'a';
          num /= 16;
        } while (num);

        // 计算要填充的前导零的个数
        int len = buffer + sizeof(buffer) - 1 - buf_ptr;
        while (width > len)
        {
          *ptr++ = '0';
          width--;
        }

        // 复制转换后的数字字符串
        while (*buf_ptr)
        {
          *ptr++ = *buf_ptr++;
        }
        break;
      }

      case 'p':
      { // 处理指针地址输出
        uintptr_t addr = (uintptr_t)va_arg(ap, void *);
        *ptr++ = '0';
        *ptr++ = 'x';

        char buffer[17]; // 地址长度为16位，加上一个空终止符
        char *buf_ptr = buffer + sizeof(buffer) - 1;
        *buf_ptr = '\0';
        do
        {
          unsigned int digit = addr % 16;
          *--buf_ptr = (digit < 10) ? digit + '0' : digit - 10 + 'a';
          addr /= 16;
        } while (addr);

        // 计算要填充的前导零的个数
        int len = buffer + sizeof(buffer) - 1 - buf_ptr;
        while (width > len)
        {
          *ptr++ = '0';
          width--;
        }

        // 复制转换后的地址字符串
        while (*buf_ptr)
        {
          *ptr++ = *buf_ptr++;
        }
        break;
      }

      case 's':
      {
        char *str = va_arg(ap, char *);
        while (*str)
        {
          *ptr++ = *str++;
        }
        break;
      }
      default:
        *ptr++ = '%'; // 如果格式不认识，打印 '%' 字符
        *ptr++ = *fmt;
        break;
      }
    }
    else
    {
      *ptr++ = *fmt; // 直接复制非格式字符
    }
    fmt++;
  }

  *ptr = '\0';      // 添加字符串结束符
  return ptr - out; // 返回写入的字符总数
}

int sprintf(char *out, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int len = vsprintf(out, fmt, args); // 调用vsprintf来完成实际的格式化工作
  va_end(args);
  return len;
}

int printf(const char *fmt, ...)
{
  char buffer[2048]; // 临时缓冲区，用于存储格式化后的字符串
  va_list args;
  va_start(args, fmt);
  int len = vsprintf(buffer, fmt, args); // 调用vsprintf来完成格式化工作
  va_end(args);
  putstr(buffer); // putch 使用串口 MMIO需要打开设备device CONFIG_DEVICE
  return len;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap)
{
  char *ptr = out;
  char *end = out + n - 1; // 留出最后一个字符给 '\0'

  while (*fmt && ptr < end)
  {
    if (*fmt == '%')
    {
      fmt++;
      int width = 0;
      if (*fmt >= '0' && *fmt <= '9')
      {
        width = *fmt - '0';
        fmt++;
        if (*fmt >= '0' && *fmt <= '9')
        {
          width = width * 10 + (*fmt - '0');
          fmt++;
        }
      }

      switch (*fmt)
      {
      case 'c':
      {
        char c = va_arg(ap, int);
        if (ptr < end)
          *ptr++ = c;
        break;
      }
      case 'd':
      case 'i':
      {
        int num = va_arg(ap, int);
        if (num == INT_MIN)
        {
          if (ptr < end)
            *ptr++ = '-';
          if (ptr < end)
            *ptr++ = '2';
          num = -(num % 1000000000);
        }
        if (num < 0)
        {
          if (ptr < end)
            *ptr++ = '-';
          num = -num;
        }

        char buffer[12];
        char *buf_ptr = buffer + sizeof(buffer) - 1;
        *buf_ptr = '\0';
        do
        {
          *--buf_ptr = (num % 10) + '0';
          num /= 10;
        } while (num);

        int len = buffer + sizeof(buffer) - 1 - buf_ptr;
        while (width > len)
        {
          if (ptr < end)
            *ptr++ = '0';
          width--;
        }

        while (*buf_ptr && ptr < end)
        {
          *ptr++ = *buf_ptr++;
        }
        break;
      }
      case 'u':
      {
        unsigned int num = va_arg(ap, unsigned int);
        char buffer[11];
        char *buf_ptr = buffer + sizeof(buffer) - 1;
        *buf_ptr = '\0';
        do
        {
          *--buf_ptr = (num % 10) + '0';
          num /= 10;
        } while (num);

        int len = buffer + sizeof(buffer) - 1 - buf_ptr;
        while (width > len)
        {
          if (ptr < end)
            *ptr++ = '0';
          width--;
        }

        while (*buf_ptr && ptr < end)
        {
          *ptr++ = *buf_ptr++;
        }
        break;
      }
      case 'x':
      {
        unsigned int num = va_arg(ap, unsigned int);
        char buffer[9];
        char *buf_ptr = buffer + sizeof(buffer) - 1;
        *buf_ptr = '\0';
        do
        {
          unsigned int digit = num % 16;
          *--buf_ptr = (digit < 10) ? digit + '0' : digit - 10 + 'a';
          num /= 16;
        } while (num);

        int len = buffer + sizeof(buffer) - 1 - buf_ptr;
        while (width > len)
        {
          if (ptr < end)
            *ptr++ = '0';
          width--;
        }

        while (*buf_ptr && ptr < end)
        {
          *ptr++ = *buf_ptr++;
        }
        break;
      }
      case 'p':
      {
        uintptr_t addr = (uintptr_t)va_arg(ap, void *);
        if (ptr < end)
          *ptr++ = '0';
        if (ptr < end)
          *ptr++ = 'x';

        char buffer[17];
        char *buf_ptr = buffer + sizeof(buffer) - 1;
        *buf_ptr = '\0';
        do
        {
          unsigned int digit = addr % 16;
          *--buf_ptr = (digit < 10) ? digit + '0' : digit - 10 + 'a';
          addr /= 16;
        } while (addr);

        int len = buffer + sizeof(buffer) - 1 - buf_ptr;
        while (width > len)
        {
          if (ptr < end)
            *ptr++ = '0';
          width--;
        }

        while (*buf_ptr && ptr < end)
        {
          *ptr++ = *buf_ptr++;
        }
        break;
      }
      case 's':
      {
        char *str = va_arg(ap, char *);
        while (*str && ptr < end)
        {
          *ptr++ = *str++;
        }
        break;
      }
      default:
        if (ptr < end)
          *ptr++ = '%';
        if (ptr < end)
          *ptr++ = *fmt;
        break;
      }
    }
    else
    {
      if (ptr < end)
        *ptr++ = *fmt;
    }
    fmt++;
  }

  *ptr = '\0';      // 确保字符串以 '\0' 结束
  return ptr - out; // 返回写入的字符总数
}

int snprintf(char *out, size_t n, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(out, n, fmt, args);
  va_end(args);
  return len;
}

#endif
