### strace

用于查看程序运行时调用的系统调用，用法如下：

```shell
# strace ./a.out
```

上述命令运行a.out并打印运行过程中调用的系统调用



### ulimit

用于查看系统各个属性的阈值，推荐使用如下用法：

```shell
# ulimit -a
```

更多细节通过--help查看

可以设置（慢慢补充）：

* 进程最大的打开文件数
* 管道文件大小
* coredump最大文件大小
* 进程最大栈大小



### umask

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



### 文件特殊权限

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

```shell
# kill -n %<job_id>
```

给任务job_id发送信号，信号值为n



### mkfifo

``` shell
# mkfifo <fifo name>
```

创建命名管道



### 前台与后台命令

```shell
# ./start.sh &
```

在命令最后加上&表示使job在后台运行（如果不加表示在当前终端的前台运行）

```shell
# jobs
```

查看当前终端下的所有后台job，加-l参数可以查看详细信息

```shell
# fg
```

将当前终端下的一个后台job调到前台运行，若要指定job调到前台运行，则

```shell
# fg %<job_id>
```

job_id使用jobs命令查看

```shell
# bg
```

将当前终端下的一个暂停的后台job继续在后台执行，若要指定job调到前台运行，则

```shell
# bg %<job_id>
```

**ctrl + c会向当前终端的前台job发送SIGINT信号，终止该job中所有进程执行**

**ctrl + z会向当前终端的前台job发送SIGTSTP信号，暂停该job中所有进程执行，该job被移到后台**

**ctrl + \会向当前终端的前台job发送SIGQUIT信号，终止该job中所有进程并核心转储**

> 将正在运行的前台运行的命令移到后台运行的方式为：
>
> * 先执行ctrl + z使前台进程暂停运行
> * 再执行bg %<job_id>命令使其在后台继续运行

**ctrl + d会向当前终端输入EOF标志，表示表示输入结束，一般用于结束输入内容的场合，和信号发送无关（并不是结束进程运行）**



### time

```shell
# time ./a.out
```

执行程序并统计运行时间，包括三种时间

```
real	0m1.001s
user	0m0.047s
sys		0m0.169s
```

real表示程序实际运行时间，user表示程序在用户态的运行时间，sys表示程序在内核态的运行时间，real = user + sys + 其他时间

其他时间包括就绪态等待CPU的时间、阻塞态等待资源的时间等



### ps命令进程状态

取自man

```shell
PROCESS STATE CODES
       Here are the different values that the s, stat and state output specifiers (header "STAT" or "S") will display to describe the state of a process:

               D    uninterruptible sleep (usually IO)
               I    Idle kernel thread
               R    running or runnable (on run queue)
               S    interruptible sleep (waiting for an event to complete)
               T    stopped by job control signal
               t    stopped by debugger during the tracing
               W    paging (not valid since the 2.6.xx kernel)
               X    dead (should never be seen)
               Z    defunct ("zombie") process, terminated but not reaped by its parent

       For BSD formats and when the stat keyword is used, additional characters may be displayed:

               <    high-priority (not nice to other users)
               N    low-priority (nice to other users)
               L    has pages locked into memory (for real-time and custom IO)
               s    is a session leader
               l    is multi-threaded (using CLONE_THREAD, like NPTL pthreads do)
               +    is in the foreground process group
```



### nc

```shell
# nc <IP> <PORT>
```

像对应IP和PORT的服务端发起TCP连接，可以充当客户端使用
