#ifndef REMINDERTHREAD_H
#define REMINDERTHREAD_H

#include <QThread>
#include <QSet>
#include <atomic>

class TaskManager;

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
    QSet<int> m_notifiedIds; 
};

#endif
