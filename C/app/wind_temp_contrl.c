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
#define TEMPOK (Dev.temp_value[0] > 50 && Dev.temp_value[1] > 50 && Dev.temp_value[2] > 50 && Dev.temp_value[3] > 50 && Dev.temp_value[0] != (-1) && Dev.temp_value[0] != 0)
#define OP_CD ((Dev.old_mode_code != Dev.mode_code || Dev.old_run_time != Dev.run_time || Dev.File_last_time != Dev.file_last_time || Dev.LR_sta != Dev.old_LR_sta) && (Dev.result == 12 || Dev.result == 8 || Dev.result < 5) && Dev.mode_code!= 0/*正式需改为!=0*/)
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
    int result;

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


/******************状态函数********************/
static char temp_read(void){         /*传感器温度读取函数*/
    int fd;
    char buf[20] ={0};
    char dev_name[20];
    int readlen = 0;
    char v = 0;
    char errt = 0;
    for(char i = 1;i < 5; i++){
        sprintf(dev_name, "/dev/ds18_%d",i);
        if((fd = open(dev_name, O_RDWR | O_NDELAY | O_NOCTTY)) < 0) {
            Dev.temp_value[i-1] = 0;
            printf("open /dev/ds18_%d failed",i);
        }else{
            readlen = read(fd, buf, sizeof(buf));
            /*if(atof(buf) <= 100)*/
                Dev.temp_value[i-1] = atof(buf);
            /*else{
                Dev.temp_value[i-1] = 110;
            }*/
            memset(buf, 0, sizeof(buf));
            close(fd);
            }
        if(Dev.temp_value[i-1] >= 110){
            Dev.temp_value[i-1] = 99;
        }
        printf("temp%d is %f\n",i, Dev.temp_value[i-1]);
        }
    for(char i = 1;i < 5; i++){
        if(Dev.temp_value[i-1] == 99 || (Dev.temp_value[i-1] < -15) || Dev.temp_value[i-1] == 0){
            errt++;
        }
        v++;
        if((v % 4) == 0 && errt!=0){
            printf("goto cmand\r\n");
            goto ret;    
            }
        }
    fflush(stdout);
return 0;
ret:
return errt;
}
static int sta_read(void){          /*上位状态读取函数*/
        int fd = 0;
        int ret;
        char buffer[6] = {0};
        
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
        
        /*memset(buffer, 0, sizeof(buffer));*/
        fd = open(PATH5,O_RDONLY);
        if(fd < 0)  return -1;
        
        lseek(fd,0,SEEK_SET);
        ret = read(fd,buffer,1);
        if(0 > ret) return -1;
        if(Dev.File_last_time != Dev.file_last_time )
        Dev.mode_code = atoi(buffer);
        /*printf("mode_code is %d\n",Dev.mode_code);*/
        memset(buffer, 0, sizeof(buffer));
        
        lseek(fd,1,SEEK_SET);
        ret = read(fd,buffer,1);
        if(0 > ret){
            return -1;
        }
        if(Dev.File_last_time != Dev.file_last_time ){
            Dev.LR_sta = atoi(buffer);
        }
        /*printf("LR_sta is %d\n",Dev.LR_sta);*/
        memset(buffer, 0, sizeof(buffer));

        lseek(fd,2,SEEK_SET);
        ret = read(fd,buffer,(size-2));/*总长度除去模式码和左右标志两个字节*/
        if(ret < 0){
            return -1;
        }
        if(Dev.File_last_time != Dev.file_last_time ){
            Dev.run_time = atoi(buffer);
        }
        memset(buffer, 0, sizeof(buffer));
        /*printf("run_time set value is %d\n",Dev.run_time);*/
        close(fd);
        return 0;
}
int sta_push(void){                 /*运行状态推送函数*/
    char sta_buf[20] = {0};
    sprintf(sta_buf, "%d%s%d%d,%.2f,%.2f", Dev.mode_code, Dev.tp_err,Dev.fan_err,Dev.hot_err,Dev.temp_value[0],Dev.temp_value[1]);
    printf("sta_buf value is %s\r\n",sta_buf);
    int fd = open("/root/cmd/errlist.txt",O_WRONLY | O_CREAT,S_IRWXU | S_IRGRP);
    lseek(fd, 0,SEEK_SET);
    int ret = write(fd,sta_buf,(strlen(sta_buf)-1));
    if (ret < 0){
        printf("状态写入失败\r\n");
        }
    close(fd);
    
    return 0;
}
static int err_jud(void){           /*错误判断函数*/
    printf("errjud fanhot is %d,%d\r\n",Dev.fan_err,Dev.hot_err);
    int err_ret = 0;
    Dev.temp_err = 0;
    Dev.result = 0;
    char arr[4] = {0};
    int temp_ret = temp_read();
    printf("temp_ret is%d\r\n",temp_ret);
    if(temp_ret > 0){
        for (int i = 0; i < 4; i++){
        if(err_ret = (Dev.temp_value[i] <= 0) || (Dev.temp_value[i] > 100) || (Dev.temp_value[i] == 99)){
            printf("第%d个温感异常\r\n",i+1);
            arr[i] = 1;
            Dev.temp_err += arr[i];
            Dev.result = Dev.result | (arr[i] << i);
            }
            
        sprintf(Dev.tp_err,"%d%d%d%d",arr[0],arr[1],arr[2],arr[3]);
        
        printf("故障传感器 %s,%d\r\n",Dev.tp_err,Dev.result);
        }
    }else{
        
        for (int i = 0; i < 4; i++){
            arr[i] = 0;
            }
            sprintf(Dev.tp_err,"%d%d%d%d",arr[0],arr[1],arr[2],arr[3]);
    }
    
    return err_ret;
}
static int fanhot_jud(){
    printf("风扇加热判断开始\r\n");
    Dev.hot_err = 0;
    Dev.fan_err = 0;
    float retbuf[4] = {0};
        if(Dev.result == 0){
        for (size_t i = 0; i < 4; i++){
            retbuf[i] = Dev.temp_value[i] - Dev.cp_temp_value[i];
            printf("retbuf[%d]%f\r\n",i,retbuf[i]);
        }
        if (retbuf[0] < 0.8 && retbuf[2] < 0.8 && retbuf[1] < 0.8 && retbuf[3] < 0.8){
                printf("两侧加热均失效(DO15 DO16)\r\n");
                Dev.hot_err = 3;
                
            }
        else if(retbuf[0] > 0.8 && retbuf[2] > 0.8 && retbuf[1] > 0.8 && retbuf[3] > 0.8 || TEMPOK){
                printf("两侧加热均正常(DO15 DO16)\r\n");
                Dev.hot_err = 0;
            }

        else if (retbuf[0] < 0.8 && retbuf[2] < 0.8){
                printf("右侧加热失效(DO15)\r\n");
                Dev.hot_err = 1;
            }
        else if (retbuf[1] < 0.8 && retbuf[3] < 0.8){
                printf("左侧加热失效(DO16)\r\n");
                Dev.hot_err = 2;
            }
        
        if(Dev.hot_err == 0){
            if ((retbuf[2] - retbuf[0]) >= 15 && (retbuf[3] - retbuf[1]) >= 15){
                printf("两侧风扇均失效(DO15 DO16)\r\n");
                Dev.fan_err = 3;
            }
            else if((retbuf[2] - retbuf[0]) <= 15 && (retbuf[3] - retbuf[1]) <= 15){
                printf("两侧风扇均正常(DO15 DO16)\r\n");
                Dev.fan_err = 0;
            }
            else if ((retbuf[2] - retbuf[0]) >= 15){
                printf("右侧风扇失效(DO15)\r\n");
                Dev.fan_err = 1;
                }
            else if ((retbuf[3] - retbuf[1]) >= 15){
                printf("左侧风扇失效(DO16)\r\n");
                Dev.fan_err = 2;
            }
            if(Dev.fan_err == 0 || TEMPOK){
            printf("风扇正常\r\n");
            }
        }
    
    if(Dev.hot_err == 0){
    printf("加热正常\r\n");
    }}
    
    return 0;
}
/*******************功能函数*********************/
static int ventilation_fuc(){
    hot_close();
    printf("进入通风模式设定周期%d\n",Dev.run_time);
    sta_push();
    int k = 1;

    Dev.old_mode_code = Dev.mode_code;          /*同步数值*/
    Dev.old_LR_sta = Dev.LR_sta;
    Dev.old_run_time = Dev.run_time;
    Dev.File_last_time = Dev.file_last_time;

    fan_open();
    Dev.fan_sta = 1;
    for ( k; Dev.run_time > k; k++){
        sta_read();                         /*读取最新状态指令，供中断判断*/
        if (Dev.old_mode_code != Dev.mode_code || Dev.old_LR_sta != Dev.LR_sta || Dev.run_time != Dev.old_run_time || Dev.temp_err > 0){
            fan_close();
            Dev.fan_sta = 0;
            return -1;
            }
        temp_read();
            /*clock_t start = clock();
            temp_read();
            clock_t end = clock();
            double runtime = (double)(end - start) / CLOCKS_PER_SEC;
            printf("temp_read runtime %fS\r\n",runtime);*/
        sleep(1);
        printf("通风模式循环周期剩余%d\r\n",Dev.run_time-k);
        err_jud();
        sta_push();
    }
    fan_close();
    Dev.fan_sta = 0;
    printf("通风模式结束，即将退出\n");
}
static int heating_fuc(){
        printf("进入加热模式，设定周期%d\n",Dev.run_time);
        sta_push();
        int k,x = 0;
        for(int i = 0; i < 4; i++){
            Dev.cp_temp_value[i] = Dev.temp_value[i];
        }
        fan_open();
        Dev.fan_sta = 1;
        STA_SAVE;
        for (k;Dev.run_time > k; k++){
            x += 1;
            temp_read();
            sta_read();
            if (Dev.old_mode_code != Dev.mode_code || Dev.old_LR_sta != Dev.LR_sta || Dev.run_time != Dev.old_run_time){
                hot_close();
                Dev.hot_sta = 0;
                return -1;
            }

            if(TEMPOVER && Dev.hot_sta != 1)
            {
                hot_open();
                Dev.hot_sta = 1;
                }
            if(Dev.temp_value[0] > 60 || Dev.temp_value[1] > 60 || Dev.temp_value[3] > 80 || Dev.temp_value[4] > 80 && Dev.temp_value[0] != (-1))
            {
                hot_close();
                Dev.hot_sta = 0;
                }
            
            printf("加热循环周期剩余%d\r\n",Dev.run_time-k);
            err_jud();
            sta_push();
            sleep(1);
            if((x % 25 == 0) && x <= 75 && TEMPOVER){/*在第25个周期执行，出风口温度上升幅度判断*/
            fanhot_jud();
            }
        }
        hot_close();
        Dev.hot_sta = 0;
        sleep(5);
        fan_close();
        Dev.fan_sta = 0;
        printf("加热模式结束，即将退出\n");
    }
