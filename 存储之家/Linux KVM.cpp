1、KVM概述
	KVM全称:基于内核的虚拟机(Kernel-based Virtual Machine),它是Linux的一个内核模块;KVM是基于虚拟化扩展(Intel VT 或者 AMD-V)的 
X86硬件的开源的Linux原生的全虚拟化解决方案。KVM 中,虚拟机被实现为常规的Linux进程,由标准Linux调度程序进行调度;虚机的每个虚拟CPU 
被实现为一个常规的Linux进程。这使得KMV能够使用Linux内核的已有功能。
	使用KVM,可以运行多个运行未修改的Linux或Windows映像的虚拟机。每个虚拟机都有专用的虚拟化硬件:网卡,磁盘,图形适配器等。
	运行在内核空间,提供CPU和内存的虚级化,以及客户机的I/O拦截。
	
2、KVM基本架构
	1.KVM驱动:主要负责虚拟机的创建、虚拟内存分配、VCPU寄存器的读写以及VCPU的运行。已经是linux kernel的一个模块。
	2.QEMU:模拟虚拟机的用户空间组件,提供I/O设备模型,访问外设的途径。
	