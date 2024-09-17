#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>
#include <stdlib.h>
#include <string.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...)
{
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner()
{
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt()
{
  sh_printf("sh> ");
}

#define CMD_ECHO "echo"

static void sh_handle_cmd(const char *cmd)
{
  assert(cmd);

  char *cpy = (char *)malloc(strlen(cmd) + 1);
  strcpy(cpy, cmd);
  // remove last return
  size_t len = strlen(cpy);
  if (len > 0 && cpy[len - 1] == '\n')
  {
    cpy[len - 1] = '\0';
  }

  // extract the first token as the command
  char *cmd_ = strtok(cpy, " ");
  if (cmd_ == NULL)
  {
    free(cpy);
    return;
  }

  // lite echo
  if (strncmp(cmd_, CMD_ECHO, strlen(CMD_ECHO)) == 0)
  {
    sh_printf("%s\n", cmd_ + strlen(cmd_) + 1);
    free(cpy);
    return;
  }

  // execve
  char *argv[20];
  char *arg;
  int argc = 0;
  while ((arg = strtok(NULL, " ")))
  {
    argv[argc++] = arg;
  }
  argv[argc] = NULL;

  SDL_Quit();
  execve(cmd_, argv, environ);
  perror("execve");
  free(cpy);
}

void builtin_sh_run()
{
  sh_banner();
  sh_prompt();

  while (1)
  {
    SDL_Event ev;
    if (SDL_PollEvent(&ev))
    {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN)
      {
        const char *res = term->keypress(handle_key(&ev));
        if (res)
        {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
