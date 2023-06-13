#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

// 蜂鸣器类
typedef struct {
    int duration;  // 响声的时间长短，0代表响0.5秒，1代表响1秒
    int fd;
    
} Buzzer;

// 创建蜂鸣器对象
Buzzer buzzer;



// 初始化蜂鸣器
/*void Buzzer_init(Buzzer *buzzer, int duration) {
    buzzer->duration = duration;
}*/

// 发出响声
int Buzzer_buzz(Buzzer *buzzer) {
    int ret = 0;

    unsigned char onbuf[2] ;
    unsigned char offbuf[2];
    onbuf[0] = 1;
    offbuf[0] = 0;
    
    
    buzzer->fd = open("/dev/beep", O_RDWR);
    if(buzzer->fd < 0){
        printf("file open failed!\r\n");
        return -1;
    }
    if (buzzer->duration == 0) {                         /*间隔0.5秒*/
     printf("0.5S\r\n");
        ret = write(buzzer->fd, onbuf, sizeof(onbuf));
        if(0 > ret){
            close(buzzer->fd);
            printf("write failed!\r\n");
            return -1;
        }

        usleep(500000);  // 延时0.5秒
        ret = write(buzzer->fd, offbuf, sizeof(offbuf));
        usleep(300000);
        if(0 > ret){
            close(buzzer->fd);
            printf("write failed!\r\n");
            return -1;
        }
    }

    if (buzzer->duration == 1) {                        /*间隔1秒*/
        printf("1S\r\n");
       ret = write(buzzer->fd, onbuf, sizeof(onbuf));
        if(0 > ret){
            close(buzzer->fd);
            printf("write failed!\r\n");
            return -1;
        }

        usleep(1000000);  // 延时1秒
        ret = write(buzzer->fd, offbuf, sizeof(offbuf));
        usleep(300000);
        if(0 > ret){
            close(buzzer->fd);
            printf("write failed!\r\n");
            return -1;
        }
        
    }
    ret = close(buzzer->fd); /* 关闭文件 */
    if(ret < 0){
        printf("file close failed!\r\n");
        return -1;
    }
    
    return 0;
}



int main(int argc, char *argv[]){
    int input = 0;;
    unsigned char databuf[2];
    unsigned char onbuf[2] ;
    unsigned char offbuf[2];
    onbuf[0] = 1;
    offbuf[0] = 0;

    if(argc != 2){
        printf("Error Usage!\r\n");
        return 0;
        }

    if(sizeof(argv[1]) < 5){    /* 传入执行的操作*/
        databuf[0] = atoi(argv[1]);
        } else{
        printf("Error ARGV Usage!%d\r\n",sizeof(argv[1]));
        return 0;
    }
    input = databuf[0];

    if (input < 1 || input > 17) {
        printf("输入无效!请输入1-17之间的数字。\n");
        return 0;
    }
    if(input == 17){
        int fd = open("/dev/beep", O_RDWR);
        write(fd, onbuf,  sizeof(onbuf));
        usleep(1500000);  // 响2S
        write(fd, offbuf, sizeof(offbuf));
        close(fd);
        goto finish;
    }
    int binary[4];
    for (int i = 0; i < 4; i++) {

        binary[i] = (input >> (3 - i)) & 1;

        buzzer.duration = binary[i];
        /*printf("duration value is%d:\n", buzzer.duration);*/
        Buzzer_buzz(&buzzer);
    }

    finish:
    printf("finish\r\n");
    return 0;

}