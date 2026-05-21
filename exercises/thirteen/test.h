
#ifndef __USER
#define __USER

enum STATUS
{
    error = 0,
    success = 1,
};

typedef struct userInfo
{
    char name[20];
    char password[20];
} user_t;

int login_user();

int sign_user();

void clear_buffer();

#endif