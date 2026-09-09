#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

void find_most_frequent(char *buf)
{
    int count[26] = {0};
    int max_count = 0;
    char max_char = 'a';

    while (*buf != '\0')
    {
        if (isalpha(*buf))
        {
            char lower_c = tolower(*buf);
            count[lower_c - 'a']++;
        }
        buf++;
    }

    for (int i = 0; i < 26; i++)
    {
        if (count[i] > max_count)
        {
            max_count = count[i];
            max_char = 'a' + i;
        }
    }

    if (max_count > 0)
    {
        printf("使用最多的字母是: %c，出现了 %d 次\n", max_char, max_count);
    }
    else
    {
        printf("字符串中没有英文字母\n");
    }
}

int main(int argc, char **argv)
{
    int fd = open("a.txt", R_OK);

    ssize_t n = 0;
    char buf[3000] = {0};
    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        buf[n] = '\0';
        // fprintf(stdout, "%s\n", buf);
        find_most_frequent(buf);
    }

    if (n == -1)
    {
        perror("Failed to read a.txt");
    }

    close(fd);
    return 0;
}