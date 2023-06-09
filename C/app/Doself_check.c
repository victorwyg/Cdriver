#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    /*int id;         // DO编号*/
    char name[16];  // DO名称
    int value;      // DO当前值
} DO;
typedef struct DOClass {
    DO do_pin;                      // DO结构体
    int (*get)(struct DOClass *self);       // 获取DO值函数指针
    void (*set)(struct DOClass *self, int value);   // 设置DO值函数指针
} DOClass;
// 获取DO值
int DO_get(DOClass *self) {
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/leds/%s/brightness", self->do_pin.name);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    char value_str[4];
    if (read(fd, value_str, sizeof(value_str)) < 0) {
        perror("read");
        exit(1);
    }

    close(fd);

    return atoi(value_str);
}

// 设置DO值
void DO_set(DOClass *self, int value) {
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/leds/%s/brightness", self->do_pin.name);

    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    dprintf(fd, "%d", value);

    close(fd);

    self->do_pin.value = value;  // 更新DO结构体的值
}

// 创建DOClass对象
DOClass* DO_new(/*int id, */const char *Sname) {
    DOClass* self = (DOClass*)malloc(sizeof(DOClass));  // 动态分配内存
    if (self == NULL) {
        perror("malloc");
        exit(1);
    }
    /*self->do_pin.id = id;*/                // 设置DO编号
    strncpy(self->do_pin.name, Sname, sizeof(self->do_pin.name));  // 设置DO名称
    self->get = DO_get;     // 设置获取DO值函数
    self->set = DO_set;     // 设置设置DO值函数
    return self;
}
int main() {
    
    char Do_name[5];
    char cp_buf[5];
    
    for (int i = 1; i < 17; i++){
        sprintf(cp_buf, "Do%d", i);                 /*格式化*/
        strncpy (Do_name,cp_buf,sizeof(cp_buf));   /*复制传递参数*/
        DOClass *Dev = DO_new(Do_name);  // 创建DO对象传参

        int set_value = 1; /*设定参数*/
        Dev->set(Dev, set_value);
        usleep(300000);
        printf("%s value: %d\n", Do_name, Dev->get(Dev));
    if (Dev == NULL) {
        fprintf(stderr, "Failed to create DOClass object.\n");
        exit(1);
        
    }
    free(Dev);
    }
    for (int j = 1; j < 17; j++){
        sprintf(cp_buf, "Do%d", j);                 /*格式化*/
        strncpy (Do_name,cp_buf,sizeof(cp_buf));   /*复制传递参数*/
        DOClass *Dev = DO_new(Do_name);  // 创建DO对象传参

        int set_value = 0; /*设定参数*/
        Dev->set(Dev, set_value);
        usleep(300000);
        printf("%s value: %d\n", Do_name, Dev->get(Dev));

    if (Dev == NULL) {
        fprintf(stderr, "Failed to create DOClass object.\n");
        exit(1);
        
    }
    free(Dev);
    }
    
    
    return 0;
}
