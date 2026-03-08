#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>   
#include <sqlite3.h>
#define MAX_INPUT 1024
#define MAX_ARGS  64

pid_t child_pid = -1;
sqlite3 *db;  // 全局数据库连接

void handle_sigint(int sig) {
    if (child_pid > 0) {
        kill(child_pid, SIGINT);
    }
}

// 普通命令执行（之前写好的）
void run_command(char **args) {
    // 检测重定向符号
    char *output_file = NULL;   // > 的文件名
    char *input_file = NULL;    // < 的文件名
    int argc = 0;
    
    // 遍历args，找>和
    while (args[argc] != NULL) {
        if (strcmp(args[argc], ">") == 0) {
            output_file = args[argc + 1];  // >右边就是文件名
            args[argc] = NULL;             // 截断args
        } else if (strcmp(args[argc], "<") == 0) {
            input_file = args[argc + 1];   // <右边就是文件名
            args[argc] = NULL;             // 截断args
        }
        argc++;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // 处理输出重定向
        if (output_file != NULL) {
            int f = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(f, STDOUT_FILENO);
            close(f);
        }
        // 处理输入重定向
        if (input_file != NULL) {
            int f = open(input_file, O_RDONLY);
            dup2(f, STDIN_FILENO);
            close(f);
        }
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    } else {
        child_pid = pid;
        wait(NULL);
        child_pid = -1;
    }
}

// 管道执行
void run_pipe(char **args1, char **args2) {
    int fd[2];
    pipe(fd);  // 创建管道

    // 子进程1：执行左边命令（如ls）
    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(fd[1], STDOUT_FILENO);  // stdout → 管道写端
        close(fd[0]);                // 关掉读端
        close(fd[1]);                // dup2完了也关掉原fd
        execvp(args1[0], args1);
        perror("execvp failed");
        exit(1);
    }

    // 子进程2：执行右边命令（如grep）
    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(fd[0], STDIN_FILENO);   // stdin → 管道读端
        close(fd[1]);                // 关掉写端
        close(fd[0]);                // dup2完了也关掉原fd
        execvp(args2[0], args2);
        perror("execvp failed");
        exit(1);
    }

    // 父进程：关掉fd，等待两个子进程
    close(fd[0]);
    close(fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void init_db() {
    // 打开数据库（不存在会自动创建）
    sqlite3_open("history.db", &db);
    
    // 创建表（如果不存在）
    char *sql = "CREATE TABLE IF NOT EXISTS history ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "command TEXT,"
                "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    sqlite3_exec(db, sql, NULL, NULL, NULL);
}

void save_history(char *command) {
    char sql[1024];
    sprintf(sql, "INSERT INTO history (command) VALUES ('%s');", command);
    sqlite3_exec(db, sql, NULL, NULL, NULL);
}

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    init_db();  
    signal(SIGINT, handle_sigint);

    while (1) {
        printf("myshell> ");
        fflush(stdout);

        if (fgets(input, MAX_INPUT, stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;

        char input_backup[MAX_INPUT];
        strcpy(input_backup, input);  // 备份原始输入

        int argc = 0;
        args[0] = strtok(input, " ");
        while (args[argc] != NULL) {
            argc++;
            args[argc] = strtok(NULL, " ");
        }

        if (argc == 0) continue;
        if (strcmp(args[0], "exit") == 0) break;

        // 检测有没有管道符
        int pipe_pos = -1;
        for (int i = 0; i < argc; i++) {
            if (strcmp(args[i], "|") == 0) {
                pipe_pos = i;
                break;
            }
        }

        if (pipe_pos == -1) {
            // 没有管道，普通执行
            run_command(args);
        } else {
            // 有管道，切割成两组参数
            args[pipe_pos] = NULL;       // 在|处截断
            char **args1 = args;         // |左边
            char **args2 = args + pipe_pos + 1;  // |右边
            run_pipe(args1, args2);
        }
        save_history(input_backup); 
    }
    return 0;
}