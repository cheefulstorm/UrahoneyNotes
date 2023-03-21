### IP地址相关函数

下面涉及到的IP或者端口的数据类型，若是整型，则是二进制表示，若是字符串，则是点分十进制表示

```c
   #include <arpa/inet.h>

   uint32_t htonl(uint32_t hostlong);
   uint16_t htons(uint16_t hostshort);
   uint32_t ntohl(uint32_t netlong);
   uint16_t ntohs(uint16_t netshort);
```

htonl：将4B数据由主机字节序转换网络字节序（可用于转换IPv4地址使用）

htons：将2B数据由主机字节序转换网络字节序（可用于转换端口号使用）

ntohl：将4B数据由网络字节序转换主机字节序

ntohs：将2B数据由网络字节序转换主机字节序

```c
   #include <arpa/inet.h>

    typedef uint32_t in_addr_t;
    struct in_addr {
        in_addr_t s_addr;
    };

   int inet_aton(const char *cp, struct in_addr *inp);
   char *inet_ntoa(struct in_addr in);
```

inet_aton：将IPv4地址进行格式转换，由**字符串(cp)** 转换成**网络字节序的数值(inp)**，转换成功返回1，异常错误返回0（不设置errno）

inet_ntoa：将IPv4地址进行格式转换，由**网络字节序数值(in)** 转换成**字符串(返回值)**，转换成功返回字符串首地址，异常错误返回NULL

> 上述两个函数异常错误不设置errno
>
> inet_ntoa返回的指针指向函数内部的字符数组静态变量，该函数不可重入，但是线程安全
>
> 对于in_addr结构体只用一个成员而为什么保留的原因，因为早期in_addr结构体中有不止一个成员，随着版本更迭，其他成员都被移除，而为了兼容性，结构体仍然保留
>
> **上述两个函数不推荐使用（供阅读代码了解，编程不要使用）**

```c
   #include <arpa/inet.h>

   typedef uint32_t in_addr_t;

   in_addr_t inet_addr(const char *cp);
   in_addr_t inet_network(const char *cp);
```

inet_addr：将IPv4地址进行格式转换，由**字符串(cp)** 转换成**网络字节序的数值(返回值)**，转换成功返回结果，异常错误返回-1

inet_network：将IPv4地址进行格式转换，由**字符串(cp)** 转换成**主机字节序的数值(返回值)**，转换成功返回结果，异常错误返回-1

> 上述两个函数异常错误不设置errno
>
> 注意别把两个函数的功能搞反了
>
> 上述两个函数返回的-1可以表示有效IP地址255.255.255.255，因此当返回-1时可能并没有发生错误，**上述两个函数不推荐使用（供阅读代码了解，编程不要使用）**

```c
   #include <arpa/inet.h>

   int inet_pton(int af, const char *src, void *dst);
   const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
```

inet_pton：将IPv4地址进行格式转换，由**字符串(src)** 转换成**网络字节序的数值(dst)**，转换成功返回1，转换失败返回0，异常错误返回-1

inet_ntop：将IPv4地址进行格式转换，由**网络字节序的数值(src)** 转换成**字符串(dst)**，转换成功返回字符串首地址，异常错误返回NULL

> p指代presentation（表示），n指代numeric（数值）
>
> 上述两个函数异常错误会设置errno

af参数的定义如下：

| af参数   | 含义       |
| -------- | ---------- |
| AF_INET  | IPv4地址族 |
| AF_INET6 | IPv6地址族 |

由于上述两个函数可以支持两种地址族，故inet_pton的dst参数以及inet_ntop的src参数均是void*类型

* 若af为AF_INET，指针指向的内存大小应该为4，常用的类型如in_addr_t、in_addr、uint32_t
* 若af为AF_INET，指针指向的内存大小应该为16，如何实现待补充

inet_ntop的size参数用于指定出参dst缓冲区的大小



### sockaddr结构体

