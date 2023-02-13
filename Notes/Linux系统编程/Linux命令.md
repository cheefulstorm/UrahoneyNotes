#### strace

用于查看程序运行时调用的系统调用，用法如下：

```shell
#strace ./a.out
```

上述命令运行a.out并打印运行过程中调用的系统调用



#### ulimit

用于查看系统各个属性的阈值，推荐使用如下用法：

```shell
#ulimit -a
```

更多细节通过--help查看



#### umask

```shell
#umask
```

查看文件默认权限的反掩码，root用户默认是0022，普通用户默认是 0002（4位8进制数组成，其中第 1 个数代表的是文件所具有的特殊权限SetUID、SetGID、Sticky BIT）

文件（或目录）的初始权限 = 文件（或目录）的最大默认权限 - umask反掩码 （或者是& ~）

在 Linux 系统中，文件和目录的最大默认权限是不一样的：

- 对文件来讲，其可拥有的最大默认权限是 0666，即 rw-rw-rw-。也就是说，使用文件的任何用户都没有执行（x）权限。原因很简单，执行权限是文件的最高权限，赋予时绝对要慎重，因此绝不能在新建文件的时候就默认赋予，只能通过用户手工赋予。
- 对目录来讲，其可拥有的最大默认权限是 0777，即 rwxrwxrwx。

同样，使用如下可以临时修改反掩码

```shell
#umask 0011
```

永久修改需要修改/etc/profile文件，细节自行查询



#### 文件特殊权限

详情自行查询

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

