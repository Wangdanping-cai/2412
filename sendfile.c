#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

int main(int argc,char *argv[]){
    int source_fd,dest_fd;
    struct stat stat_buf;
    off_t offset=0;

    //检查命令行参数（程序名 + 源文件路径 + 目标文件路
    if(argc!=3)
    {
       fprintf(stderr,"用法：%s<源文件><目标文件>\n");
       exit(EXIT_FAILURE);
    }
    
    //打开源文件（只读
    source_fd=open(argv[1],O_RDONLY);
    if(source_fd==-1)
    {
       perror("无法打开源文件");
       exit(EXIT_FAILURE);
    }
    //获取源文件信息
    if(fstat(source_fd,&stat_buf)==-1)
    {
        perror("无法获取文件信息");
        close(source_fd);
        exit(EXIT_FAILURE);
    }
    //打开目标文件（创建、只写、截断模式
    //权限设置为: 所有者读写，组可读，其他人可读
    dest_fd=open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(dest_fd==-1)
    {
        perror("无法创建目标文件");
        close(source_fd);
        exit(EXIT_FAILURE);
    }
    //使用sendfile拷贝文件内容
    //sendfile参数：目标文件描述符，源文件描述符，偏移量指针，传输的字节数
    ssize_t bytes_sent=sendfile(dest_fd,source_fd,&offset,stat_buf.st_size);
    if(bytes_sent==-1)
    {
        perror("文件拷贝失败");
        close(source_fd);
        close(dest_fd);
        exit(EXIT_FAILURE);
    }
    //验证是否拷贝成功
      if (bytes_sent != stat_buf.st_size) {
        fprintf(stderr, "警告: 只拷贝了 %zd 字节，预期 %lld 字节\n",
                bytes_sent, (long long)stat_buf.st_size);
    } else {
        printf("文件拷贝成功: %s -> %s\n", argv[1], argv[2]);
        printf("拷贝大小: %lld 字节\n", (long long)stat_buf.st_size);
    }

    // 关闭文件描述符
    close(source_fd);
    close(dest_fd);

    return 0;
}