```c
    struct sockaddr {
        sa_family_t sa_family;  // 地址族类型（兼容其他各种地址族结构体类型）
        char sa_data[14];       // 其他数据（不兼容其他各种地址族结构体类型）
    };
```

描述了类UNIX操作系统进行socket通信时存放的地址信息格式

> sockaddr结构体是伴随着UNIX网络编程接口的出现而出现的，当时IPv4协议都还没有出现，那个时候使用这个结构体进行编程，直到其他各种类型的地址族协议出现，为了使老的接口原型兼容新的地址族协议（接口类型一旦确定不能随意更改的），sockaddr结构体逐渐失去了本身的功能，**充当通用类型（类似void *）的作用**，不能直接使用在网络编程接口中，而其他各种地址族类型的地址需要使用网络编程接口，就**需要将它的指针类型强制转换成sockaddr *类型才可以使用**

sockaddr的参数sa_family指明了地址族类型，常见的类型如下所示：

| sa_family |     含义     | 对应结构体类型 |
| :-------: | :----------: | :------------: |
|  AF_INET  |  IPv4地址族  | socketaddr_in  |
| AF_INET6  |  IPv6地址族  | socketaddr_in6 |
|  AF_UNIX  | UNIX域地址族 | socketaddr_un  |
| AF_UNSPEC | 未指定地址族 |       -        |

下面是各种常见地址族的地址类型的字节分布（可以看出大小可能是不同的）：

![img](../../assets/clip_image001.png)

sockaddr_in结构体的格式如下

```c
    /* Internet address. */
    struct in_addr {
        uint32_t       s_addr;     /* address in network byte order */
    };

	struct sockaddr_in {
        sa_family_t    sin_family; /* address family: AF_INET */
        in_port_t      sin_port;   /* port in network byte order */
        struct in_addr sin_addr;   /* internet address */
    };
```

参数包括：

* sin_family，地址族类型，取值为AF_INET
* sin_port，端口号，注意是网络字节序
* sin_addr.s_addr，IPv4地址，注意是网络字节序



### 常见的IP地址宏

| 地址宏           | 对应地址        | 解释                                                         |
| ---------------- | --------------- | ------------------------------------------------------------ |
| INADDR_ANY       | 0.0.0.0         | 任意地址，常用于服务端监听socket时，指定socket的IP地址为服务器拥有的所有网卡的IP地址 |
| INADDR_BROADCAST | 255.255.255.255 | 本地广播地址                                                 |
| INADDR_LOOPBACK  | 127.0.0.1       | 环回地址                                                     |
| INADDR_NONE      | 255.255.255.255 | 错误地址，inet_addr和inet_network函数调用错误时会返回        |

> 上述IP地址宏均为主机字节序类型



### socket通信流程框架

![img](../../assets/clip_image002.jpg)



### socket

```c
   #include <sys/types.h>
   #include <sys/socket.h>

   int socket(int domain, int type, int protocol);
```

创建一个socket并返回文件描述符，异常错误返回-1

参数如下：

* domain：指定socket通信时使用的地址族，参见结构体sockaddr中参数sa_family的定义

* type：指定socket类型，常见的取值如下：

  |     type      |              含义               |
  | :-----------: | :-----------------------------: |
  | SOCKET_STREAM | 基于字节流的socket，用于TCP通信 |
  | SOCKET_DGRAM  | 基于数据报的socket，用于UDP通信 |

* protocol：指定代表通信协议的编号，一般设置为0，由内核根据参数domain和type来选择代表协议的类型

> socket函数创建的套接字的默认类型是主动类型，称为主动套接字，可以主动向其他主机发出连接请求



### bind

```c
   #include <sys/types.h>          /* See NOTES */
   #include <sys/socket.h>

   int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

将socket套接字sockfd绑定地址信息addr，addrlen指定了地址信息addr的空间长度，调用成功返回0，异常错误返回-1



### listen

```c
   #include <sys/types.h>          /* See NOTES */
   #include <sys/socket.h>

   int listen(int sockfd, int backlog);
