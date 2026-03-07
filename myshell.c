#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#define MAX_INPUT 1024
#define MAX_ARGS  64

pid_t child_pid = -1;  // 全局变量，存当前子进程的pid
// -1 表示当前没有子进程在运行

void handle_sigint(int sig) {
    if (child_pid > 0) {
        kill(child_pid, SIGINT);  // 杀死子进程
    }
}

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    signal(SIGINT, handle_sigint);
    
    while (1) {
        // 1. 显示提示符
        printf("myshell> ");
        fflush(stdout);

        // 2. 读取用户输入
        if (fgets(input, MAX_INPUT, stdin) == NULL) break;

        // 3. 去掉末尾换行符
        input[strcspn(input, "\n")] = 0;

        // 4. 切割参数
        int argc = 0;
        args[0] = strtok(input, " ");
        while (args[argc] != NULL) {
            argc++;
            args[argc] = strtok(NULL, " ");
        }

        if (argc == 0) continue;

        // 5. 内置命令：exit
        if (strcmp(args[0], "exit") == 0) break;

        // 6. fork + exec
        pid_t pid = fork();

        if (pid == 0) {
            execvp(args[0], args);
            perror("execvp failed");
            exit(1);
        } else {
            child_pid = pid;   // ← 新增这行
            wait(NULL);
            child_pid = -1;    // ← 新增这行，等待结束后重置
        }
    }
    return 0;
}