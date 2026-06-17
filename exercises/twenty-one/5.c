#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    int p1[2];
    int p2[2];

    pipe(p1);
    pipe(p2);

    pid_t pid = fork();

    if (pid == 0)
    {
        // child

        close(p1[1]);
        close(p2[0]);

        char buf[128];

        read(p1[0], buf, sizeof(buf));

        printf("child recv: %s\n", buf);

        strcpy(buf, "hello parent");

        write(p2[1], buf, strlen(buf) + 1);

        close(p1[0]);
        close(p2[1]);
    }
    else
    {
        // parent

        close(p1[0]);
        close(p2[1]);

        char buf[128] = "hello child";

        write(p1[1], buf, strlen(buf) + 1);

        read(p2[0], buf, sizeof(buf));

        printf("parent recv: %s\n", buf);

        close(p1[1]);
        close(p2[0]);

        wait(NULL);
    }

    return 0;
}