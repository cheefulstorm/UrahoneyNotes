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
* **注意mode在传参时是8进制数，也就是说，在写代码的时候，需要在数字前加0表示八进制数，如0777**，漏掉0会有异常，这里的0和umask里面开头的0是一个意思，表示8进制
* 虽然mode_t中包含文件类型信息，但是原型2、3、5不能通过传入指定的mode来指定创建文件的类型，无论传入的mode中对应位表示其他类型的文件，但是创建出来的都是regular file，不过可以指定特殊权限创建，即入参的有效8进制位是低4位，如07777
* 创建新的文件的实际访问权限为mode & ~umask（umask可以通过umask函数来设置，见后续）
* openat函数和open函数区别在于：若参数pathname是绝对路径，则参数dirfd无效，两类函数等价；若是相对路径，open函数的当前目录.是运行程序所在目录，而openat函数的当前目录.是dirfd所指向的目录
* 后续提到的文件IO的函数可能带有at后缀版本，有的并没有，知道at后缀代表的意思就行，如果想使用先查询一下就行

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

* read的返回值表示读出的字节数，读普通文件时若返回值为0表示文件结束，若为-1表示存在异常（根据errno查询）；**如果读取的文件（设备文件、网络文件等）设置了O_NONBLOCK，即非阻塞属性，若read返回-1，需要检查errno，若为EAGAIN或者为EWOULDBLOCK，应该需要再次尝试读取而不能直接判断为读取失败**。一般来说，普通文件（包括目录文件）不涉及阻塞或非阻塞属性，设备文件、管道文件、网络文件默认是阻塞属性。
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

每个进程在自己的内核空间中有一个进程控制块PCB，c语言中是通过结构体task_struct实现的，该结构体中有一个指向结构体files_struct的指针，该结构体中存放了指向文件描述符表的指针fd_array，文件描述符表是一个位于内核空间中的数组，数组元素是指向存放该进程打开的文件信息的结构体file_struct的指针，这个结构体就和C库的IO函数中FILE结构体类似。而文件描述符就是文件描述符表作为数组的下标。

task_struct（PCB） -> files_struct（进程打开文件表） ->file_struct（打开文件）（简记）

系统通过fd_array的值作为基地址，根据文件描述符作为数组偏移，找到对应指针打开文件的信息的。

文件描述符表的前三个指针默认指向stdin、stdout和stderr，对应的文件描述符为0、1、2，用宏表示为STDIN_FILENO、STDOUT_FILENO、STDERR_FILENO。(进程默认打开这三个文件，对应文件系统下/dev/tty终端设备)

文件描述符表的默认大小为1024，也就是说，一个进程最多可以打开1024个文件（包括stdin、stdout和stderr在内0 - 1023），当然这个值是可以改的

当进程打开一个新的文件时，系统会为这个文件分配的文件描述符数字尽可能小，假如某进程关闭了stdin文件，又打开了一个其他文件，该文件分配到的文件描述符就是STDIN_FILENO（0）

不过可以通过dup2函数来给打开的文件分配特定的文件描述符。

可以参考https://blog.csdn.net/wwwlyj123321/article/details/100298377查看具体信息



### fcntl（设置文件的打开属性）

       #include <unistd.h>
       #include <fcntl.h>
    
       int fcntl(int fd, int cmd, ... /* arg */ );

每个文件的打开属性（就是open函数的flags参数）是通过int类型的数据来表示的，该int类型的某些二进制位代表了对应的属性

cmd参数包括F_GETFL和F_SETFL，前者表示获取文件属性，后者表示设置文件属性

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

这个函数很复杂，cmd不同时功能也不同，后续慢慢补充



### umask

```c
   #include <sys/types.h>
   #include <sys/stat.h>

   mode_t umask(mode_t mask);
```

和umask命令功能类似，修改运行进程的文件访问权限mask掩码，注意，只有低9位uuugggooo可以修改，返回修改前的文件访问权限mask掩码（入参和返回值都是掩码而不是反掩码）



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

* **文件的读/写位置是绑定统一的**，也就是说，若文件已经从开头读了5B，若要开始写的话，则从第6个字节处开始写（很重要！）

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

此时#成功写入，但是中间包含了4个NUL字符，该字符在普通终端上不可见，这些字符部分称为文件空洞，这个字符在vim下会显示成^@，而若用程序读取则会读取到\0，实际上在磁盘空间中不存在

