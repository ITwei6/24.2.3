
#include "Udpserver.hpp"
#include <memory>
#include <cstdio>
#include <stdlib.h>
// "120.78.126.148" 点分十进制字符串风格的IP地址
/*std::string handler(const std::string &info,uint16_t&port,const std::string ip)
{ 
   //让服务器打印出是谁发送的消息
   std::cout<<"["<<ip<<":"<<port<<"]#"<<info<<std::endl; 
   std::string res="get a message: ";
   res+=info;
   std::cout<<res<<std::endl;
   return res;//最后将处理的数据返回回去
}

std::string ExcuteCommand(const std::string& cmd)
{
    std::cout<<"get a request cmd: "<<cmd<<std::endl;
    //服务器端接收到命令后，会先打印获取到一个命令
    //获取到命令后，就让服务器进行运行处理

    FILE* fp=popen(cmd.c_str(),"r");
    //popen会自动创建子进程，让子进程进行程序替换，运行字符串命令，然后会建立父子间的管道，将运行结果发送给父进程
    if(fp==nullptr)
    {
        perror("popen");
        return "error";
    }
    std::string reslut;
    char buffer[1024];
    while(true)
    {
        char*r=fgets(buffer,sizeof(buffer),fp);//按行读取，读取一行
        if(r==nullptr)break;
        reslut+=r;
    }
    pclose(fp);
    return reslut;
}
*/
#include "Udpserver.hpp"
#include <memory>
#include <cstdio>

void Usage(std::string proc)
{
    std::cout<<"\n\rUsage: "<<proc<<" port[1024+]\n"<<std::endl;
}
//服务器进程，启动时，按照./Udpserver+port的形式传递
int main(int args,char* argv[])
{
    if(args!=2)
    {
         Usage(argv[0]);
         exit(0);
    }

    uint16_t port=std::stoi(argv[1]);
    std::unique_ptr<Udpserver> svr(new Udpserver(port));//首先创建一个服务器对象指针
    //智能指针，用一个UdpServer指针来管理类对象
    svr->Init();//初始化服务器
    
    svr->Run();//启动服务器
    return 0;
} 