# NEMU
PA的目的是要实现NEMU, 一款经过简化的全系统模拟器
它模拟了一个硬件的世界, 你可以在这个硬件世界中执行程序. 换句话说, 你将要在PA中编写一个用来执行其它程序的程序! 

                         +---------------------+  +---------------------+
                         |     Super Mario     |  |    "Hello World"    |
                         +---------------------+  +---------------------+
                         |    Simulated NES    |  |      Simulated      |
                         |       hardware      |  |       hardware      |
+---------------------+  +---------------------+  +---------------------+
|    "Hello World"    |  |     NES Emulator    |  |        NEMU         |
+---------------------+  +---------------------+  +---------------------+
|      GNU/Linux      |  |      GNU/Linux      |  |      GNU/Linux      |
+---------------------+  +---------------------+  +---------------------+
|    Real hardware    |  |    Real hardware    |  |    Real hardware    |
+---------------------+  +---------------------+  +---------------------+
          (a)                      (b)                     (c)

# ISA
指令集架构
ISA的本质是规范手册
计算机硬件是按照ISA规范手册构造出来的, 而程序(软件)也是按照ISA规范手册编写(或生成)出来的
"软硬件共同协助来支持程序执行"的机理

# PA1

## 最简单的计算机
计算机就只需要做一件事情：
~~~
while (1) {
  从PC指示的存储器位置取出指令;
  执行指令;
  更新PC;
}
~~~
我们也把上面这个最简单的计算机称为"图灵机"(Turing Machine, TRM)
1. 结构上, TRM有存储器, 有PC, 有寄存器, 有加法器
2. 工作方式上, TRM不断地重复以下过程: 从PC指示的存储器位置取出指令, 执行指令, 然后更新PC

我们就可以从状态机模型的视角来理解计算机的工作过程了: 在每个时钟周期到来的时候, 计算机根据当前时序逻辑部件的状态, 在组合逻辑部件的作用下, 计算出并转移到下一时钟周期的新状态

## 程序是个状态机
现在我们就可以通过状态机的视角来解释"程序在计算机上运行"的本质了: 给定一个程序, 把它放到计算机的内存中, 就相当于在状态数量为N的状态转移图中指定了一个初始状态, 程序运行的过程就是从这个初始状态开始, 每执行完一条指令, 就会进行一次确定的状态转移. 也就是说, 程序也可以看成一个状态机! 这个状态机是上文提到的大状态机(状态数量为N)的子集.
从两个互补的视角来看待同一个程序:
1. 一个是以代码(或指令序列)为表现形式的静态视角, 大家经常说的"写程序"/"看代码", 其实说的都是这个静态视角. 这个视角的一个好处是描述精简, 分支, 循环和函数调用的组合使得我们可以通过少量代码实现出很复杂的功能. 但这也可能会使得我们对程序行为的理解造成困难.
2. 另一个是以状态机的状态转移为运行效果的动态视角, 它直接刻画了"程序在计算机上运行"的本质. 但这一视角的状态数量非常巨大, 程序代码中的所有循环和函数调用都以指令的粒度被完全展开, 使得我们难以掌握程序的整体语义. 但对于程序的局部行为, 尤其是从静态视角来看难以理解的行为, 状态机视角可以让我们清楚地了解相应的细节.

## 框架代码初探
ics2022
├── abstract-machine   # 抽象计算机
├── am-kernels         # 基于抽象计算机开发的应用程序
├── fceux-am           # 红白机模拟器
├── init.sh            # 初始化脚本
├── Makefile           # 用于工程打包提交
├── nemu               # NEMU
└── README.md

框架代码把NEMU分成两部分: ISA无关的基本框架和ISA相关的具体实现. NEMU把ISA相关的代码专门放在nemu/src/isa/目录下, 并通过nemu/include/isa.h提供ISA相关API的声明. 这样以后, nemu/src/isa/之外的其它代码就展示了NEMU的基本框架

我们可以键入make -nB, 它会让make程序以"只输出命令但不执行"的方式强制构建目标

##  优美地退出
为了测试大家是否已经理解框架代码, 我们给大家设置一个练习: 如果在运行NEMU之后直接键入q退出, 你会发现终端输出了一些错误信息. 请分析这个错误信息是什么原因造成的, 然后尝试在NEMU中修复它.

