#include <stdio.h>

void analyze_scores(int scores[], int length)
{
    int max_score = scores[0];
    int min_score = scores[0];
    int max_index = 0;
    int min_index = 0;
    int total = 0;

    printf("不及格学员信息:\n");

    for (int i = 0; i < length; i++)
    {
        total += scores[i];

        if (scores[i] < 60)
        {
            printf("下标：%d，成绩：%d\n", i, scores[i]);
        }

        if (scores[i] > max_score)
        {
            max_score = scores[i];
            max_index = i;
        }

        if (scores[i] < min_score)
        {
            min_score = scores[i];
            min_index = i;
        }
    }

    double average = (double)total / length;

    printf("最高分学员：下标 %d, 成绩 %d\n", max_index, max_score);
    printf("最低分学员：下标 %d, 成绩 %d\n", min_index, min_score);
    printf("全班总成绩：%d, 平均成绩：%.2f\n", total, average);
}


int main()
{
    const int MAX_STUDENTS = 10;
    int student_scores[MAX_STUDENTS];

    printf("请输入%d个学员的成绩：\n", MAX_STUDENTS);
    for (int i = 0; i < MAX_STUDENTS; i++)
    {
        printf("学员%d的成绩：", i + 1);
        scanf("%d", &student_scores[i]);
    }

    analyze_scores(student_scores, MAX_STUDENTS);

    return 0;
}
