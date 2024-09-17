/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

// 寄存器 -> 数组下标
enum
{
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static uint32_t sbuf_pos = 0; // 维护队列下标 音频缓冲区中的当前播放位置（从这里向后继续播放）

void audio_sdl_callback(void *userdata, uint8_t *stream, int len)
{
  SDL_memset(stream, 0, len); // 把SDL提供的缓冲区剩余的部分清零, 以避免把一些垃圾数据当做音频, 从而产生噪音.
  uint32_t used_cnt = audio_base[reg_count];
  len = len > used_cnt ? used_cnt : len;
  uint32_t sbuf_size = audio_base[reg_sbuf_size];

  if ((sbuf_pos + len) > sbuf_size)
  {
    SDL_MixAudio(stream, sbuf + sbuf_pos, sbuf_size - sbuf_pos, SDL_MIX_MAXVOLUME);                                                // 先读取到缓冲区末尾的数据，并将其混合到 stream 中
    SDL_MixAudio(stream + (sbuf_size - sbuf_pos), sbuf + (sbuf_size - sbuf_pos), len - (sbuf_size - sbuf_pos), SDL_MIX_MAXVOLUME); // 然后从缓冲区的开头继续读取剩余的数据并混合到 stream 中
  }
  else
    SDL_MixAudio(stream, sbuf + sbuf_pos, len, SDL_MIX_MAXVOLUME);
  sbuf_pos = (sbuf_pos + len) % sbuf_size;
  audio_base[reg_count] -= len; // 更新reg_count 减少
}

void init_sound()
{
  SDL_AudioSpec s = {};
  s.format = AUDIO_S16SYS; // 假设系统中音频数据的格式总是使用16位有符号数来表示
  s.userdata = NULL;       // 不使用

  s.freq = audio_base[reg_freq];
  s.channels = audio_base[reg_channels];
  s.samples = audio_base[reg_samples];
  s.callback = audio_sdl_callback;

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
  {
    printf("SDL_InitSubSystem error: %s\n", SDL_GetError());
    return;
  }
  if (SDL_OpenAudio(&s, NULL) < 0)
  {
    printf("SDL_OpenAudio error: %s\n", SDL_GetError());
    return;
  }
  SDL_PauseAudio(0); // 开始播放音频
}

static void audio_io_handler(uint32_t offset, int len, bool is_write)
{
  if (audio_base[reg_init])
  {
    init_sound();
    audio_base[reg_init] = 0;
  }
}

void init_audio()
{
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE; // 初始为0 未赋值
  // printf("whem system start, the count is %d\n", audio_base[reg_count]);
}
