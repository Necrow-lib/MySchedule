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
    QString hashPassword(const QString &password) const;

    // 加载/保存用户文件
    void loadUsers();
    void saveUsers();

    QHash<QString, QString> m_users;  // username -> hash
    bool m_dirty = false;
};

#endif