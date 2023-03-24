### IO多路复用的概念

传统的socket通信的框架，服务端在处理客户端的连接请求，以及和客户端进行交互时，经常会出现阻塞现象，诸如：监听客户端的连接请求时，accept函数出现阻塞，等待客户端给服务端发送数据时，read函数出现阻塞

这些阻塞现象也导致服务器的效率低下，在阻塞的时间段内不能完成其他事情

因此，IO多路复用的框架是用于解决服务端大量阻塞等待处理的现象，它的思想是：

服务端不再阻塞监听等待**某一个**文件描述符的事件发生，诸如监听socket监听到新连接请求，连接socket接收到客户端发送的数据等等，而是将需要关注的**多个文件描述符进行统一的阻塞监听**，若被关注某些的文件描述符触发了相应的事件，服务端就会获得这些触发事件的文件描述符的列表，再挨个进行处理，此时对文件描述符进行相应的IO操作是不会出现阻塞的

换言之，**IO多路复用即将大量的文件描述符的IO阻塞监听汇总到一起，统一监听**，随后对每一个文件描述符的IO处理都不需要阻塞，从而提高服务器性能

> 通过IO多路复用，服务器采取单进程（线程）就可以实现对多客户端的信息交互，而传统的socket通信则必须采取多进程（线程）才可以实现对多客户端的信息交互



### 文件描述符集合

为了实现IO多路复用，Linux定义了文件描述符集合以及相关操作

文件描述符集合fd_set结构体类似sigset_t结构体，底层都是利用位图来实现的，不过屏蔽了底层细节，并提供部分接口来修改它的内容，包括如下：

```c
   #include <sys/select.h>  
   
   void FD_CLR(int fd, fd_set *set);
   int  FD_ISSET(int fd, fd_set *set);
   void FD_SET(int fd, fd_set *set);
   void FD_ZERO(fd_set *set);
```

FD_CLR：将文件描述符fd从文件描述符集合set中移除

FD_ISSET：判断文件描述符fd在文件描述符集合set中是否存在，若存在返回1，否则返回0

FD_SET：将文件描述符fd添加到文件描述符集合set中

FD_ZERO：将文件描述符集合清空



### select

```c
   #include <sys/select.h>

   int select(int nfds, fd_set *readfds, fd_set *writefds,
              fd_set *exceptfds, struct timeval *timeout);
```

监听多个文件描述符的的事件是否发生，并通过出参返回发生事件的文件描述符的列表，调用成功返回发生事件的文件描述符的个数，异常错误返回-1，参数包括：

* nfds：指明被监听的文件描述符的总数，假设nfds为N，那么被监听的文件描述符的取值范围为[0, N - 1]，**nfds通常取需要监听的所有文件描述符中的最大值加1**
* readfds：被监听**可读**事件的文件描述符集合，若不需要监听可读事件，传入NULL
* writefds：被监听**可写**事件的文件描述符集合，若不需要监听可写事件，传入NULL
* exceptfds：被监听**异常**事件的文件描述符集合，若不需要监听异常事件，传入NULL

> 需要注意，readfds、writefds、exceptfds**即是入参也是出参**
>
> * 做入参时，设置需要监听对应事件的文件描述符的集合
>
> * 作为出参时，会返回发生对应事件的文件描述符的集合

* timeout：用于设置监听的持续时间，设置如下：

  | timeout的参数 |                             含义                             |
  | :-----------: | :----------------------------------------------------------: |
  |     NULL      |    无限期阻塞监听，直到监听到事件或者被信号打断（EINTR）     |
  |       0       |       无阻塞，若没监听到事件则返回0，一般用于轮询处理        |
  |      > 0      | 有限期阻塞监听，直到监听到事件、timeout到时或者被信号打断（EINTR） |

下面是单进程（线程）实现的基于select实现IO多路复用的服务端demo：

```c++
int main() {
    int listenFd = usocket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in svrAddr{};
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_port = htons(1080);
    svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ubind(listenFd, (const sockaddr *)&svrAddr, sizeof(sockaddr_in));
    ulisten(listenFd, 16);

    fd_set readSet{}, readSetBk{};   // readSet用于出参，获取发生可读事件的集合   readSetBk用于入参，保存需要监听可读事件的集合
    unordered_set<int> allSet;       // 存放所有被监听的连接socket的集合用于检索
    char buf[1024]{};
    int maxFd = listenFd;            // 定义最大fd值
    FD_ZERO(&readSetBk);             // 初始化清空readSetBk
    FD_SET(listenFd, &readSetBk);    // 将监听socket加入需要监听可读事件的集合

    while (true) {
        readSet = readSetBk;
        int fdNum = uselect(maxFd + 1, &readSet, nullptr, nullptr, nullptr);  // 阻塞监听
        if (FD_ISSET(listenFd, &readSet)) {   // 有新的客户端连接请求，即有监听socket的可读事件发生
            sockaddr_in cltAddr{};
            socklen_t cltAddrLen = sizeof(sockaddr_in);
            int connectFd = uaccept(listenFd, (sockaddr *)&cltAddr, &cltAddrLen);
            FD_SET(connectFd, &readSetBk);   // 将新的连接socket加入下一轮的需要监听可读事件发生的集合中
            allSet.emplace(connectFd);
            maxFd = max(connectFd, maxFd);   // 更新最大fd值
            if (0 == --fdNum) {   // 只有监听socket的可读事件发生，即无客户端发送数据，无需处理连接socket，进行下一轮监听
                continue;
            }
        }
        // 处理所有发生可读事件的连接socket
        auto it = allSet.begin();
        while (it != allSet.end()) {
            if (FD_ISSET(*it, &readSet)) {
                ssize_t count = uread(*it, buf, sizeof(buf));
                if (0 == count) {  // 若客户端主动断开连接，则关闭套接字并将对应的连接套接字从需要监听可读事件的集合中移除
                    uclose(*it);
                    FD_CLR(*it, &readSetBk);
                    it = allSet.erase(it);
                    continue;
                }
                // 将客户端发送的字符串中的字母由小写转大写并返回
                for (int i = 0; i < count; ++i) {
                    buf[i] = toupper(buf[i]);
                }
                uwrite(*it, buf, count);
                memset(buf, 0, count);
                // 剩余的发生可读事件的连接socket已经全部处理完成，可以提前跳出循环
                if (0 == --fdNum) {  
                    break;
                }
            }
            ++it;
        }
    }
}
```

> 这个demo中省略了头文件，系统调用封装了一层加了前缀u，用于进行异常情况处理
>
> 这里面用哈希表来存储需要检索的所有文件描述符，存在争议，有人认为没有必要，用数组即可，甚至不使用？

