#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// 将一张画布中的指定矩形区域复制到另一张画布的指定位置
void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
  assert(dst && src);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);

  if (src->format->BitsPerPixel == 32)
  {
    uint32_t *src_pixels = (uint32_t *)src->pixels;
    uint32_t *dst_pixels = (uint32_t *)dst->pixels;

    int16_t srcrect_x, srcrect_y;
    uint16_t srcrect_w, srcrect_h;
    if (srcrect == NULL)
    {
      srcrect_x = 0;
      srcrect_y = 0;
      srcrect_w = src->w;
      srcrect_h = src->h;
    }
    else
    {
      srcrect_x = srcrect->x;
      srcrect_y = srcrect->y;
      srcrect_w = srcrect->w;
      srcrect_h = srcrect->h;
    }

    int16_t dstrect_x, dstrect_y;
    if (dstrect == NULL)
    {
      dstrect_x = 0;
      dstrect_y = 0;
    }
    else
    {
      dstrect_x = dstrect->x;
      dstrect_y = dstrect->y;
    }

    // 调整矩形的宽度和高度，以防止越界
    if (dstrect_x + srcrect_w > dst->w)
    {
      srcrect_w = dst->w - dstrect_x;
    }
    if (dstrect_y + srcrect_h > dst->h)
    {
      srcrect_h = dst->h - dstrect_y;
    }

    assert(dstrect_x + srcrect_w <= dst->w);
    assert(dstrect_y + srcrect_h <= dst->h);
    assert(srcrect_x + srcrect_w <= src->w);
    assert(srcrect_y + srcrect_h <= src->h);

    for (uint16_t i = 0; i < srcrect_h; i++)
    {
      for (uint16_t j = 0; j < srcrect_w; j++)
      {
        dst_pixels[(dstrect_y + i) * dst->w + dstrect_x + j] = src_pixels[(srcrect_y + i) * src->w + srcrect_x + j];
      }
    }
  }
  else if (src->format->BitsPerPixel == 8)
  {
    uint8_t *src_pixels = (uint8_t *)src->pixels;
    uint8_t *dst_pixels = (uint8_t *)dst->pixels;

    int16_t srcrect_x, srcrect_y;
    uint16_t srcrect_w, srcrect_h;
    if (srcrect == NULL)
    {
      srcrect_x = 0;
      srcrect_y = 0;
      srcrect_w = src->w;
      srcrect_h = src->h;
    }
    else
    {
      srcrect_x = srcrect->x;
      srcrect_y = srcrect->y;
      srcrect_w = srcrect->w;
      srcrect_h = srcrect->h;
    }

    int16_t dstrect_x, dstrect_y;
    // uint16_t dstrect_w, dstrect_h; 不需要，使用时只用 srcrect_h 和 srcrect_w 就够了（调用时传入的 dstrect 的 h 和 w 可能未赋值，默认是0）
    if (dstrect == NULL)
    {
      dstrect_x = 0;
      dstrect_y = 0;
    }
    else
    {
      dstrect_x = dstrect->x;
      dstrect_y = dstrect->y;
    }

    // 调整矩形的宽度和高度，以防止越界
    if (dstrect_x + srcrect_w > dst->w)
    {
      srcrect_w = dst->w - dstrect_x;
    }
    if (dstrect_y + srcrect_h > dst->h)
    {
      srcrect_h = dst->h - dstrect_y;
    }

    assert(dstrect_x + srcrect_w <= dst->w);
    assert(dstrect_y + srcrect_h <= dst->h);
    assert(srcrect_x + srcrect_w <= src->w);
    assert(srcrect_y + srcrect_h <= src->h);

    for (uint16_t i = 0; i < srcrect_h; i++)
    {
      for (uint16_t j = 0; j < srcrect_w; j++) // srcrect_w not srcrect_h
      {
        dst_pixels[(dstrect_y + i) * dst->w + dstrect_x + j] = src_pixels[(srcrect_y + i) * src->w + srcrect_x + j];
      }
    }
  }
  else
  {
    assert(0);
  }
}