static int mitekill_fuc(void){
        printf("进入除螨模式，设定时间%d\n",Dev.run_time+11);
        sta_push();
        int k,x = 0;
        for(int i = 0; i < 4; i++){
            Dev.cp_temp_value[i] = Dev.temp_value[i];
        }
        fan_open();
        Dev.fan_sta = 1;
        STA_SAVE;
        for (k;Dev.run_time > k; k++){
            x += 1;
            temp_read();
            sta_read();
            if (Dev.old_mode_code != Dev.mode_code || Dev.old_LR_sta != Dev.LR_sta || Dev.run_time != Dev.old_run_time){
                hot_close();
                Dev.hot_sta = 0;
                return -1;
            }

            if(TEMPOVER && Dev.hot_sta != 1)
            {
                hot_open();
                Dev.hot_sta = 1;
                }
            if(Dev.temp_value[0] > 70 || Dev.temp_value[1] > 70 || Dev.temp_value[3] > 85 || Dev.temp_value[4] > 85 && Dev.temp_value[0] != (-1))
            {
                hot_close();
                Dev.hot_sta = 0;
                }
            
            printf("除螨循环周期剩余%d\r\n",Dev.run_time-k);
            err_jud();
            sta_push();
            sleep(1);
            if((x % 25 == 0) && x<= 75 && TEMPOVER)/*在第三个周期执行，出风口温度上升幅度判断*/
            fanhot_jud();
        }
        hot_close();
        Dev.hot_sta = 0;
        sleep(5);
        fan_close();
        Dev.fan_sta = 0;
        printf("除螨模式结束，即将退出\n");
}
static int drying_fuc(void){
    printf("进入烘被模式，设定时间%d\n",Dev.run_time+2);
    sta_push();
    int k,x = 0;
    for(int i = 0; i < 4; i++){
        Dev.cp_temp_value[i] = Dev.temp_value[i];
    }
    fan_open();
    Dev.fan_sta = 1;
    STA_SAVE;
    for (k;Dev.run_time > k; k++){
        x += 1;
        temp_read();
        sta_read();
        if (Dev.old_mode_code != Dev.mode_code || Dev.old_LR_sta != Dev.LR_sta || Dev.run_time != Dev.old_run_time){
            hot_close();
            Dev.hot_sta = 0;
            return -1;
        }

            if(TEMPOVER && Dev.hot_sta != 1)
            {
                hot_open();
                Dev.hot_sta = 1;
                }
            if(Dev.temp_value[0] > 85 || Dev.temp_value[1] > 85 || Dev.temp_value[3] > 90 || Dev.temp_value[4] > 90 && Dev.temp_value[0] != (-1))
            {
                hot_close();
                Dev.hot_sta = 0;
                }
            
            printf("烘被循环周期剩余%d\r\n",Dev.run_time-k);
            err_jud();
            sta_push();
            sleep(1);
            if((x % 25 == 0) && x<= 75 && TEMPOVER)/*在第三个周期执行，出风口温度上升幅度判断*/
            fanhot_jud();
        }
        hot_close();
        Dev.hot_sta = 0;
        sleep(5);
        fan_close();
        Dev.fan_sta = 0;
        printf("烘被模式结束，即将退出\n");
}

