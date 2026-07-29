#include <QTest>
#include <QTemporaryDir>

#include "AccountManager.h"

class AccountManagerTest : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;

private slots:
    void initTestCase();
    void testRegisterAndLogin();
    void testDuplicateRegister();
    void testWrongPassword();
};

void AccountManagerTest::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
}

void AccountManagerTest::testRegisterAndLogin()
{
    AccountManager mgr(m_tempDir.path());
    QVERIFY(mgr.registerUser("alice", "pass123"));
    QVERIFY(mgr.login("alice", "pass123"));
}

void AccountManagerTest::testDuplicateRegister()
{
    AccountManager mgr(m_tempDir.path());
    mgr.registerUser("bob", "bobpass");
    QVERIFY(!mgr.registerUser("bob", "newpass"));
}

void AccountManagerTest::testWrongPassword()
{
    AccountManager mgr(m_tempDir.path());
    mgr.registerUser("charlie", "correctpw");
    QVERIFY(!mgr.login("charlie", "wrongpw"));
}

QTEST_APPLESS_MAIN(AccountManagerTest)
#include "test_account.moc"