```

将socket套接字sockfd的模式设置为被动模式，并设置连接请求队列的最大长度为backlog，调用成功返回0，异常错误返回-1

> 调用listen函数后，sockfd的模式就从主动模式转换成被动模式，称为被动套接字，只能被动接受连接请求而不能发起连接请求
>
> **调用listen函数后，套接字的状态就由CLOSE转换成LISTEN**
>
> 若socket套接字未绑定地址信息就开始调用listen，内核会选择本机的IP地址和系统分配的端口号作为地址信息，称为隐式绑定
>
> 连接请求队列中存放的是尚未接受的socket连接请求，换言之，**backlog指定了尚未接受的连接请求的最大数量，而非与服务端建立连接的客户端最大数量**，若在短时间内接收到的连接请求数超过backlog，则可能会丢弃超出的连接请求，客户端的connect函数可能会收到ECONNREFUSED错误
>
> 实际上，连接请求队列的最大长度为min(backlog, /proc/sys/net/core/somaxconn)，内核参数/proc/sys/net/core/somaxconn参数用于限制连接请求队列的最大值
>
> 后续内容中会提到，这里面的连接请求队列即全连接队列
>
> 本函数不会阻塞，当调用完该函数后，**由内核开始负责进行监听，而用户程序可以继续执行**



### accept

```c
   #include <sys/types.h>
   #include <sys/socket.h>

   int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

从socket套接字sockfd对应的连接请求队列中取出一个socket连接请求，表示接受该连接请求，并通过出参addr返回请求方socket绑定的地址信息，同时通过出参addrlen返回了该地址信息的空间长度，**调用成功则返回用于和该请求方进行通信的socket文件描述符，异常错误返回-1，若连接请求队列为空，则函数阻塞，直到连接请求队列中加入新的socket连接请求**

> addrlen虽然是出参，但是对初值也有要求，初值必须是addr指向类型的大小



### connect

