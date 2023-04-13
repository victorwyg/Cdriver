#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <unistd.h>

#define S 1000000
int main(int argc, char *argv[]){
    int fd, ret1;
    unsigned char onbuf[2] ;
    unsigned char offbuf[2];
    onbuf[0] = 1;
    offbuf[0] = 0;
    unsigned char databuf[2];
    if(argc != 2){
        printf("Error Usage!\r\n");
        return -1;
        }
    fd = open("/dev/beep", O_RDWR);
    if(fd < 0){
    printf("file open failed!\r\n");
    return -1;
    }
    if(sizeof(argv[1]) < 5){    /* 传入执行的操作*/
        strcpy(databuf, argv[1]);
        } 
    else{
        printf("Error ARGV Usage!%d\r\n",sizeof(argv[1]));
    return -1;
    }
   if(!(strcmp(databuf, "E1"))){  /*字符串比较函数*/
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    printf("write E1 OK\r\n");
    if(ret1 < 0){
        printf("Control E1 Failed!\r\n");
        close(fd);
        return -1;
        }
   }
    if(!(strcmp(databuf, "E2"))){  /*字符串比较函数*/
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep(S);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    printf("write ER2 OK\r\n");
    if(ret1 < 0){
        printf("Control E2 Failed!\r\n");
        close(fd);
        return -1;
        }
   }
   if(!(strcmp(databuf, "E3"))){  /*字符串比较函数*/
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep(S);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    printf("write ER3 OK\r\n");
    if(ret1 < 0){
        printf("Control E3 Failed!\r\n");
        close(fd);
        return -1;
        }
   }
    if(!(strcmp(databuf, "E4"))){  /*字符串比较函数*/
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep((S)/2);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep(S);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    usleep((S)/2);
    ret1 = write(fd, onbuf, sizeof(onbuf));
    usleep(S);
    ret1 = write(fd, offbuf, sizeof(offbuf));
    printf("write ER4 OK\r\n");
    if(ret1 < 0){
        printf("Control E4 Failed!\r\n");
        close(fd);
        return -1;
        }
   }
    if(ret1 < 0){
        printf("Control erro2 Failed!\r\n");
        close(fd);
        return -1;
        }
    ret1 = close(fd); /* 关闭文件 */
    if(ret1 < 0){
    printf("file close failed!\r\n");
    return -1;
    }
    return 0;
}