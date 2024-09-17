#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4) // sync 在 vgactl_port_base 的高32位

void __am_gpu_init()
{
  // int i;
  // int w = io_read(AM_GPU_CONFIG).width;
  // int h = io_read(AM_GPU_CONFIG).height;
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i++)
  //   fb[i] = i;
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg)
{
  uint32_t screen_info = inl(VGACTL_ADDR);
  uint32_t width = (screen_info >> 16) & 0xffff; // 高16位是屏幕宽度
  uint32_t height = screen_info & 0xffff;        // 低16位是屏幕高度
  *cfg = (AM_GPU_CONFIG_T){
      .present = true, .has_accel = false, .width = width, .height = height, .vmemsz = 0};
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl)
{
  // 获取屏幕的宽度
  uint32_t screen_width = (inl(VGACTL_ADDR) >> 16) & 0xffff;

  // 获取绘制起点的基地址
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *pi = (uint32_t *)(uintptr_t)ctl->pixels;

  // 遍历每一行和每一列
  for (int i = 0; i < ctl->h; i++) // 共有 h 行，每次处理1行
  {
    for (int j = 0; j < ctl->w; j++)
    {
      fb[(ctl->y + i) * screen_width + (ctl->x + j)] = pi[i * ctl->w + j]; // 使用screen_width作为步长
    }
  }

  if (ctl->sync)
  {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status)
{
  status->ready = true;
}
