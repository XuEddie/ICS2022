#include "elf_parser.h"

FunctionInfoArray func_info_array;

void init_ftracer(const char *elf_filename)
{
    FILE *elf_file = fopen(elf_filename, "rb");
    if (!elf_file)
    {
        perror("fopen");
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    // 读取 ELF 头
    Elf32_Ehdr elf_header;
    if (fread(&elf_header, 1, sizeof(Elf32_Ehdr), elf_file) != sizeof(Elf32_Ehdr))
    {
        perror("Failed to read ELF header");
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    // 查找节头表位置
    if (fseek(elf_file, elf_header.e_shoff, SEEK_SET) != 0)
    {
        perror("Failed to seek to section header table");
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    Elf32_Shdr *section_headers = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr) * elf_header.e_shnum);
    if (!section_headers)
    {
        perror("Failed to allocate memory for section headers");
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    if (fread(section_headers, sizeof(Elf32_Shdr), elf_header.e_shnum, elf_file) != elf_header.e_shnum)
    {
        perror("Failed to read section headers");
        free(section_headers);
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    // 查找符号表和字符串表
    Elf32_Shdr *symtab_hdr = NULL;
    Elf32_Shdr *strtab_hdr = NULL;

    // 找到节头字符串表（用于查找节名称）
    Elf32_Shdr *shstrtab_hdr = &section_headers[elf_header.e_shstrndx];
    char shstrtab[shstrtab_hdr->sh_size];
    if (fseek(elf_file, shstrtab_hdr->sh_offset, SEEK_SET) != 0)
    {
        perror("Failed to seek to section header string table");
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }
    if (fread(shstrtab, shstrtab_hdr->sh_size, 1, elf_file) != 1)
    {
        perror("Failed to read section header string table");
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    // 查找符号表和字符串表
    for (int i = 0; i < elf_header.e_shnum; i++)
    {
        if (section_headers[i].sh_type == SHT_SYMTAB)
        {
            symtab_hdr = &section_headers[i];
        }
        if (section_headers[i].sh_type == SHT_STRTAB)
        {
            const char *name = &shstrtab[section_headers[i].sh_name];
            if (strcmp(name, ".strtab") == 0)
            {
                strtab_hdr = &section_headers[i];
            }
        }
    }

    if (!symtab_hdr || !strtab_hdr)
    {
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    // 读取符号表
    Elf32_Sym *symtab = (Elf32_Sym *)malloc(symtab_hdr->sh_size);
    if (!symtab || fseek(elf_file, symtab_hdr->sh_offset, SEEK_SET) != 0 ||
        fread(symtab, 1, symtab_hdr->sh_size, elf_file) != symtab_hdr->sh_size)
    {
        perror("Failed to read symbol table");
        free(symtab);
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    // 读取字符串表
    char *strtab = (char *)malloc(strtab_hdr->sh_size);
    if (!strtab || fseek(elf_file, strtab_hdr->sh_offset, SEEK_SET) != 0 ||
        fread(strtab, 1, strtab_hdr->sh_size, elf_file) != strtab_hdr->sh_size)
    {
        perror("Failed to read string table");
        free(symtab);
        free(strtab);
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    // 初始化 FunctionInfoArray
    func_info_array.functions = NULL;
    func_info_array.count = 0;
    int num_symbols = symtab_hdr->sh_size / sizeof(Elf32_Sym);

    // 计算函数的数量
    for (int i = 0; i < num_symbols; i++)
    {
        if (ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC)
        {
            func_info_array.count++;
        }
    }

    // 分配内存保存函数信息
    func_info_array.functions = (FunctionInfo *)malloc(func_info_array.count * sizeof(FunctionInfo));
    if (!func_info_array.functions)
    {
        perror("Failed to allocate memory for function info array");
        free(symtab);
        free(strtab);
        fclose(elf_file);
        func_info_array.functions = NULL;
        func_info_array.count = 0;
        return;
    }

    int index = 0;

    // 填充 FunctionInfoArray
    for (int i = 0; i < num_symbols; i++)
    {
        if (ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC)
        {
            func_info_array.functions[index].name = strdup(&strtab[symtab[i].st_name]);
            func_info_array.functions[index].value = symtab[i].st_value;
            func_info_array.functions[index].size = symtab[i].st_size;
            index++;
        }
    }

    print_function_info_array(&func_info_array);

    // 释放内存并关闭文件
    free(symtab);
    free(strtab);
    fclose(elf_file);
}

// 释放 FunctionInfoArray 内存
void free_function_info_array(FunctionInfoArray *info_array)
{
    for (size_t i = 0; i < info_array->count; i++)
    {
        free(info_array->functions[i].name);
    }
    free(info_array->functions);
}

// 打印 FunctionInfoArray 的内容
void print_function_info_array(const FunctionInfoArray *info_array)
{
    if (info_array->functions)
    {
        for (size_t i = 0; i < info_array->count; i++)
        {
            printf("Function: %s, Address: 0x%x, Size: %d\n",
                   info_array->functions[i].name,
                   info_array->functions[i].value,
                   info_array->functions[i].size);
        }
    }
}

char *find_function(uint32_t addr)
{
    for (size_t i = 0; i < func_info_array.count; i++)
    {
        if (addr >= func_info_array.functions[i].value && addr < func_info_array.functions[i].value + func_info_array.functions[i].size)
        {
            return func_info_array.functions[i].name;
        }
    }
    return NULL;
}
