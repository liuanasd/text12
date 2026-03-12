    #include "text12.h"
    #define bufsize 1024
    #define port_number 7856
    int main()
    {
        int severfd;//服务器套接字
        struct sockaddr_in sever;//服务器ip
        struct sockaddr_in client;//客户端ip
        char buf[bufsize];//数据缓冲区
        severfd=socket(AF_INET,SOCK_STREAM,0);//创建套接字
        if(severfd<0){
            perror("创建失败");
            close(severfd);
            return -1;
        }
        memset(&sever,0,sizeof(sever));//初始化结构体
        sever.sin_family=AF_INET;//声明这个是ipv4的协议族
        sever.sin_port=htons(port_number);//设置端口号
        sever.sin_addr.s_addr = htonl(INADDR_ANY);//监听所有的网络地址
        if(bind(severfd,(struct sockaddr*)&sever,sizeof(sever))<0){
            perror("绑定失败");
            close(severfd);
            return -1;
        }//绑定套接字和地址
        if(listen(severfd,10)<0){
            perror("监听失败");
            close(severfd);
            return -1;
        }//监听套接字，这个是决定服务器是否可以链接的函数
        while(true){
            socklen_t address_length=sizeof(client);//客户端长度地址，为了后续的accept函数的调用
            int client_fd=accept(severfd,(struct sockaddr*)&client,&address_length);//这个函数是等待客户端连接的函数，成功了就返回一个新的套接字描述符
            if(client_fd<0){
                perror("此次连接失败，等待下一个连接");
                continue ;
            }
            std::cout<<"连接成功"<<std::endl;
            while(true){//循环读取数据
             memset(buf,0,bufsize);//初始化缓冲区
            int n=read(client_fd,buf,bufsize-1);//读取数据，最后一个字节留给'\0'，为了设置字符串结束符号
             if(n<=0){
            std::cout<<"关闭了连接"<<std::endl<<std::flush;
            continue;
        }//当n=0时代表客户端关闭了连接
                buf[n]='\0';//字符串结束符号
                   std::cout<<"读取到数据"<<buf<<std::endl<<std::flush;
            std::string wt;
             std::cout<<"输入要发送的文字"<<std::endl<<std::flush;
             std::getline(std::cin,wt);//读取输入的字符串
             write(client_fd,wt.c_str(),wt.size());
            }
             //close(client_fd);无论如何每次都要关闭客户端套接字，为了下一个连接进来
             continue;
        close(severfd);
    }
}
     
  