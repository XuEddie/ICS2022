#include <NDL.h>
#include <SDL.h>
#include <assert.h>

#define keyname(k) #k,

static const char *keyname[] = {
    "NONE",
    _KEYS(keyname)};

static uint8_t keystate[sizeof(keyname) / sizeof(keyname[0])] = {0};

int SDL_PushEvent(SDL_Event *ev)
{
  assert(0);
  return 0;
}

int SDL_PollEvent(SDL_Event *ev)
{
  char buf[64];
  if (NDL_PollEvent(buf, sizeof(buf)) == 1)
  {
    char type[8];
    char keycode[32];
    sscanf(buf, "%s %s", type, keycode);

    if (strcmp(type, "kd") == 0)
    {
      ev->type = SDL_KEYDOWN;
    }
    else if (strcmp(type, "ku") == 0)
    {
      ev->type = SDL_KEYUP;
    }

    for (int i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++)
    {
      if (strcmp(keycode, keyname[i]) == 0)
      {
        ev->key.keysym.sym = i;
        keystate[i] = (ev->type == SDL_KEYDOWN) ? 1 : 0; // 更新键盘状态
        break;
      }
    }
    return 1;
  }
  else
  {
    return 0;
  }
}

int SDL_WaitEvent(SDL_Event *ev)
{
  while (1)
  {
    char buf[64];
    if (NDL_PollEvent(buf, sizeof(buf)) == 1)
    {
      char type[8];
      char keycode[32];
      sscanf(buf, "%s %s", type, keycode);

      if (strcmp(type, "kd") == 0)
      {
        ev->type = SDL_KEYDOWN;
      }
      else if (strcmp(type, "ku") == 0)
      {
        ev->type = SDL_KEYUP;
      }

      for (int i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++)
      {
        if (strcmp(keycode, keyname[i]) == 0)
        {
          ev->key.keysym.sym = i;
          keystate[i] = (ev->type == SDL_KEYDOWN) ? 1 : 0; // 更新键盘状态
          break;
        }
      }
      return 1;
    }
  }
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask)
{
  assert(0);
  return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys)
{
  if (numkeys != NULL)
  {
    *numkeys = sizeof(keystate) / sizeof(keystate[0]);
  }
  return keystate;
}
