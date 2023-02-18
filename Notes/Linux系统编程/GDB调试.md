### fork多进程调试

在gdb跟踪到fork函数之前，使用如下：

```shell
(gdb) set follow-fork-mode child
```

可以选择后续跟踪子进程的控制流
```shell
(gdb) set follow-fork-mode child
```

可以选择后续跟踪父进程的控制流

gdb默认是跟踪父进程的控制流的