nemu/src/utils/state.c中设置 NEMUState nemu_state = {.state = NEMU_STOP}; 初值为 NEMU_STOP
如果运行程序进入函数cpu_exec则会设置为 nemu_state.state = NEMU_RUNNING; 而直接q退出则仍为 NEMU_STOP，导致is_exit_status_bad 返回1
因此 cmd_q 中添加 nemu_state.state = NEMU_QUIT;

# PA2

## YEMU
~~~
#include <stdint.h>
#include <stdio.h>

#define NREG 4
#define NMEM 16

// 定义指令格式
typedef union {
  struct { uint8_t rs : 2, rt : 2, op : 4; } rtype;
  struct { uint8_t addr : 4      , op : 4; } mtype;
  uint8_t inst;
} inst_t;

#define DECODE_R(inst) uint8_t rt = (inst).rtype.rt, rs = (inst).rtype.rs
#define DECODE_M(inst) uint8_t addr = (inst).mtype.addr

uint8_t pc = 0;       // PC, C语言中没有4位的数据类型, 我们采用8位类型来表示
uint8_t R[NREG] = {}; // 寄存器
uint8_t M[NMEM] = {   // 内存, 其中包含一个计算z = x + y的程序
  0b11100110,  // load  6#     | R[0] <- M[y]
  0b00000100,  // mov   r1, r0 | R[1] <- R[0]
  0b11100101,  // load  5#     | R[0] <- M[x]
  0b00010001,  // add   r0, r1 | R[0] <- R[0] + R[1]
  0b11110111,  // store 7#     | M[z] <- R[0]
  0b00010000,  // x = 16
  0b00100001,  // y = 33
  0b00000000,  // z = 0
};

int halt = 0; // 结束标志

// 执行一条指令
void exec_once() {
  inst_t this;
  this.inst = M[pc]; // 取指
  switch (this.rtype.op) {
  //  操作码译码       操作数译码           执行
    case 0b0000: { DECODE_R(this); R[rt]   = R[rs];   break; }
    case 0b0001: { DECODE_R(this); R[rt]  += R[rs];   break; }
    case 0b1110: { DECODE_M(this); R[0]    = M[addr]; break; }
    case 0b1111: { DECODE_M(this); M[addr] = R[0];    break; }
    default:
      printf("Invalid instruction with opcode = %x, halting...\n", this.rtype.op);
      halt = 1;
      break;
  }
  pc ++; // 更新PC
}

int main() {
  while (1) {
    exec_once();
    if (halt) break;
  }
  printf("The result of 16 + 33 is %d\n", M[7]);
  return 0;
}
~~~

## 模式匹配
阅读 RISCV 手册，了解 imm 等是如何获取的
~~~
INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, R, R(rd) = src1 + src2);
~~~
在模式匹配过程的最后有一条inv的规则, 表示"若前面所有的模式匹配规则都无法成功匹配, 则将该指令视为非法指令

make ARCH=riscv32-nemu ALL=dummy run 
“为什么没有输出？” 你倒是按c运行啊

## 程序, 运行时环境与AM
我们以"ISA-平台"的二元组来表示一个架构, 例如mips32-nemu
运行程序所需要的公共要素被抽象成API, 不同的架构只需要实现这些API, 也就相当于实现了支撑程序运行的运行时环境, 这提升了程序开发的效率: 需要的时候只要调用这些API, 就能使用运行时环境提供的相应功能.
如果我们把这些需求都收集起来, 将它们抽象成统一的API提供给程序, 这样我们就得到了一个可以支撑各种程序运行在各种架构上的库了! 具体地, 每个架构都按照它们的特性实现这组API; 应用程序只需要直接调用这组API即可, 无需关心自己将来运行在哪个架构上. 由于这组统一抽象的API代表了程序运行对计算机的需求, 所以我们把这组API称为抽象计算机. 
AM(Abstract machine)作为一个向程序提供运行时环境的库, AM根据程序的需求把库划分成以下模块
~~~
AM = TRM + IOE + CTE + VME + MPE
(在NEMU中)实现硬件功能 -> (在AM中)提供运行时环境 -> (在APP层)运行程序
(在NEMU中)实现更强大的硬件功能 -> (在AM中)提供更丰富的运行时环境 -> (在APP层)运行更复杂的程序
~~~

## 通过批处理模式运行NEMU
我们之前启动NEMU的时候, 每次都需要手动键入c才能运行客户程序. 但如果不是为了使用NEMU中的sdb, 我们其实可以节省c的键入. NEMU中实现了一个批处理模式, 可以在启动NEMU之后直接运行客户程序. 请你阅读NEMU的代码并合适地修改Makefile, 使得通过AM的Makefile可以默认启动批处理模式的NEMU.
abstract-machine/scripts/platform/nemu.mk  NEMUFLAGS += -b 

