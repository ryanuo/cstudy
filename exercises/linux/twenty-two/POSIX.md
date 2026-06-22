### 1）创建共享内存对象

```text
函数名：shm_open

头文件：
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>

函数原型：
    int shm_open(
        const char *name,
        int oflag,
        mode_t mode
    );

函数功能：
    创建或打开一个 POSIX 共享内存对象

函数参数：
    name：
        共享内存名称
        必须以 '/' 开头

    oflag：
        打开方式

        O_CREAT
            不存在则创建

        O_RDWR
            读写方式

        O_RDONLY
            只读方式

    mode：
        权限设置

        0666
        0644

函数返回值：
    成功：
        返回共享内存文件描述符 fd

    失败：
        返回 -1
        错误码存放于 errno
```

### 2）设置共享内存大小

```text
函数名：ftruncate

头文件：
    #include <unistd.h>

函数原型：
    int ftruncate(
        int fd,
        off_t length
    );

函数功能：
    设置共享内存大小

函数参数：
    fd：
        shm_open返回的文件描述符

    length：
        共享内存大小(字节)

函数返回值：
    成功：
        返回 0

    失败：
        返回 -1
        错误码存放于 errno
```

### 3）建立映射

```text
函数名：mmap

头文件：
    #include <sys/mman.h>

函数原型：
    void *mmap(
        void *addr,
        size_t length,
        int prot,
        int flags,
        int fd,
        off_t offset
    );

函数功能：
    将共享内存映射到进程地址空间

函数参数：
    addr：
        映射地址
        通常填写 NULL

    length：
        映射大小

    prot：
        访问权限

        PROT_READ
            可读

        PROT_WRITE
            可写

        PROT_READ | PROT_WRITE
            可读可写

    flags：
        映射方式

        MAP_SHARED
            共享映射

    fd：
        shm_open返回的描述符

    offset：
        映射偏移
        通常为0

函数返回值：
    成功：
        返回映射首地址

    失败：
        返回 MAP_FAILED
        即 (void *)-1
```

### 4）解除映射

```text
函数名：munmap

头文件：
    #include <sys/mman.h>

函数原型：
    int munmap(
        void *addr,
        size_t length
    );

函数功能：
    解除共享内存映射

函数参数：
    addr：
        mmap返回的地址

    length：
        映射长度

函数返回值：
    成功：
        返回 0

    失败：
        返回 -1
        错误码存放于 errno
```

### 5）关闭共享内存描述符

```text
函数名：close

头文件：
    #include <unistd.h>

函数原型：
    int close(
        int fd
    );

函数功能：
    关闭共享内存描述符

函数参数：
    fd：
        shm_open返回的文件描述符

函数返回值：
    成功：
        返回 0

    失败：
        返回 -1
        错误码存放于 errno
```

### 6）删除共享内存对象

```text
函数名：shm_unlink

头文件：
    #include <sys/mman.h>

函数原型：
    int shm_unlink(
        const char *name
    );

函数功能：
    删除共享内存对象

函数参数：
    name：
        共享内存名称

函数返回值：
    成功：
        返回 0

    失败：
        返回 -1
        错误码存放于 errno
```

### POSIX共享内存操作步骤

```text
1）创建共享内存对象
    shm_open()

2）设置共享内存大小
    ftruncate()

3）映射共享内存
    mmap()

4）读写共享内存
    strcpy()
    memcpy()

5）解除映射
    munmap()

6）关闭描述符
    close()

7）删除共享内存
    shm_unlink()
```

### 与 System V 对照

```text
System V                    POSIX

shmget()      <=====>      shm_open()

shmat()       <=====>      mmap()

shmdt()       <=====>      munmap()

shmctl()
(IPC_RMID)    <=====>      shm_unlink()
```
