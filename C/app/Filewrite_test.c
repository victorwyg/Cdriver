#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#define ON (1)
#define OFF (0)
#define PATH1 ("/sys/class/leds/Do13/brightness")/*右风扇*/
#define PATH2 ("/sys/class/leds/Do14/brightness")/*左风扇*/
#define PATH3 ("/sys/class/leds/Do15/brightness")/*右加热*/
#define PATH4 ("/sys/class/leds/Do16/brightness")/*左加热*/
#define PATH5 ("/root/cmd/start.txt")
#define TEMPOVER (Dev.temp_value[0] < 50 && Dev.temp_value[1] < 50 && Dev.temp_value[2] < 50 && Dev.temp_value[3] < 50 && Dev.temp_value[0] != (-1) && Dev.temp_value[0] != 0)
#define OP_CD ((Dev.old_mode_code != Dev.mode_code || Dev.old_run_time != Dev.run_time || Dev.File_last_time != Dev.file_last_time || Dev.LR_sta != Dev.old_LR_sta) && err_ret == 0 && Dev.mode_code!= 0/*正式需改为!=0*/)
#define STA_SAVE \
    do { \
        Dev.old_mode_code = Dev.mode_code; \
        Dev.old_LR_sta = Dev.LR_sta; \
        Dev.old_run_time = Dev.run_time; \
        Dev.File_last_time = Dev.file_last_time; \
    } while (0)



typedef struct{
    char mode_code;             /*模式码** 通风1 加热2 除螨3 烘被4 **/
    char old_mode_code;         /*旧模式码*/
    char fan_sta;               /*风扇状态标志*/
    char hot_sta;               /*加热状态标志*/

    char fan_err;               /*具体故障风扇，正常为0，DO13故障为1，DO14故障为2,全部故障为3　*/
    char hot_err;               /*具体故障加热丝，正常为0，DO15故障为1，DO16故障为2,全部故障为3　*/
    char temp_err;              /*err*温度传感器*/
    char tp_err[5];             /*具体故障传感器，0000正常，高位代表WD1故障即1000，WD2故障0100　*/

    int run_time;               /*设定运行周期*/
    int old_run_time;           /*旧运行周期*/
    int LR_sta;                 /*左0，右1，全开2*/
    int old_LR_sta;             /*旧左右码*/
    time_t file_last_time;      /*文件最后修改时间*/
    time_t File_last_time;      /*文件最后修改时间比对*/
    float temp_value[4];        /*温度传感器数值1\2\3\4*/ 
    float cp_temp_value[4];        /*记录温度传感器数值1\2\3\4*/
} Dev_Class;

Dev_Class Dev;                        /*创建设备对象*/



int main()
{
Dev.mode_code = 1;
sprintf(Dev.tp_err,"2222");
Dev.fan_err = 3;
Dev.hot_err = 4;
Dev.temp_value[0] =25.5;
Dev.temp_value[1] = 75.5;

    char sta_buf[20] = {0};
    sprintf(sta_buf, "%d%s%d%d,%.2f,%.2f", Dev.mode_code, Dev.tp_err,Dev.fan_err,Dev.hot_err,Dev.temp_value[0],Dev.temp_value[1]);
    printf("sta_buf value is %s\r\n",sta_buf);
    int fd = open("/home/wyg/linux/errlist.txt",O_WRONLY | O_CREAT,S_IRWXU | S_IRGRP);
    lseek(fd, 0,SEEK_SET);
    int ret = write(fd,sta_buf,(strlen(sta_buf)-1));
    if (ret < 0){
        printf("状态写入失败\r\n");
        }
    close(fd);
    
    return 0;
}
    