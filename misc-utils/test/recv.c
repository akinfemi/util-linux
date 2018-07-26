#include <signal.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

void sig_handler(int signo)
{
    mqd_t mq = mq_open("/fkillq", O_RDONLY);
    if (mq == (mqd_t) -1){
        printf("mq_open failed.");
        perror(strerror(errno));
        return ;
    }
    struct mq_attr attr;
    char *buff;
    unsigned int priority;

    if(mq_getattr(mq, &attr) == -1)
    {
        printf("getattr error\n");
    }
    buff = malloc(attr.mq_msgsize);
    long bytesRd = mq_receive(mq, buff, attr.mq_msgsize, &priority);
    if (bytesRd == -1){
        printf("receive error\n");
    }else{
        printf("%s\n", buff);
    }
    if(signo == SIGKILL){
        printf("SIGKILL received.\n");
        exit(1);
    }
}

void shm_sig_handler(int signo)
{
   int fd;
   char *addr;
   struct stat sb;

   fd = shm_open("fkill_shm", O_RDONLY, 0);
   if (fd == -1){
       printf("shm_open fail\n");
       perror(strerror(errno));
       return ;
   }
   if (fstat(fd, &sb) == -1){
       printf("fstat fail\n");
       perror(strerror(errno));
       return ;
   }
   addr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (addr == MAP_FAILED){
       printf("mmap failed\n");
       return ;
   }
   if(signo == SIGKILL){
	   printf("SIGKILL received.\n");
	   exit(1);
   }
   write(1, addr, sb.st_size);
   write(1, "\n", 1);
}

int main(void)
{
    if(signal(SIGUSR1, sig_handler) == SIG_ERR){
        printf("Can't catch SIGUSR1\n");
    }
    if(signal(SIGUSR2, shm_sig_handler) == SIG_ERR){
        printf("Can't catch SIGUSR2\n");
    }
    while (1)
    {
    	sleep(1);
    }	
    return 0;
}
