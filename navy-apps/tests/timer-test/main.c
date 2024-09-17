#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h> // 需要包含这个头文件
#include <NDL.h>

int main()
{
    NDL_Init(0);

    uint32_t ms = NDL_GetTicks() + 500; // 将ms初始值设为当前时间加500毫秒

    while (1)
    {
        if (NDL_GetTicks() >= ms)
        {
            printf("hello world\n");
            ms += 500; // 更新ms为下一个500毫秒间隔
        }
    }

    NDL_Quit();
    return 0;
}
