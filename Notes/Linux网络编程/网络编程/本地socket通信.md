### 背景

socket API原本是为网络通讯设计的，但后来在socket的框架上发展出一种IPC机制，就是UNIX Domain Socket

虽然网络socket也可用于同一台主机的进程间通讯（通过loopback地址127.0.0.1），但是UNIX Domain Socket用于IPC更有效率：不需要经过网络协议栈，不需要打包拆包、计算校验和、维护序号和应答等，只是将应用层数据从一个进程拷贝到另一个进程

这是因为，IPC机制本质上是可靠的通讯，而网络协议是为不可靠的通讯设计的

UNIX Domain Socket也提供面向流和面向数据包两种API接口，类似于TCP和UDP，但是面向消息的UNIX Domain Socket也是可靠的，消息既不会丢失也不会顺序错乱

UNIX Domain Socket是全双工的，API接口语义丰富，相比其它IPC机制有明显的优越性，目前已成为使用最广泛的IPC机制，比如X Window服务器和GUI程序之间就是通过UNIXDomain Socket通讯的



### 本地socket套接字的创建

同样是调用socket函数，不过参数不同

```C
   #include <sys/types.h>
   #include <sys/socket.h>

   int socket(int domain, int type, int protocol);
```

domain：选AF_UNIX或者AF_LOCAL，两者均可

type：和网络通信类似，选SOCK_STREAM或者SOCK_DGRAM，两者都能提供可靠通信服务，因为IPC机制本来就是可靠的

protocol：同样传0



### socket地址的绑定

本地socket通信使用的存放地址信息的结构体是sockaddr_un，结构如下：

```c
    struct sockaddr_un {
        sa_family_t sun_family; 		        /* AF_UNIX */
        char sun_path[UNIX_PATH_MAX]; 		    /* pathname */
    };
```

sun_family：地址族类型，取值为AF_UNIX

sun_path：存放本地socket文件路径的字符串

```c
   #include <sys/types.h>
   #include <sys/socket.h>    

   int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

调用bind绑定socket地址信息时，参数如下：

* sockfd：即socket返回的文件描述符

* addr：将sockaddr_un变量取地址并强制转换成sockaddr指针类型

* addrlen：注意不是sizeof(sockaddr_un)，因为sun_path的空间不需要全部传入，需要计算字符串的实际长度，可以通过宏**offsetof**来计算addrlen的值，如下所示：

  ```c++
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  sockaddr_un addr{};
  // initiate addr
  bind(fd, (sockaddr *)&addr, offsetof(sockaddr_un, sun_path) + strlen(addr.sun_path) + 1);
  ```

> 本地socket绑定的地址信息和网络socket绑定的地址信息是不同的，网络socket绑定的是IP和端口号，而本地socket绑定的是一个文件路径，当绑定成功后，会根据该路径创建一个socket文件用于本地socket通信
>
> 需要注意，本地socket文件是特殊文件，存放在内存中而不存放在外存中
>
> 为了避免bind时创建文件失败，可以在调用bind前调用unlink删除该文件



### 实例

下面是服务端的代码

```c++
int main() {
    char svrPath[] = "/tmp/svr.socket";
    char buf[1024]{};

    int listenFd = usocket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un svrAddr{};
    svrAddr.sun_family = AF_UNIX;
    strncpy(svrAddr.sun_path, svrPath, sizeof(svrPath));

    unlink(svrPath);  // 删除svrPath对应socket文件，防止bind失败
    ubind(listenFd, (const sockaddr *)&svrAddr, offsetof(sockaddr_un, sun_path) + strlen(svrAddr.sun_path));
    
    ulisten(listenFd, 10);

    sockaddr_un cltAddr{};
    socklen_t cltAddrLen{};
    int connectFd = uaccept(listenFd, (sockaddr *)&cltAddr, &cltAddrLen);

    while (true) {
        ssize_t count = uread(connectFd, buf, sizeof(buf));
        if (count == 0) {
            uclose(connectFd);
            exit(EXIT_SUCCESS);
        }
        for (int i = 0; i < count; ++i) {
            buf[i] = toupper(buf[i]);
        }
        uwrite(connectFd, buf, count);
        memset(buf, 0, sizeof(buf));
    }
}
```

> 除了socket和bind处理不一样以外，其他的内容基本不变（这里没有实现多客户端）

下面是客户端的代码

```c++
int main() {
    int connectFd = usocket(AF_UNIX, SOCK_STREAM, 0);

    char cltPath[] = "/tmp/clt.socket";
    sockaddr_un cltAddr{};
    cltAddr.sun_family = AF_UNIX;
    strncpy(cltAddr.sun_path, cltPath, sizeof(cltPath));
    unlink(cltPath);  // 删除cltPath对应socket文件，防止bind失败
    ubind(connectFd, (const sockaddr *)&cltAddr, offsetof(sockaddr_un, sun_path) + strlen(cltAddr.sun_path));  // 客户端也要bind


    char svrPath[] = "/tmp/svr.socket";
    sockaddr_un svrAddr{};
    svrAddr.sun_family = AF_UNIX;
    strncpy(svrAddr.sun_path, svrPath, sizeof(svrPath));
    uconnect(connectFd, (const sockaddr *)&svrAddr, offsetof(sockaddr_un, sun_path) + strlen(svrAddr.sun_path));

    char buf[1024]{};
    while (cin.getline(buf, sizeof(buf))) {
        ssize_t count = strlen(buf);
        if (0 == count) {
            continue;
        }

        uwrite(connectFd, buf, count);
        ssize_t realCount = uread(connectFd, buf, count);
        if (0 == realCount) {
            uclose(connectFd);
            exit(EXIT_SUCCESS);
        }
        cout << buf << endl;
        memset(buf, 0, sizeof(buf));
    }
}
```

> 除了socket不一样以外，需要注意，**客户端也必须调用bind绑定自己的socket文件的地址**，本地socket通信不支持隐式绑定
