#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/time.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0; // 屏幕大小

// 以毫秒为单位返回系统时间
uint32_t NDL_GetTicks()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000); // 将秒和微秒转换为毫秒
}

// 读出一条事件信息, 将其写入`buf`中, 最长写入`len`字节
// 若读出了有效的事件, 函数返回1, 否则返回0
int NDL_PollEvent(char *buf, int len)
{
  int fd = open("/dev/events", O_RDONLY); // 从/dev/events 中读出事件并写入到buf中 #define	O_RDONLY	0
  assert(fd > 0);

  int ret = read(fd, buf, len); // 由于 /dev/events 是一个字符设备文件，它的内容是动态生成的键盘事件，而不是静态存储在磁盘上的数据，因此更适合使用底层的 open() 函数。这种方式可以避免 fopen() 带来的额外缓冲，从而确保读取到的事件数据是最新的，而不是读取到缓冲区中的旧数据
  close(fd);
  return ret > 0 ? 1 : 0;
}

static int canvas_w, canvas_h;         // 画布大小
static int canvas_x = 0, canvas_y = 0; // 画布左上角坐标 画布相对于系统屏幕的位置
void NDL_OpenCanvas(int *w, int *h)
{
  if (getenv("NWM_APP"))
  {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1)
    {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0)
        continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0)
        break;
    }
    close(fbctl);
  }

  int fd = open("/proc/dispinfo", O_RDONLY);
  assert(fd > 0);
  char buf[64];
  read(fd, buf, sizeof(buf));
  close(fd);
  sscanf(buf, "WIDTH: %d\nHEIGHT: %d\n", &screen_w, &screen_h);

  if (*w == 0 && *h == 0)
  {
    *w = screen_w;
    *h = screen_h;
  }
  else
  {
    if (*w > screen_w)
      *w = screen_w;
    if (*h > screen_h)
      *h = screen_h;
  }
  canvas_w = *w;
  canvas_h = *h;

  // 根据屏幕大小和画布大小, 让NDL将图像绘制到屏幕的中央
  canvas_x = (screen_w - canvas_w) / 2;
  canvas_y = (screen_h - canvas_h) / 2;
}

// 向画布`(x, y)`坐标处绘制`w*h`的矩形图像, 并将该绘制区域同步到屏幕上
// 图像像素按行优先方式存储在`pixels`中, 每个像素用32位整数以`00RRGGBB`的方式描述颜色 这里 pixels 就是将绘图区域的矩形展开（连续的数组）
// 程序绘图时的坐标都是针对画布来设定的, 这样程序就无需关心系统屏幕的大小
// 画布的每个像素位置 (x, y) 对应到系统屏幕中的位置 (x_canvas + x, y_canvas + y)，其中 (x_canvas, y_canvas) 是画布在屏幕上的起始位置（画布左上角）
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h)
{
  assert(x + w <= canvas_w);
  assert(y + h <= canvas_h);

  int FD_FB = open("/dev/fb", O_RDWR);
  // 显然 w*h 的矩形不能一次输入（传入 len = w*h 则破坏了矩形的性质）
  // 因此每次绘制一行(len = w * sizeof(uint32_t))
  for (int i = 0; i < h; i++)
  {
    lseek(FD_FB, ((canvas_y + y + i) * screen_w + (canvas_x + x)) * sizeof(uint32_t), SEEK_SET);
    write(FD_FB, pixels + i * w, w * sizeof(uint32_t));
  }
}

void NDL_OpenAudio(int freq, int channels, int samples)
{
}

void NDL_CloseAudio()
{
}

int NDL_PlayAudio(void *buf, int len)
{
  return 0;
}

int NDL_QueryAudio()
{
  return 0;
}

int NDL_Init(uint32_t flags)
{
  if (getenv("NWM_APP"))
  {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit()
{
}
