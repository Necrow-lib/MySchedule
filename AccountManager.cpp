#include "AccountManager.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>

AccountManager::AccountManager(const QString &dataDir)
    : m_dataDir(dataDir)
{
    QDir().mkpath(m_dataDir);
    loadUsers();
}

QString AccountManager::generateSalt() {
    QByteArray saltBytes;
    saltBytes.resize(16);
    for (int i = 0; i < 16; ++i) {
        saltBytes[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    return QString(saltBytes.toHex());
}

QString AccountManager::hashPassword(const QString &password, const QString &salt) {
    QByteArray data = (salt + password).toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool AccountManager::verifyPassword(const QString &password, const QString &stored) {
    int colonPos = stored.indexOf(':');
    if (colonPos <= 0) return false;
    QString salt = stored.left(colonPos);
    QString hash = stored.mid(colonPos + 1);
    return hashPassword(password, salt) == hash;
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
    QString salt = generateSalt();
    QString hash = hashPassword(password, salt);
    m_users[username] = salt + ":" + hash;
    m_dirty = true;
    saveUsers();
    return true;
}

bool AccountManager::login(const QString &username, const QString &password) {
    if (!m_users.contains(username)) return false;
    return verifyPassword(password, m_users[username]);
}

bool AccountManager::userExists(const QString &username) const {
    return m_users.contains(username);
}
