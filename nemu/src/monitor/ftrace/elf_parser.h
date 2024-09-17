#ifndef _ELF_PARSER_H_
#define _ELF_PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

// 定义结构体保存函数名、地址和值
typedef struct
{
    char *name;
    uint32_t value;
    uint32_t size;
} FunctionInfo;

// 定义函数返回结构体数组及其大小
typedef struct
{
    FunctionInfo *functions;
    size_t count;
} FunctionInfoArray;

void init_ftracer(const char *elf_filename);
void free_function_info_array(FunctionInfoArray *info_array);
void print_function_info_array(const FunctionInfoArray *info_array);
char *find_function(uint32_t addr);

#endif