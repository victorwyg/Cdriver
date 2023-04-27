#include <stdio.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <string.h>

 int main(int argc, char *argv[])
 {
     int fd;
     char buf[20]={0};
     char dev_name[20];
     float result;
     int readlen = 0;
     

    for(int i = 1;i < 5; i++){
        sprintf(dev_name, "/dev/ds18_%d", i);
        if ((fd=open(dev_name,O_RDWR | O_NDELAY | O_NOCTTY)) < 0) {
         printf("Open Device Ds18b20_%d Failed.\r\n",i);
         return -1;
     }
     else
     {
         printf("Open Device Ds18b20_%d Successed.\r\n",i);
         
            for(int i = 0;i < 5; i++){
                readlen = read(fd, buf, sizeof(buf));
             printf("%s", buf);
             usleep(500000);
            }
         close(fd);
     }
     memset(buf, 0, sizeof(buf));
    }
    return 0;
 }