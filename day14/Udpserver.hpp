#pragma once

#include "Log.hpp"
#include <iostream>
#include <string>
#include <strings.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>
#include <unordered_map>
std::string defaultip = "0.0.0.0";
uint16_t defaultport = 8080;
Log lg;                                                                                                   // 日志，默认往显示屏打印
typedef std::function<std::string(const std::string &info, uint16_t &port, const std::string ip)> func_t; // 相当于定义了一个函数指针
// 返回值是string类型，函数参数也是string类型，利用函数回调的方法，将服务器端对数据的处理操作进行分离，由上层传递的函数来决定如何处理
enum
{
    SOCKET_ERR = 1,
    BIND_ERR
};
class Udpserver
{
public:
    Udpserver(const uint16_t &port = defaultport, std::string &ip = defaultip) : _sockfd(0), _port(port), _ip(ip)
    {
    }
    void Init()
    {
        // 1.创建udp套接字，本质就是打开网络套接字文件
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sockfd < 0) // 表示创建失败
        {
            lg(Fatal, "socket create error,socket: %d", _sockfd);
            exit(SOCKET_ERR); // 创建失败之间退出
        }
        // 创建成功
        lg(Info, "socket create success,socket: %d", _sockfd);
        // 2.绑定服务器的套接信息，比如ip和端口号
        // 在绑定套接信息之前，需要先将对应的结构体对象填充完毕sock_addr
        struct sockaddr_in local;                       // 网络通信类型
        bzero(&local, sizeof(local));                   // 将内容置为0
        local.sin_family = AF_INET;                     // 网络通信类型
        local.sin_port = htons(_port);                  // 网络通信中，端口号需要不断发送，所以需要符合网络字节序,主机--->网络字节序
        local.sin_addr.s_addr = inet_addr(_ip.c_str()); // 需要将string类型的ip转换成int类型，并且还需要满足网络字节序的要求
        socklen_t len = sizeof(local);
        // 以上只是将要绑定的信息填充完毕，套接字(网络文件)而还没有绑定套接信息

        if (bind(_sockfd, (const struct sockaddr *)&local, len) < 0) // 绑定失败
        {
            lg(Fatal, "bind sockfd error,errno：%d,err string:%s", errno, strerror(errno));
            exit(BIND_ERR);
        }
        lg(Info, "bind sockfd success,errno：%d,err string:%s", errno, strerror(errno)); // 绑定成功
    }
    void Checkuser(struct sockaddr_in &client,uint16_t &clientport,const std::string &clientip)
    {
       
         //用该ip检测哈希表里是否存在该用户
         auto iter=online_user.find(clientip);
         if(iter==online_user.end())//说明没有找到， 是新用户
         {
              online_user.insert({clientip,client});
            std::cout<<"add a user"<<":["<<clientip<<":"<<clientport<<"]"<<std::endl; 
         }
         //如果不是新用户，那么什么都不干
        
    }
    void Broadcast(const std::string info,uint16_t &clientport,const std::string &clientip)
    {
          //将消息加工处理广播发送给所有在线的用户，而这些用户都在哈希表里
          for(auto& user:online_user)
          {
            std::string message="[处理客户端数据 ";
            message+=clientip;
            message+=":";
            message+=std::to_string(clientport);
            message+="]";
            message+=info;
            
            sendto(_sockfd, message.c_str(), message.size(), 0, (const struct sockaddr *)&(user.second), sizeof(user.second));
          }
    }
    void Run(/*func_t func*/) // 服务器是一旦启动不会退出，服务器接收消息，并发送答应
    {
        // 1.接收信息
        char buffer[SIZE];

        while (true)
        {
            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            // 服务器接收到消息，它还需要知道谁给它发送的，为了后续将应答返回过去
            // 利用一个输出型参数，将对方的网络信息填充到里面
            ssize_t n = recvfrom(_sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client, &len);
            if (n < 0)
            {
                lg(Warning, "recvfrom sockfd err,errno: %d, err string %s", errno, strerror(errno));
                continue;
            }
            // 读取成功，将网络文件里的消息读取到buffer里
            buffer[n] = 0; // 字符串形式


              
            // 服务器想知道是谁给它发送消息的//①获取到发送者的消息②也获取到发送者的套接字信息，所以我们怎么知道是谁发送过来的呢？
            uint16_t clientport = ntohs(client.sin_port);      // 注意port是网络字节序，需要转成用户字节序
            std::string clientip = inet_ntoa(client.sin_addr); // ip是网络字节序的int类型，需要转换成string类型
            Checkuser(client,clientport,clientip);//在线用户列表 
            std::string info = buffer;

            
            //接收消息后，就广播发送给所有在线的人,并告诉是谁发送的
            Broadcast(info,clientport,clientip);
              



            // std::string info = buffer;
            // std::string echo_string  = "server echo# " + info;

            // std::string info=buffer;
            // std::string echo_string=func(info,clientport,clientip);
            // 将接收的信息由外层函数进行处理

            // 3.将应答发送回去
            // 发送给谁呢？服务器知道吗？服务器知道！因为在接收消息时，服务器就用一个输出型参数，将客户端的网络消息保存下来了
            //sendto(_sockfd, echo_string.c_str(), echo_string.size(), 0, (const struct sockaddr *)&client, len);
        }
    }
    ~Udpserver()
    {
        if (_sockfd > 0)
            close(_sockfd);
    }

private:
    int _sockfd;     // 套接字文件描述符
    std::string _ip; // 我们习惯用string类型的ip地址
    uint16_t _port;  // 服务器进程的端口号

    std::unordered_map<std::string, struct sockaddr_in> online_user; // 网上在线列表
    // 服务器收到一个客户信息，就要检测这个客户是不是新用户，如果是就添加进去
};
