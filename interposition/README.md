# Linux 库打桩

## 编译时打桩

在编译器替换函数，如下：
```sh
gcc -DPMALLOC -c pmalloc.c
gcc -I . -o main main.c pmalloc.o
```

通过预编译查看替换函数：
```sh
gcc -I . -E -o main.i main.c
```

代码见[link](./compile/main.c)

## 链接时打桩

顾名思义，链接时打桩是在链接时替换需要的函数。Linux链接器支持用 --wrap, f的方式来进行打桩，
链接时符号 f 解析成__wrap_f，还会把__real_f 解析成 f。

代码见[link](./link/main.c)

## 运行时打桩

运行时打桩只需访问可执行文件即可，条件最低的打桩操作，通过设置 LD_PRELOAD 环境变量来实现。



## 参考资料
- [“偷梁换柱”的库打桩机制](https://zhuanlan.zhihu.com/p/76036630)