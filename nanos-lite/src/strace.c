#include "strace.h"

#ifdef HAS_STRACE
#define STRACE_LENGTH 512
typedef struct StraceNode
{
    char data[STRACE_LENGTH];
    struct StraceNode *next;
} StraceNode;

typedef struct
{
    StraceNode *head;
    StraceNode *tail;
    int size;
} StraceList;
StraceList strace_list = {NULL, NULL, 0};
#endif

void init_strace_list()
{
#ifdef HAS_STRACE
    strace_list.head = NULL;
    strace_list.tail = NULL;
    strace_list.size = 0;
#endif
}

void add_strace_entry(const char *entry)
{
#ifdef HAS_STRACE
    StraceNode *new_node = (StraceNode *)malloc(sizeof(StraceNode));
    if (new_node == NULL)
    {
        // 处理内存分配失败
        printf("Error: Unable to allocate memory for strace entry\n");
        return;
    }
    strncpy(new_node->data, entry, STRACE_LENGTH);
    new_node->data[STRACE_LENGTH - 1] = '\0'; // 确保字符串以 null 终止
    new_node->next = NULL;

    if (strace_list.tail)
    {
        strace_list.tail->next = new_node;
    }
    else
    {
        strace_list.head = new_node;
    }
    strace_list.tail = new_node;
    strace_list.size++;
#endif
}

void strace_call(uintptr_t pc, char *name)
{
#ifdef HAS_STRACE
    char buffer[STRACE_LENGTH];
    snprintf(buffer, sizeof(buffer), "0x%08x: %s", pc, name);
    add_strace_entry(buffer);
#endif
}

void print_stracebuf()
{
#ifdef HAS_STRACE
    printf("STRACER:\n");
    StraceNode *current = strace_list.head;
    while (current != NULL)
    {
        printf("%s\n", current->data);
        current = current->next;
    }
#endif
}