#include "dbhelper.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

namespace
{
const QString kConnName = QStringLiteral("new_pro_db");

// 取(或创建)命名连接, 避免与默认连接冲突
QSqlDatabase connection()
{
    if (QSqlDatabase::contains(kConnName))
    {
        return QSqlDatabase::database(kConnName);
    }
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), kConnName);
    db.setDatabaseName(db::dbPath());
    return db;
}
} // namespace

namespace db
{

QString dbPath()
{
    // 用户数据目录, 任何启动方式都可写(打包后也适用)
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/users.db");
}

bool init()
{
    QSqlDatabase db = connection();
    if (!db.isOpen() && !db.open())
    {
        qWarning() << "打开数据库失败:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    const QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS users ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " username TEXT NOT NULL UNIQUE,"
        " password TEXT NOT NULL," // SHA-256 十六进制
        " created_at TEXT NOT NULL DEFAULT (datetime('now','localtime'))"
        ")");
    if (!query.exec(sql))
    {
        qWarning() << "建表失败:" << query.lastError().text();
        return false;
    }
    return true;
}

QString hashPassword(const QString &plain)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(plain.toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool userExists(const QString &username)
{
    QSqlQuery query(connection());
    query.prepare(QStringLiteral("SELECT 1 FROM users WHERE username = ?"));
    query.addBindValue(username);
    if (!query.exec())
    {
        qWarning() << "查询失败:" << query.lastError().text();
        return false;
    }
    return query.next();
}

bool addUser(const QString &username, const QString &passwordHash)
{
    QSqlQuery query(connection());
    query.prepare(QStringLiteral("INSERT INTO users (username, password) VALUES (?, ?)"));
    query.addBindValue(username);
    query.addBindValue(passwordHash);
    if (!query.exec())
    {
        qWarning() << "插入失败:" << query.lastError().text();
        return false;
    }
    return true;
}

bool checkLogin(const QString &username, const QString &passwordHash)
{
    QSqlQuery query(connection());
    query.prepare(QStringLiteral("SELECT password FROM users WHERE username = ?"));
    query.addBindValue(username);
    if (!query.exec())
    {
        qWarning() << "查询失败:" << query.lastError().text();
        return false;
    }
    if (!query.next())
    {
        return false; // 用户不存在
    }
    return query.value(0).toString() == passwordHash;
}

} // namespace db
