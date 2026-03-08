#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>   // socket相关
#include <netinet/in.h>   // sockaddr_in结构体
#include <arpa/inet.h>    // inet_addr

#define PORT 8888         // 监听的端口号
#define BUFFER_SIZE 1024

int main() {
    // 第一步：创建socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET   = Address Family Internet = IPv4
    // SOCK_STREAM = 流式socket = TCP

    // 第二步：配置服务器地址
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // 接受任意IP的连接
    address.sin_port = htons(PORT);
    // htons 读音："h-to-n-s"
    // 含义：host to network short
    // 作用：把端口号转成网络字节序

    // 第三步：绑定端口
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // 第四步：开始监听
    listen(server_fd, 5);  // 5 = 最多5个客户端排队等待
    printf("服务器启动，监听端口 %d...\n", PORT);

    // 第五步：接受连接
    int client_fd;
    int addrlen = sizeof(address);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, 
                          (socklen_t *)&addrlen);
        printf("客户端已连接！\n");

        char buffer[BUFFER_SIZE];

        // 第六步：收命令、执行、发结果
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
            if (bytes <= 0) break;  // 客户端断开了

            printf("收到命令：%s\n", buffer);

            // 执行命令，把结果写入临时文件
            char cmd[BUFFER_SIZE*2];
            sprintf(cmd, "%s > /tmp/output.txt 2>&1", buffer);
            //                               ↑
            //                    2>&1 = 把错误输出也重定向进去
            system(cmd);

            // 读取结果文件，发给客户端
            FILE *fp = fopen("/tmp/output.txt", "r");
            char result[BUFFER_SIZE];
            memset(result, 0, BUFFER_SIZE);
            fread(result, 1, BUFFER_SIZE - 1, fp);
            fclose(fp);

            send(client_fd, result, strlen(result), 0);
        }
        close(client_fd);
    }
    return 0;
}