#include "text12.h"

#include <stdlib.h>

bool send_message(int fd, const std::string& msg) {
    int total = 0;
    int len = msg.size();

    while (total < len) {
        int n = send(fd, msg.c_str() + total, len - total, 0);
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

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);

    const char* server_ip = "127.0.0.1";
    int port = PORT_NUMBER;

    if (argc > 1) {
        server_ip = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cout << "IP地址不合法" << std::endl;
        close(server_fd);
        return -1;
    }

    if (connect(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(server_fd);
        return -1;
    }

    std::cout << "连接服务器成功: " << server_ip << ":" << port << std::endl;

    std::string name;
    std::cout << "请输入你的昵称: ";
    std::getline(std::cin, name);
    if (name.empty()) {
        name = "游客";
    }

    if (!send_message(server_fd, name + "\n")) {
        std::cout << "发送昵称失败" << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "可以开始聊天了，输入 quit 退出" << std::endl;

    // 同时监听键盘和服务器，这样就不会出现发一条才收一条
    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(server_fd, &readfds);

        int maxfd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;
        int ret = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            break;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            char buf[BUF_SIZE];
            memset(buf, 0, sizeof(buf));
            int n = recv(server_fd, buf, sizeof(buf) - 1, 0);
            if (n < 0) {
                perror("recv");
                break;
            }

            if (n == 0) {
                std::cout << "服务器已经断开连接" << std::endl;
                break;
            }

            buf[n] = '\0';
            std::cout << buf;
            std::cout.flush();
        }

        if (!FD_ISSET(STDIN_FILENO, &readfds)) {
            continue;
        }

        std::string msg;
        if (!std::getline(std::cin, msg)) {
            break;
        }

        if (msg.empty()) {
            continue;
        }

        if (msg == "quit") {
            send_message(server_fd, "/quit\n");
            break;
        }

        if (!send_message(server_fd, msg + "\n")) {
            std::cout << "发送消息失败" << std::endl;
            break;
        }
    }

    close(server_fd);
    return 0;
}
