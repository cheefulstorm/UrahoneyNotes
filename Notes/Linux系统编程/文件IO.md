## 文件IO

### open

```c
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>

   int open(const char *pathname, int flags);  //1
   int open(const char *pathname, int flags, mode_t mode);  //2

   int creat(const char *pathname, mode_t mode); //3

   int openat(int dirfd, const char *pathname, int flags); //4
   int openat(int dirfd, const char *pathname, int flags, mode_t mode); //5
```

##### 需要注意

* flags中必须恰好包含O_RDONLY、O_WRONLY、O_RDWR中的其中一个（读写方式），其他参数可以额外添加，比较常见的如O_APPEND追加写、O_TRUNC截断写、O_NONBLOCK

* 1、4原型中，若参数flags中包含O_CREAT，会分配一个堆栈中的值作为新创建文件mode参数的值（随机性），为避免这个情况，应该使用2、3或5原型
* 2、5原型中，若参数flag中不包含O_CREAT，参数mode即使设置也是无效的
* 创建新的文件的实际访问权限为mode & ~umask
* openat函数和open函数区别在于：若参数pathname是绝对路径，则参数dirfd无效，两类函数等价；若是相对路径，open函数的当前目录.是运行程序所在目录，而openat函数的当前目录.是dirfd所指向的目录

##### 常见报错

* 参数flags中不包含O_CREAT时打开不存在的文件（No such file or directory）

* 参数flags中包含O_WRONLY或O_RDWR时打开目录文件（Is a directory）

* 打开文件选择的读写方式不满足文件本身权限（Permission denied）

  

### read和write

```c
   #include <unistd.h>

   ssize_t read(int fd, void *buf, size_t count);
   ssize_t write(int fd, const void *buf, size_t count);
```

##### 需要注意

* read的返回值表示读出的字节数，读普通文件时若返回值为0表示文件结束，若为-1表示存在异常（根据errno查询）；如果读取的文件设置了O_NONBLOCK，即非阻塞属性，若read返回-1，需要检查errno，若为EAGAIN或者为EWOULDBLOCK，应该需要再次尝试读取而不能直接判断为读取失败。一般来说，普通文件（包括目录文件）不涉及阻塞或非阻塞属性，设备文件、管道文件、网络文件默认是阻塞属性。
* write的返回值表示写入的字节数，写普通文件时若返回值和入参count（需要写入的字节数）不相等表示有异常错误



### 系统调用与C库函数在IO上的对比分析

实现简单的cp命令操作，有两种思路：

* 通过系统调用read和write实现，每次从源文件中读1B并写入目标文件
* 通过C库函数fgetc和fputc实现（用fread和fwrite也可以），每次从源文件中读1B并写入目标文件

哪个运行速度更快？结果是用C库函数更快

##### 原因

虽然C库函数需要调用系统调用，多了一层函数调用环节，但是C库函数额外增加了自己的处理

C库的IO函数定义了自己的IO缓冲区（分为读和写），这个缓冲区位于用户态，大小一般为4096B，这个缓冲区并不受用户管理；同时，系统的内核态也定义了自己的IO缓冲区（分为读和写），大小一般为4096B，这个缓冲区也不受用户管理

如果用系统调用一次传输1B，假设文件大小为1024B，这就意味着每个系统调用函数需要执行1024次，每次将1B的数据传输到内核的IO缓冲区，同时每次都需要就行用户态/内核态的切换，非常耗时

如果用C库函数一次传输1B，假设文件大小为1024B，这就意味着每个C库函数需要执行1024次，传输的1024B数据会先存放在库函数的缓冲区，等到文件传输结束关闭流或者结束程序时，会将这个1024B的数据通过系统调用一并传输到内核的IO缓冲区，也就是说，读和写分别只执行了一次系统调用，显然节省了很多时间

换言之，若每次拷贝4096B的数据，这两种思路区别就不是很大了

##### 结论

* 系统调用属于无缓冲区IO，C库的IO函数属于有缓冲区IO，这个缓冲区指的是用户态的缓冲区

* 内核态的缓冲区一直都是存在的，为了解决软件速度和硬件速度不匹配的问题，通过该缓冲区实现预读入缓输出的功能

* 系统调用运行未必比C库函数快，要具体问题具体分析



### 文件描述符

文件描述符fd是一个int类型的数字，用于指代进程打开的某个文件。

每个进程在自己的内核空间中有一个进程控制块PCB，c语言中是通过结构体实现的，该结构体中有一个指向文件描述符表的指针，文件描述符表是一个位于内核空间中的数组，数组元素是指向存放该进程打开的文件信息的结构体的指针，这个结构体就和C库的IO函数中FILE结构体类似。而文件描述符就是文件描述符表作为数组的下标。

系统通过PCB中的文件描述符表的指针作为基地址，根据文件描述符作为数组偏移，找到对应指针打开文件的信息的。

