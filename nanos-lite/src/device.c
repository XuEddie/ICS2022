#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [AM_KEY_NONE] = "NONE",
    AM_KEYS(NAME)};

// 串口
size_t serial_write(const void *buf, size_t offset, size_t len)
{
  const char *p = (const char *)buf;
  for (size_t i = 0; i < len; i++)
  {
    putch(p[i]); // 向串口输出字符
  }
  return len;
}

// 键盘 上述事件抽象成一个特殊文件/dev/events
// event-test 记得先按c启动程序
size_t events_read(void *buf, size_t offset, size_t len)
{
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE)
  {
    return 0;
  }
  const char *type = ev.keydown ? "kd" : "ku";
  const char *key = keyname[ev.keycode];
  // printf("Got  (kbd): %s (%d) %s\n", keyname[ev.keycode], ev.keycode, ev.keydown ? "DOWN" : "UP");
  int n = snprintf((char *)buf, len, "%s %s\n", type, key);
  return (n < len) ? n : len;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len)
{                                        // 400*300
  int w = io_read(AM_GPU_CONFIG).width;  // 400
  int h = io_read(AM_GPU_CONFIG).height; // 300
  int n = snprintf((char *)buf, len, "WIDTH: %d\nHEIGHT: %d\n", w, h);
  // printf("%s", (char *)buf);
  if (n >= len)
  {
    assert(0);
  }

  return n + 1;
}

// 绘制`w*h`的矩形图像 规定每次绘制一行，即 len = w * sizeof(uint32_t)
size_t fb_write(const void *buf, size_t offset, size_t len)
{
  int screen_w = io_read(AM_GPU_CONFIG).width;
  int start_pixel = offset / sizeof(uint32_t); // 每个像素占4字节，因此 offset / 4 计算出像素数  x,y 是 FB（屏幕）的真实坐标
  int x = start_pixel % screen_w;
  int y = start_pixel / screen_w;
  int pixels_to_write = len / sizeof(uint32_t);
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, pixels_to_write, 1, true); // 每次绘制一行并同步整个屏幕
  return len;
}

void init_device()
{
  Log("Initializing devices...");
  ioe_init();
}
