#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QVector>

#include "AccountManager.h"
#include "TaskManager.h"
#include "Task.h"


class TaskTest : public QObject {
    Q_OBJECT

private slots:
    void testJsonRoundTrip();
    void testEnumConversions();
};

void TaskTest::testJsonRoundTrip()
{
    Task t;
    t.id = 42;
    t.name = "测试任务";
    t.startTime = QDateTime(QDate(2026, 1, 15), QTime(10, 30));
    t.priority = Priority::High;
    t.category = Category::Study;
    t.remindTime = QDateTime(QDate(2026, 1, 15), QTime(10, 0));

    QJsonObject obj = t.toJson();
    Task t2 = Task::fromJson(obj);

    QCOMPARE(t2.id, 42);
    QCOMPARE(t2.name, QString("测试任务"));
    QCOMPARE(t2.startTime, QDateTime(QDate(2026, 1, 15), QTime(10, 30)));
    QCOMPARE(t2.priority, Priority::High);
    QCOMPARE(t2.category, Category::Study);
    QCOMPARE(t2.remindTime, QDateTime(QDate(2026, 1, 15), QTime(10, 0)));
}

void TaskTest::testEnumConversions()
{
    QCOMPARE(priorityToStr(Priority::Low), QString("低"));
    QCOMPARE(priorityToStr(Priority::Medium), QString("中"));
    QCOMPARE(priorityToStr(Priority::High), QString("高"));

    QCOMPARE(categoryToStr(Category::Study), QString("学习"));
    QCOMPARE(categoryToStr(Category::Entertainment), QString("娱乐"));
    QCOMPARE(categoryToStr(Category::Life), QString("生活"));

    QCOMPARE(parsePriority("低"), Priority::Low);
    QCOMPARE(parsePriority("中"), Priority::Medium);
    QCOMPARE(parsePriority("高"), Priority::High);

    QCOMPARE(parseCategory("学习"), Category::Study);
    QCOMPARE(parseCategory("娱乐"), Category::Entertainment);
    QCOMPARE(parseCategory("生活"), Category::Life);
}

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

class TaskManagerTest : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    TaskManager *m_mgr = nullptr;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testAddAndFindTask();
    void testTimeConflict();
    void testNameTimeConflict();
    void testRemoveTask();
    void testTasksForDate();
    void testTasksForMonth();
};

void TaskManagerTest::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    m_mgr = new TaskManager(m_tempDir.path());
    m_mgr->loadTasks("testuser");
}

void TaskManagerTest::cleanupTestCase()
{
    delete m_mgr;
    m_mgr = nullptr;
}

void TaskManagerTest::testAddAndFindTask()
{
    Task t;
    t.name = "单元测试任务";
    t.startTime = QDateTime(QDate(2026, 2, 1), QTime(9, 0));
    t.priority = Priority::Medium;
    t.category = Category::Entertainment;

    QVERIFY(m_mgr->addTask(t));
    QVERIFY(m_mgr->nextId() > 1);  

    Task *found = m_mgr->findTask(1);
    QVERIFY(found != nullptr);
    QCOMPARE(found->name, QString("单元测试任务"));
}

void TaskManagerTest::testTimeConflict()
{
    Task t;
    t.name = "冲突任务";
    t.startTime = QDateTime(QDate(2026, 2, 1), QTime(9, 0));
    t.priority = Priority::Low;
    t.category = Category::Life;

    QVERIFY(!m_mgr->addTask(t));
}

void TaskManagerTest::testNameTimeConflict()
{

    Task t;
    t.name = "单元测试任务";
    t.startTime = QDateTime(QDate(2026, 2, 1), QTime(9, 0));
    t.priority = Priority::High;
    t.category = Category::Study;

    QVERIFY(!m_mgr->addTask(t));
}

void TaskManagerTest::testRemoveTask()
{
    Task t;
    t.name = "待删除任务";
    t.startTime = QDateTime(QDate(2026, 3, 1), QTime(14, 0));
    m_mgr->addTask(t);

    int lastId = m_mgr->nextId() - 1;
    QVERIFY(m_mgr->removeTask(lastId));
    QVERIFY(m_mgr->findTask(lastId) == nullptr);

    QVERIFY(!m_mgr->removeTask(lastId));
}

void TaskManagerTest::testTasksForDate()
{

    TaskManager mgr(m_tempDir.path());
    mgr.loadTasks("dateuser");

    Task t1;
    t1.name = "当天任务";
    t1.startTime = QDateTime(QDate(2026, 4, 10), QTime(8, 0));
    mgr.addTask(t1);

    Task t2;
    t2.name = "另一任务";
    t2.startTime = QDateTime(QDate(2026, 4, 10), QTime(10, 30));
    mgr.addTask(t2);

    Task t3;
    t3.name = "隔天任务";
    t3.startTime = QDateTime(QDate(2026, 4, 11), QTime(9, 0));
    mgr.addTask(t3);

    QVector<Task> dayTasks = mgr.tasksForDate(QDate(2026, 4, 10));
    QCOMPARE(dayTasks.size(), 2);

    QVERIFY(dayTasks[0].startTime < dayTasks[1].startTime);
}

void TaskManagerTest::testTasksForMonth()
{
    TaskManager mgr(m_tempDir.path());
    mgr.loadTasks("monthuser");

    Task t1;
    t1.name = "二月任务";
    t1.startTime = QDateTime(QDate(2026, 2, 15), QTime(12, 0));
    mgr.addTask(t1);

    Task t2;
    t2.name = "三月上旬";
    t2.startTime = QDateTime(QDate(2026, 3, 5), QTime(9, 0));
    mgr.addTask(t2);

    Task t3;
    t3.name = "三月下旬";
    t3.startTime = QDateTime(QDate(2026, 3, 25), QTime(16, 0));
    mgr.addTask(t3);

    QVector<Task> marchTasks = mgr.tasksForMonth(2026, 3);
    QCOMPARE(marchTasks.size(), 2);
    QCOMPARE(marchTasks[0].name, QString("三月上旬"));
    QCOMPARE(marchTasks[1].name, QString("三月下旬"));

    QVector<Task> febTasks = mgr.tasksForMonth(2026, 2);
    QCOMPARE(febTasks.size(), 1);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int status = 0;
    {
        TaskTest tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    {
        AccountManagerTest tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    {
        TaskManagerTest tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    return status;
}

#include "test_main.moc"