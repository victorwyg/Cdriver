#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define ON (1)
#define OFF (0)
#define PATH1 ("/sys/class/leds/Do13/brightness")
#define PATH2 ("/sys/class/leds/Do14/brightness")
#define PATH3 ("/sys/class/leds/Do15/brightness")
#define PATH4 ("/sys/class/leds/Do16/brightness")



typedef struct{
    int do_value[4];
    float temp_value[5];
    int run_time;
    int mode_code;
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

    sleep(11);  /*间隔延时启动*/

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
static float temp_read(int ds18_num,char *result){
    int fd;
    char buf[6] ={0};
    char dev_name[20];
    int readlen = 0;
    switch(ds18_num) {
        case 1:
            sprintf(dev_name, "/dev/ds18_1");
            break;
        case 2:
            sprintf(dev_name, "/dev/ds18_2");
            break;
        case 3:
            sprintf(dev_name, "/dev/ds18_3");
            break;
        case 4:
            sprintf(dev_name, "/dev/ds18_4");
            break;
        default:
            printf("Invalid sensor number.\r\n");
            return -1;
    }
    if ((fd = open(dev_name, O_RDWR | O_NDELAY | O_NOCTTY)) < 0) {
        return -1;
    }
    else {
        readlen = read(fd, buf, sizeof(buf));
        usleep(2000);
        close(fd);
    }
    strcpy(result,buf);
    return atof(result);
}

/*******************功能函数*********************/
static void ventilation_fuc(int time){
    
    printf("进入通风模式\n");
    


}
static void heating_fuc(){
    printf("***进入加热模式***\n");
    if(Dev.temp_value[0] < 20 && Dev.temp_value[0] != (-1))
    fan_open();
    sleep(1);
    hot_open();
    printf("***加热启动完成***\n");
    sleep(Dev.run_time);
}
static void mitekill_fuc(int time){

}
static void drying_fuc(int time){

}

int main(void){
    
    int flag = 0;
    int ds18 = 0;   /*传感器选择*/
    int i = 1;
    char *result = malloc(20);
    
    
    while(1){
        ds18 = 1;
        Dev.run_time = 20;
        for(i;i < 5; i++){
            Dev.temp_value[i-1] = temp_read(i,result);
            sleep(0.01);
        }
        for(i;i < 6; i++){
            heating_fuc(Dev.run_time);
            sleep(0.01);
        }
        flag++;
        sleep(1);
        printf("i is %d,flag is %d\n",i,flag);
        if(flag == Dev.run_time)
        break;
    }


    hot_close();
    fan_close();
    memset(result, 0, sizeof(result));
    free(result);
    return 0;
}