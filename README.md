# MyShell

基于C语言实现的Linux Shell，支持本地执行和远程命令执行。

## 功能

- 命令执行：fork-exec模型
- 管道：pipe() + dup2()
- 重定向：> 和 
- 信号处理：Ctrl+C只终止子进程
- 命令历史：SQLite存储，history命令查看
- 远程执行：TCP socket，CS架构

## 编译运行

### 本地Shell
​```bash
gcc myshell.c -o myshell -lsqlite3
./myshell
​```

### 远程Shell
​```bash
# 服务端
gcc myshell_server.c -o myshell_server
./myshell_server

# 客户端
gcc myshell_client.c -o myshell_client
./myshell_client
​```

## 技术栈
- 语言：C
- 系统调用：fork, exec, pipe, dup2, signal, socket
- 数据库：SQLite3
- 网络：TCP/IP