* 文件空洞并不占据物理磁盘空间，直到后续真正写入了数据才会落到磁盘，但是文件空洞占据文件系统可用磁盘空间
* 空洞的存在意味着一个文件名义上的大小（命令看到的）可能比其占用的磁盘存储总量要大（有时大出很多），向文件空洞中写入字节，内核需要为其分配存储单元，即使文件大小不变（命令看到的，文件系统可用磁盘空间不变），但是实际的物理可用磁盘空间也将减少

* 文件系统会扣减程序可用磁盘空间数值大小，做到预留



### truncate和ftruncate

```c
   #include <unistd.h>
   #include <sys/types.h>

   int truncate(const char *path, off_t length);  //1
   int ftruncate(int fd, off_t length);  //2
```

truncate可以实现文件的截断或扩展，若length比文件大小小则实现截断，比文件大小大则实现扩展，和lseek类似产生文件空洞，但是truncate实现文件扩展时不需要进行写操作，会自动再文件末尾补上\0

因此，这两种写法是等价的（假设文件fd的大小一开始为0，**打开时没有加O_APPEND参数，加了会有问题？**）

```c
	lseek(fd, 4095, SEEK_END);
	write(fd, "\0", 1);
```

```c
	ftruncate(fd, 4096);
```

原型1和2的区别就在于提供的文件源是由路径还是文件描述符来指明的

使用时，两个原型文件必须均可写，使用原型1可以不用先调用open打开文件，原型2需要



### stat、fstat和lstat

```c
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <unistd.h>

   int stat(const char *pathname, struct stat *statbuf);
   int fstat(int fd, struct stat *statbuf);
   int lstat(const char *pathname, struct stat *statbuf);
```

statbuf作为入参，用于获取文件的属性，返回0表示成功，返回-1表示异常错误

你会发现lstat函数的原型和stat函数的原型相同，他们的区别在于：

* stat函数的pathname参数中若是软链接文件，那么stat会找到该软链接文件所指的原文件（会传递查找），再去获取该文件的属性信息
* lstat函数的pathname参数中若是软链接文件，会直接获取该软链接文件的属性信息。

称stat函数会穿透符号链接，而lstat函数不会

同理，如ls命令不会穿透符号链接，cat命令会穿透符号链接

stat与fstat的区别就在于提供的文件源是由路径还是文件描述符来指明的，使用fstat是文件需要打开

结构体stat的格式如下

```c
       struct stat {
           dev_t     st_dev;         /* ID of device containing file */
           ino_t     st_ino;         /* Inode number */
           mode_t    st_mode;        /* File type and mode */
           nlink_t   st_nlink;       /* Number of hard links */
           uid_t     st_uid;         /* User ID of owner */
           gid_t     st_gid;         /* Group ID of owner */
           dev_t     st_rdev;        /* Device ID (if special file) */
           off_t     st_size;        /* Total size, in bytes */
           blksize_t st_blksize;     /* Block size for filesystem I/O */
           blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */

           /* Since Linux 2.6, the kernel supports nanosecond
              precision for the following timestamp fields.
              For the details before Linux 2.6, see NOTES. */

           struct timespec st_atim;  /* Time of last access */
           struct timespec st_mtim;  /* Time of last modification */
           struct timespec st_ctim;  /* Time of last status change */

       #define st_atime st_atim.tv_sec      /* Backward compatibility */
       #define st_mtime st_mtim.tv_sec
       #define st_ctime st_ctim.tv_sec
       };
```

更多细节参见APUE p.74 或 https://blog.csdn.net/m0_60663280/article/details/126884780

值得一提的是mode_t，它是一个无符号32位整数，里面的信息包含了文件的类型、文件的权限（包括特殊权限），其中低16位是真正有效的，后面只讨论低16位。

在低16位中，比特分布如下，ttttsssuuugggooo，其中u、g、o分别是user、group和other所属的文件权限，3位s是特殊权限位（3位从高到低分别表示SetUID、SetGID、Sticky BIT），4位t用于表示文件类型，可以表示16种文件类型，对应的掩码是宏S_IFMT，但是目前linux只有7种文件类型

文件类型的相关宏使用如下：

