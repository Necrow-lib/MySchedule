#include "ReminderThread.h"
#include "TaskManager.h"
#include <QDateTime>

// 检查间隔（毫秒）
static constexpr int CHECK_MS = 5000;
// 每 N 次检查清理一次已删除任务的过期通知记录
static constexpr int CLEANUP_CYCLE = 12;  // 12×5s = 每分钟

ReminderThread::ReminderThread(TaskManager *mgr, QObject *parent)
    : QThread(parent), m_taskMgr(mgr) {}

void ReminderThread::stop() {
    m_running = false;
}

void ReminderThread::run() {
    int cycleCount = 0;

    while (m_running) {
        QDateTime now = QDateTime::currentDateTime();

        if (m_taskMgr) {
            const QVector<Task> tasks = m_taskMgr->allTasks();  // 线程安全拷贝

            for (const auto &task : tasks) {
                if (!task.remindTime.isValid()) continue;
                if (m_notifiedIds.contains(task.id)) continue;

                // 提醒时间已到 且 任务尚未开始
                if (task.remindTime <= now && task.startTime > now) {
                    emit remind(task.name,
                                task.startTime.toString("yyyy-MM-dd HH:mm"));
                    m_notifiedIds.insert(task.id);
                }
            }

            // 定期清理：去掉已被删除的任务ID
            if (++cycleCount >= CLEANUP_CYCLE) {
                cycleCount = 0;
                QSet<int> valid;
                for (const auto &t : tasks)
                    valid.insert(t.id);
                m_notifiedIds.intersect(valid);
            }
        }

        QThread::msleep(CHECK_MS);
    }
}