## 理解 Makefile 
make ARCH=riscv32-nemu ALL=dummy run
从 am-kernels/tests/cpu-tests/Makefile 出发，发现它 @/bin/echo -e "NAME = $*\nSRCS = $<\ninclude $${AM_HOME}/Makefile" > $@ 包含了 abstract-machine/Makefile
在 abstract-machine/Makefile 中：
~~~
### Print build info message
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

# Building dummy-run [riscv32-nemu]
~~~
接着 -include $(AM_HOME)/scripts/$(ARCH).mk 包含架构特定的 Makefile  abstract-machine/scripts/riscv32-nemu.mk
在 abstract-machine/scripts/riscv32-nemu.mk 中又
~~~
include $(AM_HOME)/scripts/isa/riscv32.mk
include $(AM_HOME)/scripts/platform/nemu.mk
~~~
其中 $(AM_HOME)/scripts/isa/riscv32.mk 指定了如 CROSS_COMPILE := riscv64-linux-gnu-
而 $(AM_HOME)/scripts/platform/nemu.mk 则包含 NEMU 的运行参数 NEMUFLAGS += -l $(shell dirname $(IMAGE).elf)/nemu-log.txt 最终通过调用 NEMU 来运行镜像，传递必要的参数 (ARGS 和 IMG)
run: image
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) run ARGS="$(NEMUFLAGS)" IMG=$(IMAGE).bin
$(MAKE) 调用 make 命令 -C $(NEMU_HOME) 选项表示在 $(NEMU_HOME) 目录中执行 make

## 重新认识计算机: 计算机是个抽象层
微观视角: 程序是个状态机
宏观视角: 计算机是个抽象层

## 输入输出
nemu/src/device 实现硬件
abstract-machine/am/src/platform/nemu/ioe 实现软件
为了实现各种设备，我们要同时实现硬件和软件
nemu-main 中 IFDEF(CONFIG_DEVICE, init_device());
IOE提供三个API:
~~~
bool ioe_init();
void ioe_read(int reg, void *buf);
void ioe_write(int reg, void *buf);
~~~
这里的reg寄存器并不是上文讨论的设备寄存器, 因为设备寄存器的编号是架构相关的. 在IOE中, 我们希望采用一种架构无关的"抽象寄存器", 这个reg其实是一个功能编号, 我们约定在不同的架构中, 同一个功能编号的含义也是相同的, 这样就实现了设备寄存器的抽象

cpu_exec()在执行每条指令之后就会调用device_update()函数

## 时钟
nemu/src/device/timer.c模拟了i8253计时器的功能
i8253计时器初始化时会分别注册0x48处长度为8个字节的端口, 以及0xa0000048处长度为8字节的MMIO空间, 它们都会映射到两个32位的RTC寄存器. CPU可以访问这两个寄存器来获得用64位表示的当前时间
typedef struct
{
  const char *name;
  // we treat ioaddr_t as paddr_t here
  paddr_t low;
  paddr_t high;
  void *space;
  io_callback_t callback;
} IOMap;
static uint32_t *rtc_port_base = NULL; 为目标空间
#define CONFIG_RTC_MMIO 0xa0000048
add_mmio_map("rtc", CONFIG_RTC_MMIO, rtc_port_base, 8, rtc_io_handler); 作内存I/O映射，rtc_io_handler为回调函数
{
  "rtc"
  [0xa0000048,0xa0000048 + 8 - 1]
  rtc_port_base
  rtc_io_handler
}
测试程序 am-kernels/tests/am-tests/src/tests/rtc.c 中调用 io_read(AM_TIMER_UPTIME).us
~~~
#define io_read(reg) \
  ({ reg##_T __io_param; \
    ioe_read(reg, &__io_param); \
    __io_param; })
