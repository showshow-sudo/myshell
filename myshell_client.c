#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    // 第一步：创建socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 第二步：配置服务器地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // 127.0.0.1 = localhost = 本机地址
    // 用于测试：服务端和客户端跑在同一台机器上

    // 第三步：连接服务器（三次握手在这里发生）
    connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("已连接到服务器！\n");

    char input[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    while (1) {
        // 显示提示符
        printf("remote> ");
        fflush(stdout);

        // 读取用户输入
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) break;
        if (strlen(input) == 0) continue;

        // 发送命令给服务器
        send(client_fd, input, strlen(input), 0);

        // 接收执行结果
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        printf("%s\n", buffer);
    }

    close(client_fd);
    return 0;
}