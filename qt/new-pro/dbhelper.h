#ifndef DBHELPER_H
#define DBHELPER_H

#include <QString>

// SQLite 数据库辅助: 注册/登录共用的连接与查询
namespace db {

// 打开数据库并建表(幂等), 返回是否成功
bool init();

// 数据库文件路径
QString dbPath();

// SHA-256 十六进制哈希
QString hashPassword(const QString &plain);

// 用户名是否已存在
bool userExists(const QString &username);

// 新增用户, 返回是否成功(用户名重复返回 false)
bool addUser(const QString &username, const QString &passwordHash);

// 校验登录: 用户名存在且密码哈希匹配
bool checkLogin(const QString &username, const QString &passwordHash);

} // namespace db

#endif // DBHELPER_H
