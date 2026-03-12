        #include "text12.h"
        #define bufsize 1024
        #define port_number 7856
        std::vector<int> sockname;//保存套接字
        pthread_mutex_t mutex; //互斥锁
        struct sever_init{
            int seveerfd;//套接字
            struct sockaddr_in sever;//服务器地址
        };
        struct sockaddr_in cliner;//客户地址
        void task(){
            
        }
        void* asd(void *arg){
        int s= (long)arg;
            while(true){
        char buf[bufsize];
        memset(&buf,0,sizeof(buf));
        int n=read(s,buf,sizeof(buf));
        if(n<0){
            perror("读取失败");
            pthread_mutex_lock(&mutex);
             for(int i=0;i<sockname.size();i++){
                if(sockname[i]==s){
                    sockname.erase(sockname.begin()+i);
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            close(s);
            return NULL;
        }
        if(n==0){
            std::cout<<"客户端断开连接"<<std::endl;
            pthread_mutex_lock(&mutex);
            for(int i=0;i<sockname.size();i++){
                if(sockname[i]==s){
                    sockname.erase(sockname.begin()+i);
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            close (s);
            return NULL;
        }
        buf[n]='\0';
        if(n>0){
            std::cout<<"读到数据"<<buf<<std::endl<<std::flush;
            std::cout<<n<<std::endl;
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
        sockname.push_back(s);
        pthread_mutex_unlock(&mutex);
        pthread_t tid;
        if(pthread_create(&tid,NULL,asd,(void*)s)!=0){
            perror("线程创建失败");
            pthread_mutex_lock(&mutex);
            sockname.pop_back();
            pthread_mutex_unlock(&mutex);
            continue;
        }
        pthread_detach(tid);
        }
        }

        
    