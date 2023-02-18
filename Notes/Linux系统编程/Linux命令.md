#### strace

用于查看程序运行时调用的系统调用，用法如下：

```shell
# strace ./a.out
```

上述命令运行a.out并打印运行过程中调用的系统调用



#### ulimit

用于查看系统各个属性的阈值，推荐使用如下用法：

```shell
# ulimit -a
```

更多细节通过--help查看



#### umask

```shell
# umask
```

查看当前终端进程的文件默认权限的反掩码，root用户默认是0022，普通用户默认是 0002（3位8进制数组成）

文件（或目录）的初始权限 = 文件（或目录）的最大默认权限 & ~umask反掩码

在 Linux 系统中，文件和目录的最大默认权限是不一样的：

- 对文件来讲，其可拥有的最大默认权限是 0666，即 rw-rw-rw-。也就是说，使用文件的任何用户都没有执行（x）权限。原因很简单，执行权限是文件的最高权限，赋予时绝对要慎重，因此绝不能在新建文件的时候就默认赋予，只能通过用户手工赋予。
- 对目录来讲，其可拥有的最大默认权限是 0777，即 rwxrwxrwx。

同样，使用如下可以临时修改反掩码（8进制数）

```shell
#umask 0011
```

永久修改需要修改/etc/profile文件，细节自行查询

自测：umask命令的参数将输入的字符串会检查长度在4以内，数字是否在0-7之间，**但是umask命令不具备特殊权限控制**，第一个数字即使输入也是无效的，比如输入了1111字符串，最后得到的umask等于0111



#### 文件特殊权限

对应编码如下：

```shell
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# chmod 0777 a
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# ll a
-rwxrwxrwx 1 root root 0 Feb 13 16:31 a*
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# chmod 1777 a
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# ll a
-rwxrwxrwt 1 root root 0 Feb 13 16:31 a*
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# chmod 2777 a
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# ll a
-rwxrwsrwx 1 root root 0 Feb 13 16:31 a*
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# chmod 3777 a
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# ll a
-rwxrwsrwt 1 root root 0 Feb 13 16:31 a*
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# chmod 4777 a
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# ll a
-rwsrwxrwx 1 root root 0 Feb 13 16:31 a*
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# chmod 5777 a
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# ll a
-rwsrwxrwt 1 root root 0 Feb 13 16:31 a*
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# chmod 6777 a
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# ll a
-rwsrwsrwx 1 root root 0 Feb 13 16:31 a*
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# chmod 7777 a
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# ll a
-rwsrwsrwt 1 root root 0 Feb 13 16:31 a*
```

自测：chmod命令将权限字符串如3777转换成8进制数进行处理，会检查长度是否在4以内，数字是否在0-7之间，若开头写了0并不表示8进制，而是特殊权限的控制，换言之，**chmod支持特殊权限控制**，详情见https://blog.csdn.net/mengmeng_921/article/details/128236146



### readlink

查询符号链接（软链接）文件实际存放的内容，而不是它指向的文件内容

```shell
root@LAPTOP-URAHONEY:/home/urahoney# ll test.soft
lrwxrwxrwx 1 root root 19 Feb 15 16:51 test.soft -> /home/urahoney/test/
root@LAPTOP-URAHONEY:/home/urahoney# readlink test.soft
/home/urahoney/test
root@LAPTOP-URAHONEY:/home/urahoney# cat test.soft
cat: test.soft: Is a directory
```

从这个命令中也可以看出，软链接文件中实际存放的内容就是源文件的路径（可能是绝对路径，也可能是相对路径，取决于ln -s命令是如何执行的），故软链接文件的大小就是原文件的路径字符串的长度（上述的例子ls -l命令中目录最后面的/没有算在内，可能是ls -l命令自己加上去的，实际存放的内容应该参考readlink的内容）



### env

查看当前Shell终端的所有环境变量

```shell
# env
```



### kill

```shell
# kill -n <pid>
```

给进程pid发送信号，信号值为n

```shell
# kill -l
```

查看系统支持的所有信号
