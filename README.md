# mini-Linux-kernel
A simple linux kernel based on `riscv-64` architecture.

It now supports:

* A simple trap handler to handle Interrupt and Exception

* multi-process schedule based on time interrupt with two different schedule mechanisms to choose from:

  * SJF (shortest job first) Schedule policy
  * Priority Schedule policy

* VA to PA address mapping

  VA uses the format of [Sv39](https://www.five-embeddev.com/riscv-isa-manual/latest/supervisor.html#sec:sv39) in riscv and every page has a 4KB block size

* User mode and Kernel mode transform

  Some simple system calls are supported, such as SYS_WRITE, SYS_GETPID...