```c++
	struct stat st;
	stat(argv[1], &st);
	if (S_ISREG(st.st_mode))       // is it a regular file?
        cout << 1 << endl;         // 数字无意义
	else if (S_ISDIR(st.st_mode))  // is it a directory?
        cout << 2 << endl;
	else if (S_ISCHR(st.st_mode))  // is it a character device?
        cout << 3 << endl;
	else if (S_ISBLK(st.st_mode))  // is it a block device?
        cout << 4 << endl;
	else if (S_ISFIFO(st.st_mode))  // is it a FIFO (named pipe)?
        cout << 5 << endl;
	else if (S_ISLNK(st.st_mode))   // is it a symbolic link?
        cout << 6 << endl;
	else if (S_ISSOCK(st.st_mode))  // is it a socket?
        cout << 7 << endl;
	else
        cout << "error" << endl;
```

还有一种使用方法（使用S_IFMT宏）：

```c
    struct stat st;
	stat(argv[1], &st);
	switch (st.st_mode & S_IFMT) {
        case S_IFBLK:  printf("block device\n");            break;
        case S_IFCHR:  printf("character device\n");        break;
        case S_IFDIR:  printf("directory\n");               break;
        case S_IFIFO:  printf("FIFO/pipe\n");               break;
        case S_IFLNK:  printf("symlink\n");                 break;
        case S_IFREG:  printf("regular file\n");            break;
        case S_IFSOCK: printf("socket\n");                  break;
        default:       printf("unknown?\n");                break;
    }
```



### chmod和fchmod

```c
   #include <sys/stat.h>

   int chmod(const char *pathname, mode_t mode);  // 1
   int fchmod(int fd, mode_t mode);  // 2
```

修改文件的访问权限，实际能够修改的部分为mode_t的低12位，即sssuuugggooo，八进制表示0xxxx（x:0-7）

原型1和2的区别就在于提供的文件源是由路径还是文件描述符来指明的

若使用原型2，文件必须已经打开，两个原型文件必须均可写

成功访问0，异常错误返回-1



### link和unlink

```c
   #include <unistd.h>

   int link(const char *oldpath, const char *newpath);
   int unlink(const char *pathname);
```

link函数效果和命令link的效果类似，用于创建硬链接，如

```shell
#link func.txt func.txt.hard
```

成功访问0，异常错误返回-1

unlink函数用于删除一个文件在所在目录的对应目录项，使该文件对应的硬链接个数-1

##### 需要注意

* 若该文件对应的硬链接个数在-1之后为0，该文件即将被删除，所以说unlink具备删除文件的功能

* 操作系统不会立即释放该文件所占的磁盘空间，它会根据内部的调度算法择机释放，一般要**等打开该文件的所有进程关闭该文件后**才准备释放删除
* 若一个打开的文件调用了unlink后硬链接个数变为0，随后又对该文件进行一系列的IO操作如write等，这些操作并不会失败，因为该文件即将被删除但是还没有被删除，删除时机见上述，实际上，在这个情况下进行的write操作只改变了内核缓冲区的内容，并没有修改磁盘内容



### Linux隐式回收

进程运行结束时，会自动释放该进程申请的内核空间，刷新库缓冲区，并关闭所有打开的文件，执行进程退出函数（比较简单待补充）



### readlink

```c
   #include <unistd.h>

   ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
```

readlink函数和readlink命令的功能差不多，只不过获取的路径字符串通过buf返回而已，**不会自动在字符串最后补\0**，返回路径的长度，不包括\0，失败返回-1，也就是说，如果buf缓冲区不够大，不仅仅是内容截断这么简单，甚至会发生ub行为



### rename

```c
   #include <stdio.h>

   int rename(const char *oldpath, const char *newpath);
```

和mv命令功能类似，具备移动文件或重命名的功能，成功返回0，异常错误返回-1



### getcwd

```c
   #include <unistd.h>
   
   char *getcwd(char *buf, size_t size);
```

和pwd命令功能类似，通过buf返回运行进程的当前工作目录，即该进程代码中涉及到的.目录，**会在路径字符串后面补\0，**如果size大小不够大，不会产生截断，函数执行失败，返回NULL，并且buf的内容不会发生改变，函数执行成功时返回buf指针地址



### chdir

```c
   #include <unistd.h>

   int chdir(const char *path);   // 1
   int fchdir(int fd);  // 2
```

和cd命令功能类似，改变运行进程的当前工作目录，成功返回0，异常错误返回-1，原型1和2的区别在于一个是提供目录的路径，一个是提供目录文件的文件描述符，即需要注意，原型2中的参数fd的文件类型是目录文件



