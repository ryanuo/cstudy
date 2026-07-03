#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// 任务结构体：代表要执行的工作（漫画里的任务A/B/C）
typedef struct task {
    void (*func)(void* arg); // 任务回调函数
    void* arg;               // 任务参数
    struct task* next;       // 链表任务队列
} task_t;

// 线程池结构体 struct thread_pool_t（漫画里的木盆）
typedef struct thread_pool_t {
    task_t* task_head;    // 任务队列头
    task_t* task_tail;    // 任务队列尾

    pthread_mutex_t mutex;    // 互斥锁：保护任务队列
    pthread_cond_t cond;      // 条件变量：唤醒休眠线程

    int thread_cnt;    // 池内线程总数
    int pool_closed;    // 线程池销毁标记
} thread_pool_t;

// 全局线程池对象
static thread_pool_t pool;

// 单个线程工作函数（木盆里睡觉的小黑线程）
void* thread_worker(void* arg) {
    (void)arg;
    printf("【线程%lu】进入线程池，开始休眠等待任务...\n", (unsigned long)pthread_self());

    while (1) {
        // 加锁访问任务队列
        pthread_mutex_lock(&pool.mutex);

        // 无任务 && 池子没关闭 → 休眠（漫画：zzz睡觉）
        while (pool.task_head == NULL && !pool.pool_closed) {
            pthread_cond_wait(&pool.cond, &pool.mutex);
        }

        // 池子关闭，线程退出
        if (pool.pool_closed) {
            pthread_mutex_unlock(&pool.mutex);
            printf("【线程%lu】线程池关闭，退出\n", (unsigned long)pthread_self());
            break;
        }

        // 取出队首任务
        task_t* cur_task = pool.task_head;
        pool.task_head = pool.task_head->next;
        // 队列空时尾指针置空
        if (pool.task_head == NULL)
            pool.task_tail = NULL;

        pthread_mutex_unlock(&pool.mutex);

        // 执行任务（漫画：被唤醒干活）
        printf("【线程%lu】被唤醒，开始执行任务\n", (unsigned long)pthread_self());
        cur_task->func(cur_task->arg);
        free(cur_task); // 释放任务

        // 任务执行完毕，回到循环继续等待下一个任务（复用，冷启动=0）
        printf("【线程%lu】任务完成，回归线程池休眠等待新任务\n\n", (unsigned long)pthread_self());
    }
    return NULL;
}

// 1. 初始化线程池（漫画第一格：一次造好，创建线程池）
void pool_init(int thread_num) {
    memset(&pool, 0, sizeof(pool));
    pthread_mutex_init(&pool.mutex, NULL);
    pthread_cond_init(&pool.cond, NULL);

    pool.thread_cnt = thread_num;
    pool.pool_closed = 0;

    // 批量创建N个工作线程
    for (int i = 0; i < thread_num; i++) {
        pthread_t tid;
        pthread_create(&tid, NULL, thread_worker, NULL);
    }
    printf("线程池初始化完成，共创建 %d 个常驻线程\n\n", thread_num);
}

// 2. 向线程池投放任务（任务队列新增任务）
int pool_add_task(void (*func)(void*), void* arg) {
    task_t* new_task = (task_t*)malloc(sizeof(task_t));
    if (!new_task) return -1;
    new_task->func = func;
    new_task->arg = arg;
    new_task->next = NULL;

    pthread_mutex_lock(&pool.mutex);
    // 尾插法加入任务队列
    if (pool.task_tail == NULL) {
        pool.task_head = new_task;
        pool.task_tail = new_task;
    } else {
        pool.task_tail->next = new_task;
        pool.task_tail = new_task;
    }
    pthread_mutex_unlock(&pool.mutex);

    // 唤醒任意一个休眠线程（漫画：叮！唤醒）
    pthread_cond_signal(&pool.cond);
    return 0;
}

// 3. 销毁线程池
void pool_destroy() {
    pthread_mutex_lock(&pool.mutex);
    pool.pool_closed = 1;
    pthread_mutex_unlock(&pool.mutex);

    // 唤醒所有休眠线程，让它们退出
    pthread_cond_broadcast(&pool.cond);
    sleep(1); // 等待线程全部退出

    // 释放锁、条件变量
    pthread_mutex_destroy(&pool.mutex);
    pthread_cond_destroy(&pool.cond);
    // 清空剩余任务
    task_t* p = pool.task_head;
    while (p) {
        task_t* tmp = p;
        p = p->next;
        free(tmp);
    }
    printf("线程池销毁完毕\n");
}

// 自定义测试任务函数（模拟业务任务A/B/C）
void test_task(void* arg) {
    char* task_name = (char*)arg;
    printf("执行任务：%s，线程ID：%lu\n", task_name, (unsigned long)pthread_self());
    sleep(1); // 模拟任务耗时
}

int main() {
    // 1. 初始化线程池，创建3个常驻线程（漫画第1、2格）
    pool_init(3);
    sleep(2); // 等待线程全部进入休眠

    // 2. 投放3个任务到任务队列（漫画第3格：任务到达唤醒线程）
    pool_add_task(test_task, "任务A");
    pool_add_task(test_task, "任务B");
    pool_add_task(test_task, "任务C");

    sleep(5); // 等待所有任务执行完成，线程回归池休眠

    // 再投放新任务，验证线程复用（冷启动=0，不用新建线程）
    printf("===== 投放第二批新任务，复用已有线程 =====\n");
    pool_add_task(test_task, "任务D");
    pool_add_task(test_task, "任务E");
    sleep(3);

    // 销毁线程池
    pool_destroy();
    return 0;
}