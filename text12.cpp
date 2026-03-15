        #include "text12.h"
        #define bufsize 1024
        #define port_number 7856
        int epfd;
        pthread_mutex_t mutex; //互斥锁
        struct sever_init{
            int seveerfd;//套接字
            struct sockaddr_in sever;//服务器地址
        };
        struct sockaddr_in cliner;//客户地址
        void task(){
            
        }
        void* asd(void *arg){
        int s= (int)(long)arg;
            while(true){
        char buf[bufsize];
        memset(&buf,0,sizeof(buf));
        int n=read(s,buf,sizeof(buf));
        if(n<0){
            perror("读取失败");
            pthread_mutex_lock(&mutex);
           epoll_ctl(epfd,EPOLL_CTL_DEL,s,NULL);
            pthread_mutex_unlock(&mutex);
            close(s);
            return NULL;
        }
        if(n==0){
            std::cout<<"客户端断开连接"<<std::endl;
            pthread_mutex_lock(&mutex);
          epoll_ctl(epfd,EPOLL_CTL_DEL,s,NULL);
            pthread_mutex_unlock(&mutex);
            close (s);
            return NULL;
        }
        if(n>0){
            buf[n]='\0';
            std::cout<<"读到数据"<<buf<<std::endl;
            std::cout<<n<<"字节"<<std::endl;
            memset(&buf,0,sizeof(buf));
            std::string wt;
            getline(std::cin,wt);
            write(s,wt.c_str(),wt.size());
            std::cout.flush();
        }
        }
        }
        int main()
        {
            pthread_mutex_init(&mutex, NULL);
        sever_init sever_fd;
        memset(&sever_fd.sever,0,sizeof(sever_fd.sever));
        sever_fd.seveerfd=socket(AF_INET,SOCK_STREAM,0);
        sever_fd.sever.sin_family=AF_INET;
        sever_fd.sever.sin_port=htons(port_number);
        sever_fd.sever.sin_addr.s_addr=htonl(INADDR_ANY);
        int opt=1;
        if(setsockopt(sever_fd.seveerfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0){
            perror("端口复用失败");
            close(sever_fd.seveerfd);
            return -1;
        }
        if(bind(sever_fd.seveerfd,(struct sockaddr*)&sever_fd.sever,sizeof(sever_fd.sever))<0){
            perror("绑定失败");
            close(sever_fd.seveerfd);
            return -1;
        }
        if(listen(sever_fd.seveerfd,10)<0){
            perror("监听失败");
            close(sever_fd.seveerfd);
            return -1;
        }
        while(true){
            socklen_t len=sizeof(cliner);
        int s=(accept(sever_fd.seveerfd,(struct sockaddr*)&cliner,&len));
        if(s<0){
            perror("连接失败");
            continue;
        }
        pthread_mutex_lock(&mutex);
       epfd=epoll_create1(0);
       if(epfd<0){
        perror("epoll创建失败");
        return -1;
       }
     struct epoll_event ev;
     ev.events=EPOLLIN|EPOLLET;
     ev.data.fd=s;
     if(epoll_ctl(epfd,EPOLL_CTL_ADD,s,&ev)<0){
        perror("添加失败");
        return -1;
     }
        pthread_mutex_unlock(&mutex);
        pthread_t tid;
        if(pthread_create(&tid,NULL,asd,(void*)(long)s)!=0){
            perror("线程创建失败");
            pthread_mutex_lock(&mutex);
            epoll_ctl(epfd,EPOLL_CTL_DEL,s,NULL);
            pthread_mutex_unlock(&mutex);
            continue;
        }
        pthread_detach(tid);
        }
        }

        
    