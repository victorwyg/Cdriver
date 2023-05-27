#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#define ON (1)
#define OFF (0)
#define PATH1 ("/sys/class/leds/Do13/brightness")/*右风扇*/
#define PATH2 ("/sys/class/leds/Do14/brightness")/*左风扇*/
#define PATH3 ("/sys/class/leds/Do15/brightness")/*右加热*/
#define PATH4 ("/sys/class/leds/Do16/brightness")/*左加热*/
#define PATH5 ("/root/cmd/start.txt")
#define TEMPOVER (Dev.temp_value[0] < 35 && Dev.temp_value[1] < 35 && Dev.temp_value[2] < 35 && Dev.temp_value[3] < 35 && Dev.temp_value[0] != (-1) && Dev.temp_value[0] != 0)



typedef struct{
    char mode_code;             /*模式码** 通风1 加热2 除螨3 烘被4 **/
    char old_mode_code;         /*旧模式码*/
    char mode_code_compare;     /*比对模式码*/
    char fan_sta;               /*风扇状态标志*/
    char hot_sta;               /*加热状态标志*/
    int run_time;               /*设定运行周期*/
    int old_run_time;           /*旧运行周期*/
    int LR_sta;                 /*左0，右1，全开2*/
    int old_LP_sta;             /*旧左右码*/
    time_t file_last_time;      /*文件最后修改时间*/
    time_t File_last_time;      /*文件最后修改时间比对*/
    float temp_value[4];        /*温度传感器数值1\2\3\4*/
} Dev_Class;

Dev_Class Dev;                        /*创建设备对象*/

/******************基础操作函数********************/

static int fan_open(void){
    int fd;
    if(Dev.LR_sta == 2){
        fd = open(PATH1, O_WRONLY);
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
    }
    /*** 左右单独 ***/
    if(Dev.LR_sta == 0){
        int fd = open(PATH2, O_WRONLY);
        if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", ON);
    close(fd);
    }
    if(Dev.LR_sta == 1){
        int fd = open(PATH1, O_WRONLY);
        if (fd < 0) {
        exit(1);
    }
    dprintf(fd, "%d", ON);
    close(fd);
    }
    
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
    int fd;
    if(Dev.LR_sta == 2){fd = open(PATH3, O_WRONLY);
        if (fd < 0) {
        return -1;
        }
    dprintf(fd, "%d", ON);
    close(fd);
    sleep(5);  /*间隔延时启动，大电流保护*/
    fd = open(PATH4, O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    dprintf(fd, "%d", ON);
    close(fd);
    }
    if(Dev.LR_sta == 1){fd = open(PATH3, O_WRONLY);
        if (fd < 0) {
        return -1;
        }
    dprintf(fd, "%d", ON);
    close(fd);
    }
    if(Dev.LR_sta == 0){
        fd = open(PATH4, O_WRONLY);
        if (fd < 0) {
        return -1;
    }
    dprintf(fd, "%d", ON);
    close(fd);
    }
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
        }else {
        readlen = read(fd, buf, sizeof(buf));
        Dev.temp_value[i-1] = atof(buf);
        /*printf("temp%d is %f\n",i, Dev.temp_value[i-1]);*/
        close(fd);
    }
    memset(buf, 0, sizeof(buf));
    }

