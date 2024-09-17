#include "iringbuf.h"

#ifdef CONFIG_IRINGBUF
#define IRINGBUF_LINES 10
#define IRINGBUF_LENGTH 128

char iringbuf[IRINGBUF_LINES][IRINGBUF_LENGTH];
int end = 0;
int id = 0;
#endif

void print_iringbuf()
{
#ifdef CONFIG_IRINGBUF
    printf("IRINGBUF:\n");
    for (int i = 0; i < end; i++)
    {
        if (i == (id - 1) % IRINGBUF_LINES)
        {
            printf("%s\t*\n", iringbuf[i]);
        }
        else
        {
            printf("%s\n", iringbuf[i]);
        }
    }
#endif
}

void update_iringbuf(Decode *s)
{
#ifdef CONFIG_IRINGBUF
    char *p_ = iringbuf[id % IRINGBUF_LINES];
    p_ += snprintf(p_, sizeof(iringbuf[id % IRINGBUF_LINES]), FMT_WORD ":", s->pc);
    int ilen_ = s->snpc - s->pc;
    int i_;
    uint8_t *inst_ = (uint8_t *)&s->isa.inst.val;
    for (i_ = ilen_ - 1; i_ >= 0; i_--)
    {
        p_ += snprintf(p_, 4, " %02x", inst_[i_]);
    }
    int ilen_max_ = MUXDEF(CONFIG_ISA_x86, 8, 4);
    int space_len_ = ilen_max_ - ilen_;
    if (space_len_ < 0)
        space_len_ = 0;
    space_len_ = space_len_ * 3 + 1;
    memset(p_, ' ', space_len_);
    p_ += space_len_;
#ifndef CONFIG_ISA_loongarch32r
    void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
    disassemble(p_, iringbuf[id % IRINGBUF_LINES] + sizeof(iringbuf[id % IRINGBUF_LINES]) - p_,
                MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen_);
#else
    p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
    id++;
    if (end < IRINGBUF_LINES)
    {
        end++;
    }
#endif
}