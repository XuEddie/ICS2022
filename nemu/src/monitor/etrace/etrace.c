#include "etrace.h"

#ifdef CONFIG_ETRACE
#define ETRACE_LENGTH 512
typedef struct EtraceNode
{
    char data[ETRACE_LENGTH];
    struct EtraceNode *next;
} EtraceNode;

typedef struct
{
    EtraceNode *head;
    EtraceNode *tail;
    int size;
} EtraceList;
EtraceList etrace_list = {NULL, NULL, 0};
int ecall_depth = 0;
#endif

void init_etrace_list()
{
#ifdef CONFIG_ETRACE
    etrace_list.head = NULL;
    etrace_list.tail = NULL;
    etrace_list.size = 0;
#endif
}

void add_etrace_entry(const char *entry)
{
#ifdef CONFIG_FTRACE
    EtraceNode *new_node = (EtraceNode *)malloc(sizeof(EtraceNode));
    if (new_node == NULL)
    {
        // 处理内存分配失败
        printf("Error: Unable to allocate memory for etrace entry\n");
        return;
    }
    strncpy(new_node->data, entry, ETRACE_LENGTH);
    new_node->data[ETRACE_LENGTH - 1] = '\0'; // 确保字符串以 null 终止
    new_node->next = NULL;

    if (etrace_list.tail)
    {
        etrace_list.tail->next = new_node;
    }
    else
    {
        etrace_list.head = new_node;
    }
    etrace_list.tail = new_node;
    etrace_list.size++;
#endif
}

void etrace_call(vaddr_t pc, vaddr_t dnpc)
{
#ifdef CONFIG_ETRACE
    char buffer[ETRACE_LENGTH];
    int len = snprintf(buffer, sizeof(buffer), FMT_WORD ": ", pc);

    for (int i = 0; i < ecall_depth; i++)
    {
        len += snprintf(buffer + len, sizeof(buffer) - len, "  ");
    }

    char *func = find_function(dnpc);
    assert(func != NULL);
    if (func != NULL)
    {
        snprintf(buffer + len, sizeof(buffer) - len, "ecall [%s@0x%08x]", func, dnpc);
    }

    add_etrace_entry(buffer);
    ecall_depth++;
#endif
}

void etrace_ret(vaddr_t pc, vaddr_t dnpc)
{
#ifdef CONFIG_ETRACE
    char buffer[ETRACE_LENGTH];
    ecall_depth--;
    int len = snprintf(buffer, sizeof(buffer), FMT_WORD ": ", pc);

    for (int i = 0; i < ecall_depth; i++)
    {
        len += snprintf(buffer + len, sizeof(buffer) - len, "  ");
    }

    char *func = find_function(pc);
    assert(func != NULL);
    if (func != NULL)
    {
        snprintf(buffer + len, sizeof(buffer) - len, "mret  [%s]", func);
    }

    add_etrace_entry(buffer);
#endif
}

void print_etracebuf()
{
#ifdef CONFIG_ETRACE
    printf("ETRACER:\n");
    EtraceNode *current = etrace_list.head;
    while (current != NULL)
    {
        printf("%s\n", current->data);
        current = current->next;
    }
#endif
}

void free_etrace_list()
{
#ifdef CONFIG_ETRACE
    EtraceNode *current = etrace_list.head;
    while (current != NULL)
    {
        EtraceNode *next = current->next;
        free(current);
        current = next;
    }
    etrace_list.head = NULL;
    etrace_list.tail = NULL;
    etrace_list.size = 0;
#endif
}