```c
   #include <sys/types.h>
   #include <sys/socket.h>

   int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

通过socket套接字sockfd向服务端建立连接，服务端监听socket的地址信息由addr指定，其长度由addrlen，成功建立连接返回0，异常错误返回-1

> 若socket套接字未绑定地址信息就开始调用connect，内核会选择本机的IP地址和系统分配的端口号作为地址信息，称为隐式绑定
>
> 客户端一般会利用这种特性，省去调用bind的环节，因为客户端的端口号不需要指定，但是服务端的端口号是确定的，不能跳过绑定地址信息的操作



### 三次握手原理

三次握手是由客户端调用connect函数发起的，具体的流程图如下所示：

<img src="../../assets/79146dfbdb9b411091a8358739de021f.png" alt="79146dfbdb9b411091a8358739de021f" style="zoom:67%;" />



当服务端调用listen后，**服务端监听套接字状态由CLOSE转换成LISTEN**，当客户端调用connect后，**客户端套接字状态由CLOSE转换成SYN_SENT**，此时触发三次握手，在三次握手阶段，内核负责控制两个队列，**半连接队列（syn队列）和全连接队列（accept队列）**

* 第一次握手SYN：客户端发送连接请求给服务端，服务端接收到请求后判断半连接队列是否为满，若已满会丢弃请求或者回送RST报文，若半连接队列没满，则弹出队列并加入半连接队列，**服务端监听套接字状态由LISTEN转换成SYN_RECV**，并发起第二次握手SYN+ACK
* 第二次握手SYN+ACK：服务端发送连接确认给客户端，**客户端接收到确认后套接字状态由SYN_RECV转换成ESTABLISHED**，并发起第三次握手ACK
* 第三次握手ACK：客户端发送连接确认给服务端，服务端接收到确实后判断全连接队列是否为满，若已满会丢弃请求或者回送RST报文，若全连接队列没满，则从半连接队列中弹出对应请求，并加入全连接队列，**服务端监听套接字状态由SYN_RECV转换成ESTABLISHED**

> 内核参数/proc/sys/net/ipv4/tcp_abort_on_overflow用于控制半/连接队列满时内核的处理方式，若为1，则回送RST报文，若为0，则丢弃，客户端收到RST报文后，connect函数会报错，errno置ECONNRESET（Connection reset by peer）

**上述三次握手的过程都是由内核来组织的**，当完成三次握手后，服务端才可以调用accept函数取走全连接队列中的连接请求，并进行socket通信

> 半连接队列的大小由内核参数/proc/sys/net/ipv4/tcp_max_syn_backlog指定
>
> 全连接队列的大小由listen函数的参数backlog和内核参数/proc/sys/net/core/somaxconn指定，为min(backlog, /proc/sys/net/core/somaxconn)
>
> 以上适用于Linux 2.2以后的内核版本

三次握手若出现异常：

* 在客户端发起第一次握手SYN后，客户端在指定时间内没有收到服务端的第二次握手SYN+ACK，则触发超时重发，超时重传时间由内核指定，不可修改，重复次数由内核参数/proc/sys/net/ipv4/tcp_retries2指定，超过一定次数，客户端connect函数会报错，errno置ETIMEOUT（Connection timed out）

  > 客户端没有收到第二次握手的原因很多，第一次或者第二次握手丢失，数据损坏，服务端半连接队列已满等等

* 在服务端发起第二次握手SYN+ACK后，服务端在指定时间内没有收到客户端的第三次握手ACK，则触发超时重发，超时重传时间和重复次数同上述，超过一定次数，服务端会放弃该连接，将请求从半连接队列移除，并将套接字状态重置

  > 服务端没有收到第三次握手的原因很多，第二次或者第三次握手丢失，数据损坏，**还有可能是客户端故意不进行第三次握手，这就是SYN攻击的原理**
  >
  > 客户端可以伪造假的IP发送地址，导致第二次握手不可达，这样就会让服务端监听套接字卡在SYN_RECV状态，连接请求也会卡在半连接队列中，并且不断超时重发
  >
  > 若这种恶意客户端数量很多，会导致大量的连接请求堆积在半连接队列中，最终使半连接队列占满，无法响应正常客户端的连接请求，使服务端瘫痪无法响应服务（虽然前面提到服务端超时重发到一定次数后会放弃该连接，但是这个速度远远小于客户端的请求速度）
  >
  > 解决这种问题的方法有：
  >
  > * 增加半连接队列大小，同样，也应该增加全连接队列大小，这样半连接队列变满的速度会变慢
  > * 开启tcp_syncookies功能：
  >   * 开启这个功能后会不使用半连接队列实现三次握手，当客户端发起第一次握手SYN时，服务端根据第一次握手的信息构造cookie值通过第二次握手SYN+ACK发送给客户端，客户端根据算法构造新的cookie通过第三次握手带回给服务端，服务端验证cookie通过后将其加入全连接队列
  >   * 这个方法虽然可以解决SYN攻击问题，但本身也存在一些问题，参见https://zhuanlan.zhihu.com/p/357887244
  > * 减少第二次握手SYN+ACK的重传次数，这样服务器能够更快速地放弃恶意连接
  >
  > SYN攻击也是DoS（Denial of Service）拒绝服务攻击的一种

* 在客户端收到第二次握手SYN+ACK后，但是第三次握手失败，客户端无法感知这个情况，并且状态已经切换成ESTABLISHED，在这种情况下，若客户端没有需要发送的数据，隔一段时间后会检查连接状态，检查失败后会重置状态为SYN_SENT，若有发送的数据，服务端不会进行回应，然后客户端会发现连接失败，重置状态为SYN_SENT

  > 第三次握手失败的原因很多，第三次握手ACK丢失或者全连接队列已满等等
