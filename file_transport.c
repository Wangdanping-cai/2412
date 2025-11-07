#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include<sys/types.h>
#define LEN 1024*4

int main()
{
    int fd[2];
    pipe(fd);
    char buf[LEN];
    int num=0;

    pid_t pid= fork(); 
    if(pid>0)
    {
        int src_fd=open("src.txt",O_RDONLY);
        close(fd[0]);
        while((num=read(src_fd,buf,LEN))>0)
        {
            write(fd[1],buf,num);
        }
        close(fd[1]);
        wait(NULL);
    }
    else if(pid==0)
    {
        close(fd[1]);
        int dest_fd = open("dest.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);   
        while((num=read(fd[0],buf,LEN))>0)
        {
            write(dest_fd,buf,num);
        }
    }
    else
    {
        printf("creat error");
    }
    return 0;
}
            
