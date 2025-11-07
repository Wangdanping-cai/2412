#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024   // 命令最大长度
#define MAX_ARGS 64        // 最大参数数量
#define DELIMITERS " \t\n" // 命令分割符（空格、制表符、换行符）

// 解析命令行，将输入字符串分割为参数数组
int parse_command(char *cmd, char **args) {
    int i = 0;
    // 分割命令为参数（首次调用strtok用cmd作为参数）
    char *token = strtok(cmd, DELIMITERS);
    
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, DELIMITERS); // 后续调用用NULL
    }
    args[i] = NULL; // 参数数组以NULL结尾（execvp要求）
    return i;       // 返回参数数量
}

// 执行外部命令（通过fork+execvp实现）
void execute_external(char **args) {
    pid_t pid = fork(); // 创建子进程
    if (pid == -1) {
        perror("fork失败");
        return;
    }
    
    if (pid == 0) { // 子进程：执行命令
        // execvp会在PATH路径中查找程序，args[0]为程序名
        if (execvp(args[0], args) == -1) {
            perror("命令执行失败");
            exit(EXIT_FAILURE); // 子进程执行失败时退出
        }
    } else { // 父进程：等待子进程结束
        int status;
        waitpid(pid, &status, 0); // 等待指定子进程
    }
}

// 处理内置命令（目前仅支持cd）
int handle_builtin(char **args) {
    if (args[0] == NULL) {
        return 1; // 空命令，继续循环
    }
    
    // 处理cd命令
    if (strcmp(args[0], "cd") == 0) {
        char *dir;
        // 如果没有指定目录，默认切换到HOME目录
        if (args[1] == NULL) {
            dir = getenv("HOME");
            if (dir == NULL) {
                fprintf(stderr, "cd: 未设置HOME环境变量\n");
                return 1;
            }
        } else {
            dir = args[1];
        }
        
        // 执行目录切换
        if (chdir(dir) != 0) {
            perror("cd失败");
        }
        return 1; // 内置命令处理完成，继续循环
    }
    
    // 处理退出命令
    if (strcmp(args[0], "exit") == 0) {
        printf("退出Shell...\n");
        return 0; // 返回0表示退出Shell
    }
    
    return -1; // 不是内置命令，需要执行外部程序
}

// 显示命令提示符（包含当前工作目录）
void print_prompt() {
    char cwd[1024];
    // 获取当前工作目录
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("[%s]$ ", cwd); // 提示符格式：[当前目录]$ 
    } else {
        perror("获取当前目录失败");
        printf("$ "); // 失败时显示简单提示符
    }
    fflush(stdout); // 刷新输出缓冲区
}

int main() {
    char cmd[MAX_CMD_LEN];  // 存储输入的命令
    char *args[MAX_ARGS];   // 存储解析后的参数
    int running = 1;        // 控制Shell主循环
    
    printf("简易Shell启动（支持cd和外部命令，输入exit退出）\n");
    
    while (running) {
        print_prompt(); // 显示命令提示符
        
        // 读取用户输入
        if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
            perror("读取命令失败");
            break;
        }
        
        // 解析命令
        int arg_count = parse_command(cmd, args);
        if (arg_count == 0) {
            continue; // 空命令，跳过
        }
        
        // 处理命令（内置命令或外部命令）
        int result = handle_builtin(args);
        if (result == 0) {
            running = 0; // 退出Shell
        } else if (result == -1) {
            execute_external(args); // 执行外部命令
        }
        // 若result == 1，继续下一次循环
    }
    
    return 0;
}

