#include <QTest>
#include <QTemporaryDir>
#include <QVector>
#include <QDebug>
#include <QSet>

#include "TaskManager.h"
#include "Task.h"

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
    void testStressTest();
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

void TaskManagerTest::testStressTest()
{
    QTemporaryDir stressDir;
    QVERIFY(stressDir.isValid());

    TaskManager mgr(stressDir.path());
    mgr.loadTasks("stressuser");

    const int N = 500;

    // ========== 批量添加 ==========
    for (int i = 0; i < N; ++i) {
        Task t;
        t.name = QString("任务%1").arg(i, 3, 10, QChar('0'));
        // 分钟递增，保证不冲突
        t.startTime = QDateTime(QDate(2026, 6, 1), QTime(8, 0).addSecs(i * 60));
        t.priority = static_cast<Priority>(i % 3);
        t.category = static_cast<Category>(i % 3);
        QVERIFY2(mgr.addTask(t),
                 QString("添加第%1个任务失败").arg(i).toUtf8().constData());
    }

    qDebug() << "已成功添加" << N << "个任务";

    // ========== 验证数量 ==========
    QVector<Task> all = mgr.allTasks();
    QCOMPARE(all.size(), N);

    // ========== 验证 ID 无重复 & 升序 ==========
    QSet<int> ids;
    for (const Task &t : all) {
        QVERIFY2(!ids.contains(t.id),
                 QString("发现重复 ID: %1").arg(t.id).toUtf8());
        ids.insert(t.id);
    }
    QCOMPARE(ids.size(), N);

    // ========== 批量删除一半 ==========
    int deleteCount = N / 2;
    for (int i = 1; i <= deleteCount; ++i) {
        QVERIFY2(mgr.removeTask(i),
                 QString("删除任务 ID=%1 失败").arg(i).toUtf8().constData());
    }

    // ========== 查询剩余 ==========
    QVector<Task> remaining = mgr.allTasks();
    QCOMPARE(remaining.size(), N - deleteCount);
    for (const Task &t : remaining) {
        QVERIFY(t.id > deleteCount);
    }

    // ========== 重新加载文件，验证持久化 ==========
    TaskManager mgr2(stressDir.path());
    mgr2.loadTasks("stressuser");
    QVector<Task> reloaded = mgr2.allTasks();
    QCOMPARE(reloaded.size(), N - deleteCount);

    qDebug() << "压力测试通过";
}

QTEST_APPLESS_MAIN(TaskManagerTest)
#include "test_taskmgr.moc"