### opendir和closedir

```c
   #include <sys/types.h>
   #include <dirent.h>

   DIR *opendir(const char *name);
   DIR *fdopendir(int fd);
   int closedir(DIR *dirp);
```

opendir、fdopendir：根据目录路径名或打开的目录文件的文件描述符来打开目录流，返回DIR结构体的指针，类似FILE结构体一样，描述目录文件的信息，该指针用于后续对目录进行操作，若异常失败返回NULL

closedir：通过DIR结构体的指针来关闭目录流（包括该目录文件），成功返回0，异常失败返回-1



### mkdir

```c
   #include <sys/stat.h>
   #include <sys/types.h>

   int mkdir(const char *pathname, mode_t mode);
```

创建一个目录，参数以及返回值的含义和creat函数基本相同



### readdir

```c
   #include <dirent.h>

   struct dirent *readdir(DIR *dirp);
```

根据目录指针dirp获取当前目录流的一条目录项，返回目录项dirent指针，若已经达到目录流的结尾（即目录项已经全部读出）或者出错，会返回NULL指针，前者errno不变，后者会设置errno，**故当readdir返回NULL时，要检查errno是否异常**，不能简单判断出错还是目录已经读完

**readdir获取的目录项的顺序是不透明的**

dirent结构体的内容如下：

```c
       struct dirent {
           ino_t          d_ino;       /* Inode number */
           off_t          d_off;       /* Not an offset; see below */
           unsigned short d_reclen;    /* Length of this record */
           unsigned char  d_type;      /* Type of file; not supported
                                          by all filesystem types */
           char           d_name[256]; /* Null-terminated filename */
       };
```

在编程时，一般只需要关注d_name字段（除了d_ino和d_name其他字段文件系统不一定支持），即文件名字段，看下面的demo：

```c++
int main(int argc, char *argv[]) {
    DIR *dirp = opendir(argv[1]);
    if (nullptr == dirp) {
        perror("open dir failed");
        exit(EXIT_FAILURE);
    }
    dirent *dentp = nullptr;
    while (nullptr != (dentp = readdir(dirp))) {
        cout << dentp->d_name << "\t";
    }
    if (0 != errno) {
        perror("read dir error");
        exit(EXIT_FAILURE);
    }
    cout << endl;
    closedir(dirp);
    return 0;
}
```

d_type类型字段也可以关注，可以在不通过stat函数的前提下获取文件类型，对应宏如下：

    DT_BLK      This is a block device.
    
    DT_CHR      This is a character device.
    
    DT_DIR      This is a directory.
    
    DT_FIFO     This is a named pipe (FIFO).
    
    DT_LNK      This is a symbolic link.
    
    DT_REG      This is a regular file.
    
    DT_SOCK     This is a UNIX domain socket.
    
    DT_UNKNOWN  The file type could not be determined.

若文件系统不支持该字段，会返回DT_UNKNOWN

##### 需要注意

* 虽然glibc的头文件中指明了d_name数组的大小为256，但是编程实现时不要依赖该大小，POSIX.1标准并未规定d_name数组的大小，应该使用strlen来获取d_name字符串的实际大小，也不能将d_name数组作为左值来修改文件名，有的文件系统实现的头文件中是用d_name[0]可变长数组来实现的
* 不要通过sizeof(struct dirent)来获取目录项的大小，d_reclen字段描述的也并非是目录项的大小，这两者之间也是无关的，**总之，不要依赖着两个值来进行编程**

### rewinddir

```c
   #include <sys/types.h>
   #include <dirent.h>

   void rewinddir(DIR *dirp);
```

重置目录流dirp到开始的地方，这样可以重新从头读取对应目录的目录项



### telldir和seekdir

```c
   #include <dirent.h>

   long telldir(DIR *dirp);
   void seekdir(DIR *dirp, long loc);
```

tellldir返回目录流dirp的当前读写位置，若出现异常错误返回-1

seekdir设置目录流dirp的当前读写位置为loc

##### 需要注意

telldir函数的返回值和当前调用readdir获取的目录项dirent中的d_off字段值相同（新的POSIX标准将off_t改为了long类型，但是本机上off_t和long是相同的），同时注意，**不要将该值当做目录流中目录项的偏移量**，以前老版本的glibc实现是这样的，现代版本的glibc是通过哈希表或树结构实现目录，只需要将其当做一个不透明的值就行



