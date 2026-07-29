#include <QTest>
#include <QJsonObject>
#include <QDateTime>

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

QTEST_APPLESS_MAIN(TaskTest)
#include "test_task.moc"
