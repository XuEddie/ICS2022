#include "ftracer.h"

#ifdef CONFIG_FTRACE
#define FTRACE_LENGTH 512
typedef struct FtraceNode
{
    char data[FTRACE_LENGTH];
    struct FtraceNode *next;
} FtraceNode;

typedef struct
{
    FtraceNode *head;
    FtraceNode *tail;
    int size;
} FtraceList;
FtraceList ftrace_list = {NULL, NULL, 0};
int call_depth = 0;
#endif

void init_ftrace_list()
{
#ifdef CONFIG_FTRACE
    ftrace_list.head = NULL;
    ftrace_list.tail = NULL;
    ftrace_list.size = 0;
#endif
}

void add_ftrace_entry(const char *entry)
{
#ifdef CONFIG_FTRACE
    FtraceNode *new_node = (FtraceNode *)malloc(sizeof(FtraceNode));
    if (new_node == NULL)
    {
        // 处理内存分配失败
        printf("Error: Unable to allocate memory for ftrace entry\n");
        return;
    }
    strncpy(new_node->data, entry, FTRACE_LENGTH);
    new_node->data[FTRACE_LENGTH - 1] = '\0'; // 确保字符串以 null 终止
    new_node->next = NULL;

    if (ftrace_list.tail)
    {
        ftrace_list.tail->next = new_node;
    }
    else
    {
        ftrace_list.head = new_node;
    }
    ftrace_list.tail = new_node;
    ftrace_list.size++;
#endif
}

void ftrace_call(vaddr_t pc, vaddr_t dnpc)
{
#ifdef CONFIG_FTRACE
    char buffer[FTRACE_LENGTH];
    int len = snprintf(buffer, sizeof(buffer), FMT_WORD ": ", pc);

    for (int i = 0; i < call_depth; i++)
    {
        len += snprintf(buffer + len, sizeof(buffer) - len, "  ");
    }

    char *func = find_function(dnpc);
    assert(func != NULL);
    if (func != NULL)
    {
        snprintf(buffer + len, sizeof(buffer) - len, "call [%s@0x%08x]", func, dnpc);
    }

    add_ftrace_entry(buffer);
    call_depth++;
#endif
}

void ftrace_ret(vaddr_t pc, vaddr_t dnpc)
{
#ifdef CONFIG_FTRACE
    char buffer[FTRACE_LENGTH];
    call_depth--;
    int len = snprintf(buffer, sizeof(buffer), FMT_WORD ": ", pc);

    for (int i = 0; i < call_depth; i++)
    {
        len += snprintf(buffer + len, sizeof(buffer) - len, "  ");
    }

    char *func = find_function(pc);
    assert(func != NULL);
    if (func != NULL)
    {
        snprintf(buffer + len, sizeof(buffer) - len, "ret  [%s]", func);
    }

    add_ftrace_entry(buffer);
#endif
}

void print_ftracebuf()
{
#ifdef CONFIG_FTRACE
    printf("FTRACER:\n");
    FtraceNode *current = ftrace_list.head;
    while (current != NULL)
    {
        printf("%s\n", current->data);
        current = current->next;
    }
#endif
}

void free_ftrace_list()
{
#ifdef CONFIG_FTRACE
    FtraceNode *current = ftrace_list.head;
    while (current != NULL)
    {
        FtraceNode *next = current->next;
        free(current);
        current = next;
    }
    ftrace_list.head = NULL;
    ftrace_list.tail = NULL;
    ftrace_list.size = 0;
#endif
}