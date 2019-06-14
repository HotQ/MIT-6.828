6.828: Operating System Engineering
=================================== 
<!-- TOC -->
- [Lect 1](#lect-1)
<!-- /TOC -->

## Lect 1

**LEC 1:** [Operating systems](Lect_1/Operating_systems.md) (handouts: [xv6 source](readings/xv6-rev7.pdf), [xv6 book](readings/book-rev7.pdf))  
**Preparation**: Look over the xv6 book and source. We will cover chapter 0 today.  
**In class**: [shell exercises](Lect_1/shell_exercises.md)  
**Assignment**: [Lab 1: C, Assembly, Tools, and Bootstrapping](Lect_1/Lab_1_Booting_a_PC.md)


# 记录

the BIOS in a PC is "hard-wired" to the physical address range `F0000H`-`FFFFFH`。

The PC starts executing with CS = 0xf000 and IP = 0xfff0.
`0xf000:0xfff0=0xffff0`, 在进行完这些初始化后，BIOS 便将Boot Loader从一个合适的位置装载到内存0x7C00处，

`0x7c00`历史原因
```
0x7FFF - 512 - 512 + 1 = 0x7C00 
```

boot.S 主要是将处理器从实模式转换到 32 位的保护模式，因为只有在保护模式中我们才能访问到物理内存高于 1MB 的空间。
main.c 的主要作用是将内核的可执行代码从硬盘镜像中读入到内存中， 具体的方式是运用 x86 专门的 I/O 指令读取。



8086中有20根地址总线，CS:IP均为16位，最大访问地址为 1MB，`FFFFH:FFFFH = 10FFEFH`，也就是说从 100000H 到 10FFEFH 无法访问，当访问这段地址时，会产生 wrap-around，也就是实际访问地址会对 1MB 求模。
到了 80286 中有 24 根地址总线，最大访问地址为 16MB。这个时候，不会产生 wrap-around，为了向下兼容 8086，需要使用第 21 根地址总线。
所以 IBM 的工程师使用 PS/2 Controller 输出端口中多余的端口来管理 A20 gate，也就是第 21 根地址总线（从 0 开始）。
注意下表，0x60用于读写数据，0x64用于读写状态。

