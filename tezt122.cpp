#include "text12.h"
#define port_number 7856
int main(){
    int severfd;//服务器套接字
    struct sockaddr_in sever;//服务器ip地址
    struct sockaddr_in cliner;//客户端ip地址
    severfd=socket(AF_INET,SOCK_STREAM,0);//创建套接字
    if(severfd<0){
        perror("创建失败");
        close(severfd);
        return -1;
    }
    memset(&sever,0,sizeof(sever));//初始化结构体，这里不能写套接字，因为fd是一个文件描述符，清空就会导致程序无法连接
    sever.sin_family=AF_INET;
    sever.sin_port=htons(port_number);
    uint32_t ip=inet_addr("118.25.27.206");//将十进制ip地址转换成网络字节序二进制地址（大端）
    if(connect(severfd,(struct sockaddr*)&sever,sizeof(sever))<0){//这个函数是连接服务器的函数
        perror("连接失败");
        close(severfd);
        return -1;
    }
    else {
        std::cout<<"连接成功"<<std::endl<<std::flush;
    }
    while(true){
        char buf[1024];
         memset(&buf,0,sizeof(buf));
        std::string wt;
        getline(std::cin,wt);
        write(severfd,wt.c_str(),wt.size());
         int n=read(severfd,buf,sizeof(buf));
          if(n<0) {
            perror("数据读取失败");
            close(severfd);
            continue;
        }
        std::cout<<"读到数据"<<buf<<std::endl<<std::flush;
        buf[n]='\0';
        std::cout<<n<<"字节"<<std::endl;
        std::cout.flush();
    }
    close(severfd);
    return 0;
}