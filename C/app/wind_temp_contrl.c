#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define ON (1)
#define OFF (0)
#define PATH1 ("/sys/class/leds/Do13/brightness")
#define PATH2 ("/sys/class/leds/Do14/brightness")
#define PATH3 ("/sys/class/leds/Do15/brightness")
#define PATH4 ("/sys/class/leds/Do16/brightness")
#define TEMPOVER (Dev.temp_value[0] < 26 && Dev.temp_value[1] < 26 && Dev.temp_value[2] < 26 && Dev.temp_value[3] < 26 && Dev.temp_value[0] != (-1))



typedef struct{
    char mode_code;
    char mode_code_compare;
    char fan_sta;
    char hot_sta;
    int do_value[4];
    float temp_value[4];
    int run_time;
    int stop;
} dev;
dev Dev;

/******************基础操作函数********************/

static int fan_open(void){
    int fd = open(PATH1, O_WRONLY);
    if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", ON);
    close(fd);
    fd = open(PATH2, O_WRONLY);
    if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", ON);
    close(fd);
    return 0;
}
static int fan_close(void){
    int fd = open(PATH1, O_WRONLY);
    if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", OFF);
    close(fd);

    fd = open(PATH2, O_WRONLY);
    if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", OFF);
    close(fd);
    return 0;
}
static int hot_open(void){
    int fd = open(PATH3, O_WRONLY);
    if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", ON);
    close(fd);

    sleep(11);  /*间隔延时启动，大电流保护*/

    fd = open(PATH4, O_WRONLY);
    if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", ON);
    close(fd);
    return 0;
}
static int hot_close(void){
    int fd = open(PATH3, O_WRONLY);
    if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", OFF);
    close(fd);

    fd = open(PATH4, O_WRONLY);
    if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", OFF);
    close(fd);
    return 0;
}
static int temp_read(void){         /*传感器温度读取函数*/
    int fd;
    char buf[20] ={0};
    char dev_name[20];
    int readlen = 0;

    for(int i = 1;i < 5; i++){
        sprintf(dev_name, "/dev/ds18_%d",i);
        if ((fd = open(dev_name, O_RDWR | O_NDELAY | O_NOCTTY)) < 0) {
        return -1;
    }
    else {
        readlen = read(fd, buf, sizeof(buf));
        Dev.temp_value[i-1] = atof(buf);
        /*printf("%f\n", Dev.temp_value[i-1]);*/
        close(fd);
    }
    memset(buf, 0, sizeof(buf));
    }

return 0;
}
static int sta_read(void){          /*上位状态读取函数*/
        int fd = 0;
        char buffer[3] = {0};
        memset(buffer, 0, sizeof(buffer));
        fd = open("/root/cmd/start.txt",O_RDONLY);
        lseek(fd,0,SEEK_SET);
        read(fd,buffer,1);

        Dev.mode_code = atoi(buffer);
        /*printf("mode_code is %d\n",Dev.mode_code);*/
        close(fd);
        memset(buffer, 0, sizeof(buffer));
        
        struct stat st;
        if (stat("/root/cmd/start.txt", &st) < 0) {
        printf("Error: stat() failed.\n");
        return -1;
        }
        int size = st.st_size;

        fd = open("/root/cmd/start.txt",O_RDONLY);
        lseek(fd,1,SEEK_SET);
        read(fd,buffer,(size - 1));
        Dev.run_time = atoi(buffer);
        /*printf("run_time is %d\n",Dev.run_time);*/
        close(fd);
        return 0;
}
/*******************功能函数*********************/
static void ventilation_fuc(void){
    hot_close();
    int k = Dev.run_time;
    printf("进入通风模式\n");
    for (k; k > 0; k--)
        {
            sta_read();
            if (Dev.mode_code_compare != Dev.mode_code)
            break;
            sleep(1);
            if(Dev.fan_sta != 1)
            {
                fan_open();
                Dev.fan_sta = 1;
            }
            printf("通风模式倒计时%d\n",k);
        }
        fan_close();
        Dev.fan_sta = 0;
}
static void heating_fuc(){
        printf("进入加热模式，设定时间%d\n",Dev.run_time+11);
        int k = Dev.run_time;
        Dev.mode_code_compare = Dev.mode_code;
        fan_open();
        Dev.fan_sta = 1;
        for (k;k > 0; k--)
        {
            sta_read();
            if (Dev.mode_code_compare != Dev.mode_code)
            break;
            
            temp_read();
            if(TEMPOVER && Dev.hot_sta != 1)
            {
                hot_open();
                Dev.hot_sta = 1;
                }
            if(Dev.temp_value[0] > 30 && Dev.temp_value[0] != (-1))
            {
                hot_close();
                Dev.hot_sta = 0;
                }
        }
        hot_close();
        Dev.hot_sta = 0;
        sleep(5);
        fan_close();
        Dev.fan_sta = 0;
        printf("加热模式结束，即将退出\n");
    }
static void mitekill_fuc(void){
printf("进入除螨模式，设定时间%d\n",Dev.run_time+11);
        int k = Dev.run_time;
        Dev.mode_code_compare = Dev.mode_code;
        fan_open();
        Dev.fan_sta = 1;
        for (k;k > 0; k--)
        {
            sta_read();
            if (Dev.mode_code_compare != Dev.mode_code)
            break;
            
            temp_read();
            if(TEMPOVER && Dev.hot_sta != 1)
            {
                hot_open();
                Dev.hot_sta = 1;
                }
            if(Dev.temp_value[0] > 50 && Dev.temp_value[0] != (-1))
            {
                hot_close();
                Dev.hot_sta = 0;
                }
        }
        hot_close();
        Dev.hot_sta = 0;
        sleep(5);
        fan_close();
        Dev.fan_sta = 0;
        printf("除螨模式结束，即将退出\n");
}
static void drying_fuc(void){
    printf("进入烘焙模式，设定时间%d\n",Dev.run_time+11);
        int k = Dev.run_time;
        Dev.mode_code_compare = Dev.mode_code;
        fan_open();
        Dev.fan_sta = 1;
        for (k;k > 0; k--)
        {
            sta_read();
            if (Dev.mode_code_compare != Dev.mode_code)
            break;
            
            temp_read();
            if(TEMPOVER && Dev.hot_sta != 1)
            {
                hot_open();
                Dev.hot_sta = 1;
                }
            if(Dev.temp_value[0] > 60 && Dev.temp_value[0] != (-1))
            {
                hot_close();
                Dev.hot_sta = 0;
                }
        }
        hot_close();
        Dev.hot_sta = 0;
        sleep(5);
        fan_close();
        Dev.fan_sta = 0;
        printf("烘焙模式结束，即将退出\n");
}

int main(void){
    Dev.fan_sta = 0;
    Dev.hot_sta = 0;
    Dev.mode_code = 0;
    while(1){

        sta_read();

        switch (Dev.mode_code)
        {
        case 1:
            ventilation_fuc();
            break;
        case 2:
            heating_fuc();
            break;
        case 3:
            mitekill_fuc();
            break;
        case 4:
            drying_fuc();
            break;
        /*default:
            hot_close();
            fan_close();
            break;*/
        }
    }
    return 0;
}