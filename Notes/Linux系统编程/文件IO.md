## 文件IO

* #### open：

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

##### 需要注意：

* flags中必须恰好包含O_RDONLY、O_WRONLY、O_RDWR中的其中一个（读写方式），其他参数可以额外添加

* 1、4原型中，若参数flags中包含O_CREAT，会分配一个堆栈中的值作为新创建文件mode参数的值（随机性），为避免这个情况，应该使用2、3或5原型
* 2、5原型中，若参数flag中不包含O_CREAT，参数mode即使设置也是无效的
* 文件的实际访问权限为mode & ~umask

##### 常见报错

* 参数flags中不包含O_CREAT时打开不存在的文件（No such file or directory）
* 参数flags中包含O_WRONLY或O_RDWR时打开目录文件（Is a directory）
* 打开文件选择的读写方式不满足文件本身权限（）

