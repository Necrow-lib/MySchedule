#include "AccountManager.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AccountManager::AccountManager(const QString &dataDir)
    : m_dataDir(dataDir)
{
    QDir().mkpath(m_dataDir);
    loadUsers();
}

QString AccountManager::hashPassword(const QString &password) const {
    QByteArray data = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

void AccountManager::loadUsers() {
    QFile file(m_dataDir + "/users.json");
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it) {
        m_users[it.key()] = it.value().toString();
    }
    m_dirty = false;
}

void AccountManager::saveUsers() {
    if (!m_dirty) return;
    QJsonObject root;
    for (auto it = m_users.begin(); it != m_users.end(); ++it) {
        root[it.key()] = it.value();
    }
    QJsonDocument doc(root);
    QFile file(m_dataDir + "/users.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
    m_dirty = false;
}

bool AccountManager::registerUser(const QString &username, const QString &password) {
    if (username.isEmpty() || m_users.contains(username)) return false;
    m_users[username] = hashPassword(password);
    m_dirty = true;
    saveUsers();
    return true;
}

bool AccountManager::login(const QString &username, const QString &password) {
    if (!m_users.contains(username)) return false;
    return m_users[username] == hashPassword(password);
}

bool AccountManager::userExists(const QString &username) const {
    return m_users.contains(username);
}