// 往画布的指定矩形区域中填充指定的颜色
void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color)
{
  if (dst->format->BitsPerPixel == 32)
  {
    uint32_t *pixels = (uint32_t *)dst->pixels;
    int dst_w = dst->w;
    int16_t rect_x, rect_y;
    uint16_t rect_h, rect_w;

    if (dstrect == NULL)
    {
      rect_x = 0;
      rect_y = 0;
      rect_w = dst->w;
      rect_h = dst->h;
    }
    else
    {
      rect_x = dstrect->x;
      rect_y = dstrect->y;
      rect_w = dstrect->w;
      rect_h = dstrect->h;
    }

    assert(rect_x + rect_w <= dst->w);
    assert(rect_y + rect_h <= dst->h);

    for (uint16_t i = 0; i < rect_h; i++)
    {
      for (uint16_t j = 0; j < rect_w; j++)
      {
        pixels[(rect_y + i) * dst_w + rect_x + j] = color;
      }
    }
  }
  else
  {
    assert(0);
  }
}

static inline uint32_t translate_color(SDL_Color *color)
{
  return (color->a << 24) | (color->r << 16) | (color->g << 8) | color->b;
}

// 将画布中的指定矩形区域同步到屏幕上
void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h)
{
  if (s->format->BitsPerPixel == 32)
  {
    // 更新整个屏幕
    if (w == 0 && h == 0 && x == 0 && y == 0)
    {
      NDL_DrawRect((uint32_t *)s->pixels, 0, 0, s->w, s->h);
    }

    // 确保更新区域在 Surface 内部
    if (x + w > s->w)
    {
      w = s->w - x;
    }
    if (y + h > s->h)
    {
      h = s->h - y;
    }

    uint32_t *pixels = malloc(w * h * sizeof(uint32_t)); // 这里 pixels 就是将绘图区域的矩形展开（连续的数组） 因此要自己创建该数组
    assert(pixels);
    uint32_t *src = (uint32_t *)s->pixels;
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        int src_index = (y + i) * s->w + (x + j);
        int dst_index = i * w + j;

        // 确保索引在范围内
        assert(src_index < s->w * s->h);
        assert(dst_index < w * h);

        pixels[dst_index] = src[src_index];
      }
    }
    NDL_DrawRect(pixels, x, y, w, h);
    free(pixels);
  }
  else if (s->format->BitsPerPixel == 8)
  {
    // 处理8位像素格式的Surface
    if (w == 0 && h == 0 && x == 0 && y == 0)
    {
      w = s->w;
      h = s->h;
    }

    // 确保更新区域在 Surface 内部
    if (x + w > s->w)
    {
      w = s->w - x;
    }
    if (y + h > s->h)
    {
      h = s->h - y;
    }

    uint32_t *pixels = malloc(w * h * sizeof(uint32_t)); // 为32位颜色值分配内存
    assert(pixels);
    uint8_t *src = (uint8_t *)s->pixels;
    uint32_t *palette = s->format->palette->colors; // 获取调色板

    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        int src_index = (y + i) * s->w + (x + j);
        int dst_index = i * w + j;

        // 确保索引在范围内
        assert(src_index < s->w * s->h);
        assert(dst_index < w * h);

        pixels[dst_index] = translate_color(&s->format->palette->colors[src[src_index]]);
      }
    }
    NDL_DrawRect(pixels, x, y, w, h);
    free(pixels);
  }
  else
  {
    assert(0);
  }
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask)
{
  switch (mask)
  {
  case 0x000000ff:
    return 0;
  case 0x0000ff00:
    return 8;
  case 0x00ff0000:
    return 16;
  case 0xff000000:
    return 24;
  case 0x00000000:
    return 24; // hack
  default:
    assert(0);
  }
}

