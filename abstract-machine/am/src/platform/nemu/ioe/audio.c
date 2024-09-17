#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)

void __am_audio_init()
{
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg)
{
  uint32_t bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
  *cfg = (AM_AUDIO_CONFIG_T){
      .present = true,
      .bufsize = bufsize};
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl)
{
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1); // 给 init寄存器赋值，用于初始化
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat)
{
  stat->count = inl(AUDIO_COUNT_ADDR);
}

static uint32_t sbuf_pos = 0; // 维护队列下标
void __am_audio_play(AM_AUDIO_PLAY_T *ctl)
{
  uint32_t sbuf_size = inl(AUDIO_SBUF_SIZE_ADDR);
  uint32_t len = (ctl->buf).end - (ctl->buf).start;
  while (inl(AUDIO_SBUF_SIZE_ADDR) - inl(AUDIO_COUNT_ADDR) < len)
    ;
  uint8_t *audio_data = (ctl->buf).start;
  uint8_t *ab = (uint8_t *)(uintptr_t)AUDIO_SBUF_ADDR;
  for (uint32_t i = 0; i < len; i++)
  {
    ab[sbuf_pos] = audio_data[i];
    sbuf_pos = (sbuf_pos + 1) % sbuf_size;
  }
  outl(AUDIO_COUNT_ADDR, inl(AUDIO_COUNT_ADDR) + len); // 更新reg_count 增加
}
