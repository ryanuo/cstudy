#include <stdio.h>

void cp_file(char *src, char *dst)
{
    FILE *fp_src, *fp_dst;
    char buf;

    if ((fp_src = fopen(src, "r")) == NULL || (fp_dst = fopen(dst, "w")) == NULL)
    {
        printf("cannot  open  this  file !\n");
        return;
    }

    while ((buf = (fgetc(fp_src))) != EOF)
    {
        fputc(buf, fp_dst);
    }

    fclose(fp_src);
    fclose(fp_dst);
}

int main(int argc, char **argv)
{
    cp_file(argv[1], argv[2]);
    return 0;
}