~~~
这里会调用 ioe_read 并返回一个结构体（这就是为什么可以有 io_read(AM_TIMER_UPTIME).us ） 因此函数传递参数中的指针一定要处理，至少要给它赋值
abstract-machine/am/src/platform/nemu/ioe/ioe.c 中实现了上述的三个IOE API,ioe_read()和ioe_write()都是通过抽象寄存器的编号索引到一个处理函数, 然后调用它
这里调用 abstract-machine/am/src/platform/nemu/ioe/timer.c 中的 __am_timer_uptime，也就是我们要实现的软件部分
~~~
#define DEVICE_BASE 0xa0000000
#define RTC_ADDR (DEVICE_BASE + 0x0000048) // 这里 RTC_ADDR = 0xa0000048，与上面硬件的映射起点地址相同
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) // 从0开始计数
{
  // RTC_ADDR 是 RTC 存储的地址（内存映射）访问即可获得时间
  // 0xa0000048处长度为8字节的MMIO空间, 它们都会映射到两个32位的RTC寄存器. CPU可以访问这两个寄存器来获得用64位表示的当前时间.
  uint32_t high_part = inl(RTC_ADDR + 4);              // 高32位 先读高32位
  uint32_t low_part = inl(RTC_ADDR);                   // 低32位
  uptime->us = ((uint64_t)high_part << 32) | low_part; // 拼接
}

static inline uint8_t inb(uintptr_t addr) { return *(volatile uint8_t *)addr; } // 转换成指针再解引用访问内容
~~~
我们已知只有 map_read()和map_write() 会调用相应的回调函数，即执行硬件中定义的相关代码
但我们这里只用了 inl 函数，并没有显示调用 map_read 函数，那我们是如何实现硬件和软件的联合工作呢？
结合 __am_timer_uptime 反汇编代码
~~~
8000113c <__am_timer_uptime>:
8000113c:	a00007b7          	lui	a5,0xa0000
80001140:	04c7a703          	lw	a4,76(a5) # a000004c <_end+0x1ff6304c>
80001144:	0487a783          	lw	a5,72(a5)
80001148:	00e52223          	sw	a4,4(a0)
8000114c:	00f52023          	sw	a5,0(a0)
80001150:	00008067          	ret
~~~
注意这里 inl 被编译为了 lw 指令，而在 nemu/src/isa/riscv32/inst.c 中我们实现了 lw 
INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw, I, R(rd) = Mr(src1 + imm, 4));
其中 Mr 为 #define Mr vaddr_read，vaddr_read 会调用 paddr_read，接着调用 mmio_read，然后调用 map_read，触发设备的回调函数
这里还有一个坑，rtc_io_handler，当 offset 为4时（即读取高32位时），会更新时间，因此我们的 __am_timer_uptime 要先读取高32位，再读取低32位

至此，我们理清了软硬件如何协同工作

## 键盘
~~~
void send_key(uint8_t scancode, bool is_keydown)
{
  if (nemu_state.state == NEMU_RUNNING && keymap[scancode] != _KEY_NONE)
  {
    uint32_t am_scancode = keymap[scancode] | (is_keydown ? KEYDOWN_MASK : 0);
    key_enqueue(am_scancode);
  }
}
~~~
最高位为 keydown 标志，低31位为 keycode
软件中按上述要求实现
~~~
void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd)
{
  uint32_t kc = inl(KBD_ADDR);
  kbd->keydown = kc & KEYDOWN_MASK ? true : false;
  kbd->keycode = kc & ~KEYDOWN_MASK;
}
~~~

## VGA
~~~
vgactl_port_base = (uint32_t *)new_space(8);
vgactl_port_base[0] = (screen_width() << 16) | screen_height();
~~~
nemu/src/device/vga.c 中 vgactl_port_base 的低32位中的高16位是 width，低16位是 height （这里不是内存中小段存储，就是一个32位数的构成，规定左边高右边低）
#define SYNC_ADDR (VGACTL_ADDR + 4) vgactl_port_base 的高32位是 sync
注意区分 数字的位构成 与 数字在内存中的存储格式

我们的 __am_gpu_fbdraw 只是向 FB_ADDR 开始的部分区域写入信息（只是把需要绘制的区域的像素点数据放到frame buffer中），并根据 sync 决定是否要同步更新屏幕s，如果要同步，则硬件部分中的 vga_update_screen 会调用 update_screen，执行 SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(uint32_t)) ，输出信息
测试时，传入的 ctl->sync 为 true 则会调用 outl(SYNC_ADDR, 1) ，则此时 sync = vgactl_port_base[1] 非0，会调用 update_screen()

## 冯诺依曼计算机系统
游戏可以抽象成一个死循环:
~~~
while (1) {
  等待新的一帧();  // AM_TIMER_UPTIME
  处理用户按键();  // AM_INPUT_KEYBRD
  更新游戏逻辑();  // TRM
  绘制新的屏幕();  // AM_GPU_FBDRAW
}
~~~