return 0;
}
static int sta_read(void){          /*上位状态读取函数*/
        int fd = 0;
        int ret;
        char buffer[3] = {0};
        memset(buffer, 0, sizeof(buffer));
        fd = open(PATH5,O_RDONLY);
        if(fd < 0){
            return -1;
        }

        lseek(fd,0,SEEK_SET);

        ret = read(fd,buffer,1);
        if(0 > ret){
            return -1;
        }
        Dev.mode_code = atoi(buffer);
        /*printf("mode_code is %d\n",Dev.mode_code);*/
        memset(buffer, 0, sizeof(buffer));
        
        lseek(fd,1,SEEK_SET);
        ret = read(fd,buffer,1);
        if(0 > ret){
            return -1;
        }
        Dev.LR_sta = atoi(buffer);
        /*printf("LR_sta is %d\n",Dev.LR_sta);*/
        close(fd);
        memset(buffer, 0, sizeof(buffer));
        
        struct stat st;
        if (stat(PATH5, &st) < 0) {
        printf("Error: stat() failed.\n");
        return -1;
        }
        int size = st.st_size;/*保存文件内容，字节总长度*/

        time_t modTime = st.st_mtime;
        /*printf("Last modified time: %ld\n", modTime);*/
        Dev.file_last_time = modTime;
        
        /*struct tm* timeInfo = localtime(&modTime);
        char modTimeStr[21];
        strftime(modTimeStr, sizeof(modTimeStr), "%Y-%m-%d %H:%M:%S", timeInfo);
        
        strncpy(Dev.file_last_time,modTimeStr,sizeof(modTimeStr));/*保存文件最后修改时间
        printf("Last modified time: %s\n", modTimeStr);*/


        fd = open("/root/cmd/start.txt",O_RDONLY);
        if(fd < 0){
            return -1;
        }
        lseek(fd,2,SEEK_SET);
        ret = read(fd,buffer,(size - 1));
        if(ret < 0){
            return -1;
        }
        Dev.run_time = atoi(buffer);
        /*printf("run_time is %d\n",Dev.run_time);*/
        close(fd);
        return 0;
}
static int sta_push(void){          /*运行状态推送函数*/

}
/*******************功能函数*********************/
static int ventilation_fuc(){
    hot_close();
    printf("进入通风模式\n");
    int k = Dev.run_time;
    Dev.mode_code_compare = Dev.mode_code;
    Dev.old_LP_sta = Dev.LR_sta;
    fan_open();
    Dev.fan_sta = 1;
    for (k;k > 0; k--)
        {
            
            sta_read();
            temp_read();
            if (Dev.mode_code_compare != Dev.mode_code || Dev.old_LP_sta != Dev.LR_sta)
            return -1;
            /*clock_t start = clock();
            temp_read();
            clock_t end = clock();
            double runtime = (double)(end - start) / CLOCKS_PER_SEC;
            printf("temp_read runtime %fS\r\n",runtime);*/
            sleep(1);
        }
        
    fan_close();
    Dev.fan_sta = 0;
    printf("通风模式结束，即将退出\n");
    
}
static int heating_fuc(){
        printf("进入加热模式，设定周期%d\n",Dev.run_time+11);
        int k = Dev.run_time;
        Dev.mode_code_compare = Dev.mode_code;
        fan_open();
        Dev.fan_sta = 1;
        for (k;k > 0; k--)
        {
            temp_read();
            sta_read();
            if (Dev.mode_code_compare != Dev.mode_code)
            return -1;

            if(TEMPOVER && Dev.hot_sta != 1)
            {
                hot_open();
                Dev.hot_sta = 1;
                }
            if(Dev.temp_value[0] > 50 || Dev.temp_value[1] > 50 && Dev.temp_value[0] != (-1))
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
        Dev.old_mode_code = Dev.mode_code;
        Dev.old_run_time = Dev.run_time;
        printf("F is %d\nf is %d\n",Dev.old_mode_code,Dev.old_run_time);
        Dev.File_last_time = Dev.file_last_time;
        printf("F is %ld\nf is %ld\n",Dev.File_last_time,Dev.file_last_time);
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
    printf("进入烘被模式，设定时间%d\n",Dev.run_time+11);
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
        printf("烘被模式结束，即将退出\n");
}

int main(void){
    int ret = 0;
    char command[32];
    Dev.fan_sta = 0;
    Dev.hot_sta = 0;
    Dev.mode_code = 0;
    Dev.old_mode_code = 0;
    Dev.old_run_time = 0;
    Dev.file_last_time = 0;
    Dev.File_last_time = 1;
    /*for (size_t i = 1; i < 5; i++)
    {
        sprintf(command, "insmod /home/temp%d.ko", i);
        ret = system(command); // 使用system()函数执行命令
        
    if (ret != 0) {
        printf("执行命令失败！\n");
        return -1;
    }else{
        printf("命令执行成功！\n");
        }
    }
    ret = system("insmod /home/beep_driver.ko"); // 使用system()函数执行命令
        
    if (ret != 0) {
        printf("执行命令失败！\n");
        return -1;
    }else{
        printf("命令执行成功！\n");
        }
    */
    
    
    
    
    while(1){
        sleep(1);
        ret = sta_read();       /*读取状态修改*/
        if(0 > ret){
            /*告知上位机状态读取异常*/
        }
        
        
        if(Dev.old_mode_code != Dev.mode_code || Dev.old_run_time != Dev.run_time || Dev.File_last_time != Dev.file_last_time)
        {
        switch (Dev.mode_code)
        {
            case 1:
                ret = ventilation_fuc();
                break;
            case 2:
                ret = heating_fuc();
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
            Dev.old_mode_code = Dev.mode_code;
            Dev.old_run_time = Dev.run_time;
            Dev.File_last_time = Dev.file_last_time;
            if (0 > ret){
            Dev.File_last_time = 1;
        }
        }else{
            printf("File time not change,Waiting for command\r\n");
        }
        
    }
    return 0;
}