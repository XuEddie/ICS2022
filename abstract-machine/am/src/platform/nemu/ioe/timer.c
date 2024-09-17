#include <am.h>
#include <nemu.h>

void __am_timer_init()
{
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) // 从0开始计数
{
  // RTC_ADDR 是 RTC 存储的地址（内存映射）访问即可获得时间
  // 0xa0000048处长度为8字节的MMIO空间, 它们都会映射到两个32位的RTC寄存器. CPU可以访问这两个寄存器来获得用64位表示的当前时间.
  uint32_t high_part = inl(RTC_ADDR + 4);              // 高32位 先读高32位
  uint32_t low_part = inl(RTC_ADDR);                   // 低32位
  uptime->us = ((uint64_t)high_part << 32) | low_part; // 拼接
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc)
{
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour = 0;
  rtc->day = 0;
  rtc->month = 0;
  rtc->year = 1900;
}
