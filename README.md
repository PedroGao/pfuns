# c/c++ 学习笔记

> 有一些底层的东西必须要 c/c++ 才能实现，所以特开一个仓库，专门来记录底层的学习

## 内容

* jit（即时编译）的[实现原理](./jit.c)
* malloc（内存管理）的[实现原理](./pmalloc.c)
* container（容器）[实现原理](./container.go)
* VM（虚拟机）[实现原理](./vm.cpp)
* ping[实现原理](./ping.py)
* ether以太网[实现原理](./send_ether.py)
* linux [内核模块实现](./linux_module.md)

## 笔记

[虚拟机](./vm.cpp) 是一个极其精巧的微型虚拟机实现，通过 `c/c++` 来模拟 CPU，
有名义上的内存、寄存器和中间代码，中间代码定义是非常简单易懂，是学习虚拟机的绝佳材料。

可通过如下命令来运行一个中间代码的 2048 小游戏：

``` sh
g++ vm.cpp -o vm
./vm 2048.obj
```

## 参考资料

* project-base-learning：https://github.com/tuvtran/project-based-learning
* 简单的容器实现：https://mp.weixin.qq.com/s/wTby-KcsQdss7Sg20ay6vw
* 简单的虚拟机实现：https://justinmeiners.github.io/lc3-vm/
* Linux网络编程：https://network.fasionchan.com/zh_CN/latest/index.html
