#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

int main(int argc, char **argv)
{
    MYSQL *mysql = NULL;

    // 初始化 MySQL
    mysql = mysql_init(NULL);

    if (mysql == NULL)
    {
        printf("mysql_init error\n");
        return -1;
    }

    // 连接数据库
    mysql = mysql_real_connect(
        mysql,
        "localhost", // host
        "admin",      // user
        "123456",    // password
        "test_db",   // database
        0,           // port 0 默认3306
        NULL,
        0);

    if (mysql == NULL)
    {
        printf(
            "connect error: %s\n",
            mysql_error(mysql));

        return -1;
    }

    printf("mysql connect success\n");

    // 执行SQL
    int ret = mysql_query(
        mysql,
        "select * from tb_character_info");

    if (ret != 0)
    {
        printf(
            "query error: %s\n",
            mysql_error(mysql));

        mysql_close(mysql);
        return -1;
    }

    // 获取查询结果
    MYSQL_RES *result =
        mysql_store_result(mysql);

    if (result == NULL)
    {
        printf(
            "store result error: %s\n",
            mysql_error(mysql));

        mysql_close(mysql);
        return -1;
    }

    // 获取列数量
    int fields =
        mysql_num_fields(result);

    printf(
        "fields: %d\n",
        fields);

    // 获取每一行数据

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != NULL)
    {

        for (int i = 0; i < fields; i++)
        {
            printf(
                "%s\t",
                row[i] ? row[i] : "NULL");
        }

        printf("\n");
    }

    // 释放结果集

    mysql_free_result(result);

    // 关闭数据库连接

    mysql_close(mysql);

    return 0;
}