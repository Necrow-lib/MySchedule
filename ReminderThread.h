#ifndef REMINDERTHREAD_H
#define REMINDERTHREAD_H

#include <QThread>
#include <QSet>
#include <atomic>

class TaskManager;

// 后台提醒线程 —— 每5秒检查一次，提醒时间到达时发射信号
class ReminderThread : public QThread {
    Q_OBJECT
public:
    explicit ReminderThread(TaskManager *mgr, QObject *parent = nullptr);
    void stop();

signals:
    void remind(const QString &taskName, const QString &startTimeStr);

protected:
    void run() override;

private:
    TaskManager *m_taskMgr;
    std::atomic<bool> m_running{true};
    QSet<int> m_notifiedIds;            // 已提醒的任务ID，防止重复
};

#endif // REMINDERTHREAD_H