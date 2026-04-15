#include "text12.h"

#include <stdlib.h>

std::vector<ClientInfo> client_list;
pthread_mutex_t client_mutex;

struct ThreadArg {
    int client_fd;
};

std::string get_client_address(const sockaddr_in& client_addr) {
    char ip[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
    return std::string(ip) + ":" + std::to_string(ntohs(client_addr.sin_port));
}

bool send_message(int client_fd, const std::string& msg) {
    int total = 0;
    int len = msg.size();

    while (total < len) {
        int n = send(client_fd, msg.c_str() + total, len - total, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (n == 0) {
            return false;
        }
        total += n;
    }

    return true;
}

int add_client(int client_fd, const std::string& name) {
    pthread_mutex_lock(&client_mutex);

    ClientInfo info;
    info.client_fd = client_fd;
    info.name = name;
    client_list.push_back(info);

    int count = client_list.size();
    pthread_mutex_unlock(&client_mutex);
    return count;
}

int remove_client(int client_fd, std::string& name) {
    pthread_mutex_lock(&client_mutex);

    for (size_t i = 0; i < client_list.size(); i++) {
        if (client_list[i].client_fd == client_fd) {
            name = client_list[i].name;
            client_list.erase(client_list.begin() + i);
            break;
        }
    }

    int count = client_list.size();
    pthread_mutex_unlock(&client_mutex);
    return count;
}

void broadcast_message(const std::string& msg, int skip_fd) {
    pthread_mutex_lock(&client_mutex);

    for (size_t i = 0; i < client_list.size(); i++) {
        if (client_list[i].client_fd == skip_fd) {
            continue;
        }

        if (!send_message(client_list[i].client_fd, msg)) {
            std::cout << "给 " << client_list[i].name << " 发送消息失败" << std::endl;
        }
    }

    pthread_mutex_unlock(&client_mutex);
}

void handle_client_exit(int client_fd, const std::string& name) {
    if (name.empty()) {
        close(client_fd);
        return;
    }

    std::string remove_name;
    int count = remove_client(client_fd, remove_name);
    close(client_fd);

    std::string out_name = name;
    if (!remove_name.empty()) {
        out_name = remove_name;
    }

    std::string msg = "[系统] " + out_name + " 离开聊天室，当前在线人数: " + std::to_string(count) + "\n";
    std::cout << msg;
    std::cout.flush();
    broadcast_message(msg, -1);
}

void handle_line(int client_fd, const std::string& line, std::string& name, bool& login_ok) {
    if (line.empty()) {
        return;
    }

    // 客户端连上后发来的第一条消息，默认当成昵称
    if (!login_ok) {
        name = line;
        if (name.size() > NAME_SIZE) {
            name = name.substr(0, NAME_SIZE);
        }
        if (name.empty()) {
            name = "游客";
        }

        int count = add_client(client_fd, name);
        login_ok = true;

        std::string welcome = "[系统] 欢迎 " + name + " 进入聊天室\n";
        std::string online_msg = "[系统] " + name + " 上线了，当前在线人数: " + std::to_string(count) + "\n";

        send_message(client_fd, welcome);
        std::cout << online_msg;
        std::cout.flush();
        broadcast_message(online_msg, -1);
        return;
    }

    if (line == "/quit") {
        handle_client_exit(client_fd, name);
        pthread_exit(NULL);
    }

    std::string msg = "[" + name + "] " + line + "\n";
    std::cout << msg;
    std::cout.flush();
    broadcast_message(msg, -1);
}

void* client_thread(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    int client_fd = thread_arg->client_fd;
    delete thread_arg;

    std::string name;
    std::string cache;
    bool login_ok = false;

    char buf[BUF_SIZE];
    while (true) {
        memset(buf, 0, sizeof(buf));
        int n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("recv");
            break;
        }

        if (n == 0) {
            break;
        }

        // 用换行符区分一条完整消息，避免一次 recv 收到半条或多条
        cache.append(buf, n);

        std::string::size_type pos = 0;
        while ((pos = cache.find('\n')) != std::string::npos) {
            std::string line = cache.substr(0, pos);
            cache.erase(0, pos + 1);

            if (!line.empty() && line[line.size() - 1] == '\r') {
                line.erase(line.size() - 1, 1);
            }

            handle_line(client_fd, line, name, login_ok);
        }
    }

    handle_client_exit(client_fd, name);
    return NULL;
}

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);
    pthread_mutex_init(&client_mutex, NULL);

    int port = PORT_NUMBER;
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0) {
            port = PORT_NUMBER;
        }
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 20) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    std::cout << "服务器启动成功，监听端口: " << port << std::endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        std::cout << "新客户端连接: " << get_client_address(client_addr) << std::endl;

        ThreadArg* thread_arg = new ThreadArg;
        thread_arg->client_fd = client_fd;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, thread_arg) != 0) {
            perror("pthread_create");
            close(client_fd);
            delete thread_arg;
            continue;
        }

        pthread_detach(tid);
    }

    close(server_fd);
    pthread_mutex_destroy(&client_mutex);
    return 0;
}