文件描述符表的前三个指针默认指向stdin、stdout和stderr，对应的文件描述符为0、1、2，用宏表示为STDIN_FILENO、STDOUT_FILENO、STDERR_FILENO。(进程默认打开这三个文件，对应文件系统下/dev/tty终端设备)

文件描述符表的默认大小为1024，也就是说，一个进程最多可以打开1024个文件（包括stdin、stdout和stderr在内0 - 1023），当然这个值是可以改的

当进程打开一个新的文件时，系统会为这个文件分配的文件描述符数字尽可能小，假如某进程关闭了stdin文件，又打开了一个其他文件，该文件分配到的文件描述符就是STDIN_FILENO（0）

不过可以通过dup函数来给打开的文件分配特定的文件描述符



### fcntl

       #include <unistd.h>
       #include <fcntl.h>
    
       int fcntl(int fd, int cmd, ... /* arg */ );

每个打开文件的属性是通过int类型的数据来表示的，该int类型的某些二进制位代表了对应的属性

cmd参数只有两种选择，F_GETFL和F_SETFL，前者表示获取文件属性，后者表示设置文件属性

若cmd = F_GETFL，函数原型应该为

```c
   int fcntl(int fd, int cmd);
```

即不需要第三个参数，当然若加了第三个参数arg也是无效的，此时返回值即为fd对应的文件属性，若返回值为-1表示存在异常错误

若cmd = F_SETFL，函数原型应该为

```c
   int fcntl(int fd, int cmd, int arg);
```

第三个参数arg为即将设置的文件属性值，若返回值为-1表示存在异常错误，为0表示成功

参考代码

```c
int main() {
    int flag = fcntl(STDIN_FILENO, F_GETFL);
    if (flag == -1) {
        perror("fcntl error");
        exit(1);
    }
    flag |= O_NONBLOCK;
    if (-1 == fcntl(STDIN_FILENO, F_SETFL, flag)) {
        perror("fcntl error");
        exit(1);
    }
}
```

有了这个函数就不需要再重新打开文件时设置文件属性了



### lseek

       #include <sys/types.h>
       #include <unistd.h>
    
       off_t lseek(int fd, off_t offset, int whence);

修改文件fd的IO读/写位置为基地址 + offset

参数whence取值为：

* SEEK_SET：基地址为文件开头
* SEEK_CUR：基地址为当前读/写位置
* SEEK_END：基地址为文件结束

参数offset可以为负数，返回值为新的读/写位置（从文件开头计算），若为-1表示异常错误

##### 需要注意

* 文件的读/写位置是绑定统一的，也就是说，若文件已经从开头读了5B，若要开始写的话，则从第6个字节处开始写（很重要！）

* 通过lseek可以通过返回值来获取文件大小（offset = 0 whence = SEEK_END，不需要+1）或者当前读写位置（offset = 0 whence = SEEK_CUR），读写位置从0开始计数，SEEK_END的基地址是文件最后一个字节的下一个字节
* 通过lseek可以扩展文件大小，即将读写位置移动到SEEK_END之后并进行IO操作写入数据（只移动不写入是不能扩展文件大小的），如下：

```c
    int readFd = open("a", O_RDWR);
    lseek(readFd, 4, SEEK_END);
    char a = '#';
    write(readFd, &a, 1);
    close(readFd);
```

假设文件a的原内容为（末尾是\n）：

```
my name is urahoney

```

执行上述代码后，a的内容为

```
my name is urahoney
#
```

![image-20230213172857917](../../assets/image-20230213172857917.png)

此时#成功写入，但是中间包含了4个NUL字符，该字符在普通终端上不可见，这些字符部分称为文件空洞，有的时候这个字符会显示成^@，文件空洞实际存储的字符为\0

* 文件空洞并不占据物理磁盘空间，直到后续真正写入了数据才会落到磁盘，但是文件空洞占据文件系统可用磁盘空间
* 空洞的存在意味着一个文件名义上的大小（命令看到的）可能比其占用的磁盘存储总量要大（有时大出很多），向文件空洞中写入字节，内核需要为其分配存储单元，即使文件大小不变（命令看到的，文件系统可用磁盘空间不变），但是实际的物理可用磁盘空间也将减少

* 文件系统会扣减程序可用磁盘空间数值大小，做到预留



### truncate

```c
   #include <unistd.h>
   #include <sys/types.h>

   int truncate(const char *path, off_t length);  //1
   int ftruncate(int fd, off_t length);  //2
```

truncate可以实现文件的截断或扩展，若length比文件大小小则实现截断，比文件大小大则实现扩展，扩展原理同lseek原理，会产生文件空洞

原型1和2的区别就在于提供的文件源是由路径还是文件描述符来指明的

若使用原型1，文件必须已经打开，两个原型文件必须均可写