SDL_Surface *SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
                                  uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask)
{
  assert(depth == 8 || depth == 32);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
  if (depth == 8)
  {
    s->format->palette = malloc(sizeof(SDL_Palette));
    assert(s->format->palette);
    s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
    assert(s->format->palette->colors);
    memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
    s->format->palette->ncolors = 256;
  }
  else
  {
    s->format->palette = NULL;
    s->format->Rmask = Rmask;
    s->format->Rshift = maskToShift(Rmask);
    s->format->Rloss = 0;
    s->format->Gmask = Gmask;
    s->format->Gshift = maskToShift(Gmask);
    s->format->Gloss = 0;
    s->format->Bmask = Bmask;
    s->format->Bshift = maskToShift(Bmask);
    s->format->Bloss = 0;
    s->format->Amask = Amask;
    s->format->Ashift = maskToShift(Amask);
    s->format->Aloss = 0;
  }

  s->format->BitsPerPixel = depth;
  s->format->BytesPerPixel = depth / 8;

  s->w = width;
  s->h = height;
  s->pitch = width * depth / 8;
  assert(s->pitch == width * s->format->BytesPerPixel);

  if (!(flags & SDL_PREALLOC))
  {
    s->pixels = malloc(s->pitch * height);
    assert(s->pixels);
  }

  return s;
}

SDL_Surface *SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
                                      int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask)
{
  SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
                                        Rmask, Gmask, Bmask, Amask);
  assert(pitch == s->pitch);
  s->pixels = pixels;
  return s;
}

void SDL_FreeSurface(SDL_Surface *s)
{
  if (s != NULL)
  {
    if (s->format != NULL)
    {
      if (s->format->palette != NULL)
      {
        if (s->format->palette->colors != NULL)
          free(s->format->palette->colors);
        free(s->format->palette);
      }
      free(s->format);
    }
    if (s->pixels != NULL && !(s->flags & SDL_PREALLOC))
      free(s->pixels);
    free(s);
  }
}

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags)
{
  if (flags & SDL_HWSURFACE)
    NDL_OpenCanvas(&width, &height);
  return SDL_CreateRGBSurface(flags, width, height, bpp,
                              DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
  assert(src && dst);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BitsPerPixel == 8);

  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  assert(dstrect);
  if (w == dstrect->w && h == dstrect->h)
  {
    /* The source rectangle and the destination rectangle
     * are of the same size. If that is the case, there
     * is no need to stretch, just copy. */
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_BlitSurface(src, &rect, dst, dstrect);
  }
  else
  {
    assert(0);
  }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors)
{
  assert(s);
  assert(s->format);
  assert(s->format->palette);
  assert(firstcolor == 0);

  s->format->palette->ncolors = ncolors;
  memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

  if (s->flags & SDL_HWSURFACE)
  {
    assert(ncolors == 256);
    for (int i = 0; i < ncolors; i++)
    {
      uint8_t r = colors[i].r;
      uint8_t g = colors[i].g;
      uint8_t b = colors[i].b;
    }
    SDL_UpdateRect(s, 0, 0, 0, 0);
  }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len)
{
  int i;
  uint8_t(*pdst)[4] = dst;
  uint8_t(*psrc)[4] = src;
  union
  {
    uint8_t val8[4];
    uint32_t val32;
  } tmp;
  int first = len & ~0xf;
  for (i = 0; i < first; i += 16)
  {
#define macro(i)                      \
  tmp.val32 = *((uint32_t *)psrc[i]); \
  *((uint32_t *)pdst[i]) = tmp.val32; \
  pdst[i][0] = tmp.val8[2];           \
  pdst[i][2] = tmp.val8[0];

    macro(i + 0);
    macro(i + 1);
    macro(i + 2);
    macro(i + 3);
    macro(i + 4);
    macro(i + 5);
    macro(i + 6);
    macro(i + 7);
    macro(i + 8);
    macro(i + 9);
    macro(i + 10);
    macro(i + 11);
    macro(i + 12);
    macro(i + 13);
    macro(i + 14);
    macro(i + 15);
  }

  for (; i < len; i++)
  {
    macro(i);
  }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags)
{
  assert(src->format->BitsPerPixel == 32);
  assert(src->w * src->format->BytesPerPixel == src->pitch);
  assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

  SDL_Surface *ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
                                          fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  assert(fmt->Gmask == src->format->Gmask);
  assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
  ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

  return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask)
    p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s)
{
  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s)
{
}
