#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#define THREAD_NUM 2

typedef enum
{
    EXTRACT_SIZE,
    EXTRACT_PERM
} extract_type_t;

typedef struct
{
    long long file_size;
    mode_t file_perm;
    char *file_path;
    extract_type_t extract_type;
} thread_data_t;

void *thread_worker(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;
    struct stat st;

    if (stat(data->file_path, &st) == 0)
    {
        switch (data->extract_type)
        {
        case EXTRACT_SIZE:
            data->file_size = st.st_size;
            break;
        case EXTRACT_PERM:
            data->file_perm = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
            break;
        default:
            fprintf(stderr, "Unknown extract type\n");
            break;
        }
    }
    else
    {
        perror("stat failed");
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    pthread_t threads[THREAD_NUM];

    thread_data_t thread_data[THREAD_NUM];

    thread_data[0].file_path = argv[1];
    thread_data[0].extract_type = EXTRACT_SIZE;
    thread_data[0].file_size = 0;
    thread_data[0].file_perm = 0;

    thread_data[1].file_path = argv[1];
    thread_data[1].extract_type = EXTRACT_PERM;
    thread_data[1].file_size = 0;
    thread_data[1].file_perm = 0;

    pthread_create(&threads[0], NULL, thread_worker, &thread_data[0]);
    pthread_create(&threads[1], NULL, thread_worker, &thread_data[1]);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    printf("该文件大小为: %lld 字节\n", thread_data[0].file_size);
    printf("该文件的权限为: %04o\n", thread_data[1].file_perm);

    return 0;
}