### scandir

```c
   #include <dirent.h>

   int scandir(const char *dirp, struct dirent ***namelist,
          int (*filter)(const struct dirent *),
          int (*compar)(const struct dirent **, const struct dirent **));
```

https://blog.csdn.net/weixin_44498318/article/details/116431854

待补充



### dup和dup2

\# dup是duplicate的缩写

```c
   #include <unistd.h>

   int dup(int oldfd);
   int dup2(int oldfd, int newfd);
```

dup：为旧的文件描述符oldfd产生一个新的文件描述符副本（这两个文件描述符指向相同的文件）并返回其值，若异常错误返回-1

dup2：为旧的文件描述符oldfd产生一个新的文件描述符副本newfd（这两个文件描述符指向相同的文件）并返回newfd，异常错误返回-1

##### 需要注意

* dup产生的新的文件描述符副本遵循尽可能小的原则分配未使用的值
* 在使用dup2时，若newfd对应的文件已经被打开（即该数字已经被其他打开文件占用），**会先关闭该文件**释放newfd的值
* 在使用dup2时，若oldfd是无效文件描述符，函数调用失败返回-1，若newfd对应的文件已经被打开则不会被关闭
* 在使用dup2时，若oldfd是有效文件描述符且oldfd == newfd，函数什么都不做返回newfd
* dup和dup2本质是将文件描述符表fd_array[oldfd]中的指针拷贝给fd_array[newfd]，从而使多个fd指向同一个进程打开的文件
* 如果两个文件描述符fd指向同一个文件，通过其中一个fd关闭对应的文件，并不会直接关闭该文件，只会使被关闭的fd无效，而另一个fd还可以操作该文件，原理类似硬链接（引用计数），**直到该进程没有fd指向该文件时该文件才被进程彻底关闭**

下面有一个重定向标准输出stdout的demo：

```c++
int main(int argc, char *argv[]) {
	int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (-1 == fd) {
        perror("open error");
        exit(EXIT_FAILURE);
    }
    int newFd = dup2(fd, STDOUT_FILENO);   // 关闭stdout并使STDOUT_FILENO指向argv[1]文件
    if (STDOUT_FILENO != newFd) {
        perror("dup2 error");
        exit(EXIT_FAILURE);
    }
    cout << argv[2] << endl;   // 标准输出已经被重定向
    close(fd);
    close(newFd);
    exit(EXIT_SUCCESS);
}
```

测试程序如下：

```shell
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# ./main test.txt hello
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug# cat test.txt 
test.txt
root@LAPTOP-URAHONEY:/home/urahoney/Study/cmake-build-debug#
```

**dup2对于重定位的用法简单记忆为：将newfd重定向到oldfd**



### fcntl（产生文件描述符的副本）

```c
   #include <unistd.h>
   #include <fcntl.h>

   int fcntl(int fd, int cmd, ... /* arg */ );
```

该功能和dup类函数功能类似，cmd参数是F_DUPFD，若不适用arg参数，功能和dup类似，若使用arg参数，功能和dup2类似，异常错误返回-1，成功返回新的文件描述符副本newfd的值

```c
	newfd = fcntl(fd, F_DUPFD);  // 1
	newfd = fcntl(fd, F_DUPFD, 0);  // 2
	newfd = dup(fd);
```

上面三种写法等价（原型2可以看下面的解释）

```c
	newfd = fcntl(fd, F_DUPFD, newfd);  // 3
	newfd = dup2(fd, newfd);  // 4
```

上面两种写法功能基本相同，但是存在区别在于：

* 原型3中若newfd已经被进程打开的文件占用，不会关闭该文件释放newfd的值，会重新指定一个**未被使用的并且大于newfd的且尽可能小的值**作为新的文件描述符的值，换言之，**新的文件描述符的值一定是大于等于newfd的**，这个原则和open、dup以及原型1的文件描述符分配原则有些不同

* 原型4中则会关闭占用newfd的文件释放newfd的值，见dup2的用法



### pathconf和fpathconf

```c
   #include <unistd.h>

   long fpathconf(int fd, int name);  // 1
   long pathconf(const char *path, int name);  // 2
```

查看文件的配置参数值，分别提供文件描述符或者文件路径作为参数1，另一个参数name表示配置参数类型，取值参考APUE p.35或man pathconf，返回对应文件的配置参数值

