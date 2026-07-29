#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QString>
#include <QHash>

class AccountManager {
public:
    AccountManager(const QString &dataDir = "data/");

    bool registerUser(const QString &username, const QString &password);
    bool login(const QString &username, const QString &password);
    bool userExists(const QString &username) const;

private:
        QString m_dataDir;

    //哈希算法
    static QString hashPassword(const QString &password, const QString &salt);
    static QString generateSalt();
    static bool verifyPassword(const QString &password, const QString &stored);

    // 加载/保存用户文件
    void loadUsers();
    void saveUsers();

    QHash<QString, QString> m_users;
    bool m_dirty = false;
};

#endif