void handle_interrupt(int signum) {
    // 在这里执行需要在中断时执行的逻辑
    printf("程序被中断\n");
    hot_close();
    fan_close();
    exit(1);
    // 可以在这里进行清理操作，比如关闭文件、释放内存等
    // 请注意，由于信号处理函数在中断上下文中执行，因此应该尽量避免在此处执行耗时操作
}

int main(void){
    int ret = 0;
    char command[32];
    char driver_path[12];
    /*驱动加载*/
    for (size_t i = 1; i < 5; i++){
        sprintf(driver_path , "/dev/ds18_%d",i);
        /*printf("%s\r\n",driver_path);*/
        if(access(driver_path, F_OK) != 0){
            sprintf(command, "insmod /home/temp%d.ko", i);
            ret = system(command); // 使用system()函数执行命令
            if (ret != 0) {
            printf("驱动ds18_%d加载失败!\n",i);
            }else{
            printf("驱动ds18_%d加载成功!\n",i);
            }
        }else{
            printf("驱动ds18_%d已加载!\n",i);
        }
    }
    sprintf(driver_path , "/dev/beep");
    if(access(driver_path, F_OK) != 0){
        ret = system("insmod /home/beep_driver.ko"); // 使用system()函数执行命令
        if (ret != 0) {
            printf("驱动beep加载失败!\n");
            }else{
            printf("驱动beep加载成功!\n");
            }
    }else{
        printf("beep驱动已加载!\n");
    }
    
    signal(SIGINT, handle_interrupt); //声明捕获信号
    signal(SIGTERM, handle_interrupt);
    
    int ret1 =0;
    
    
    fan_close();
    Dev.fan_sta = 0;
    hot_close();
    Dev.hot_sta = 0;

    sta_read();
    temp_read();
    STA_SAVE;

    while(1){
        sleep(1);
        
        ret = sta_read();       /*读取状态修改，文件时间*/
        int err_ret = err_jud();        /*err_ret为真，则无法进行之后动作*/
        
        if(OP_CD){
        switch (Dev.mode_code){
            case 1:
                ret1 = ventilation_fuc();
                break;
            case 2:
                ret1 = heating_fuc();
                break;
            case 3:
                ret1 = mitekill_fuc();
                break;
            case 4:
                ret1 = drying_fuc();
                break;
            default:
                break;
            }
            sta_push();
            if (0 > ret1){
            printf("功能循环中断\r\n");
            Dev.File_last_time = 1;
            fan_close();
            Dev.fan_sta = 0;
            hot_close();
            Dev.hot_sta = 0;
            }
        }else{
            printf("File time not change,Waiting for command\r\n");
            Dev.mode_code = 0;
            sta_push();
        }
        if (0 == ret1){
        STA_SAVE;
        }
    }
    return 0;
}
