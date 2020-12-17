# linux tcp 统计模块

> 通过 linux 内核的模块机制，将 tcpstat 置于内核中进行数据统计，方便查看和调用～

## 如何编译内核模块

1. 首先查看 linux 版本，如下：

``` sh
$ uname -a
Linux pedro-OptiPlex-3020 5.4.0-48-generic #52-Ubuntu SMP Thu Sep 10 10:58:49 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux
```

这里 linux 的版本是 `5.4.0-48-generic` ，因此进入到 `/usr/src/linux-headers-5.4.0-48-generic` 目录下。

``` sh
cd /usr/src/linux-headers-5.4.0-48-generic
```

2. 编写模块，如`tcpstat`，详见[tcpstat](./tcpstat.c)

并在当前目录下添加上对应的 `Makefile` ，如下：

``` makefile
# Makefile 4.0
obj-m := tcpstat.o
CURRENT_PATH := $(shell pwd)
LINUX_KERNEL := $(shell uname -r)
LINUX_KERNEL_PATH := /usr/src/linux-headers-$(LINUX_KERNEL)

all:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
```

3. 编译模块并且加载

使用 `make` 命令编译模块，编译成功后，生存 `tcpstat.ko` 文件。

``` sh
make
```

加载该文件至内核：

``` sh
insmod tcpstat.ko
```

查看模块是否存在：

``` sh
✗ lsmod | grep tcpstat
tcpstat                16384  0
```

调用这个模块：

``` sh
cat /proc/tcpstat
```

结果如下：

``` 

ESTABLISHED : 4
SYN_SENT    : 0
SYN_RECV    : 0
FIN_WAIT1   : 0
FIN_WAIT2   : 0
TIME_WAIT   : 0
CLOSE       : 0
CLOSE_WAIT  : 0
LAST_ACK    : 3
LISTEN      : 3
CLOSING     : 0
(null)      : 0
```

so cool~!

## 参考资料

* https://mp.weixin.qq.com/s?src=11&timestamp=1608192718&ver=2771&signature=WpLXHZadTz3p9FhSPkM78ru4VsHiVt06GQ6Hs01qmAnwqfYimopgyApc76QdrNGZi8syfQWNNIE*twhP-baMYEBqYHkpV1gOleHXgFdmFx1Y1OFb7PDDXOVgEdU8BmVm&new=1
