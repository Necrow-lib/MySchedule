#include "ReminderThread.h"
#include "TaskManager.h"
#include <QDateTime>

// ============================================================
// ReminderThread.cpp — 后台提醒线程实现
//
// 工作流程：
//   1. 每 5 秒检查一次所有任务的提醒时间
//   2. 对每个任务判断：remindTime ≤ now < startTime？
//      - remindTime ≤ now：提醒时间已到
//      - now < startTime：任务还没开始（已开始的任务不需要提醒）
//   3. 满足条件且未提醒过的任务，发射 remind 信号
//   4. 记录已提醒的 ID，防止每 5 秒重复提醒同一任务
//   5. 每 1 分钟清理一次 m_notifiedIds，移除已删除任务的旧记录
// ============================================================

// 检查间隔 5 秒（5000 毫秒）
static constexpr int CHECK_INTERVAL_MS = 5000;

// 清理周期：每 12 次检查（12 × 5 = 60 秒）清理一次过期通知记录
static constexpr int CLEANUP_CYCLE = 12;

ReminderThread::ReminderThread(TaskManager *mgr, QObject *parent)
    : QThread(parent), m_taskMgr(mgr) {}

void ReminderThread::stop() {
    m_running = false;      // 原子操作，线程安全地通知 run() 退出循环
}

void ReminderThread::run() {
    int cycleCount = 0;     // 检查次数计数器，用于触发定期清理

    while (m_running) {
        QDateTime now = QDateTime::currentDateTime();   // 获取当前时间

        if (m_taskMgr) {
            // 获取任务快照（TaskManager::allTasks 返回拷贝，线程安全）
            const QVector<Task> tasks = m_taskMgr->allTasks();

            for (const auto &task : tasks) {
                // 跳过没有设置提醒时间的任务
                if (!task.remindTime.isValid()) continue;

                // 跳过本轮已经提醒过的任务
                if (m_notifiedIds.contains(task.id)) continue;

                // 核心判断：提醒时间已到 且 任务尚未开始
                if (task.remindTime <= now && task.startTime > now) {
                    // 发射信号，由主线程的槽函数显示提醒
                    emit remind(task.name,
                                task.startTime.toString("yyyy-MM-dd HH:mm"));

                    // 记录已提醒，防止重复
                    m_notifiedIds.insert(task.id);
                }
            }

            // 定期清理过期通知记录
            // 为什么需要：用户可能删除了已经提醒过的任务，该任务 ID
            // 不会再出现在 tasks 中，但 m_notifiedIds 里还留着。
            // 虽然 Set 很小，但长期运行会慢慢堆积，每分钟清理一次即可。
            if (++cycleCount >= CLEANUP_CYCLE) {
                cycleCount = 0;

                // 取当前有效任务 ID 的集合
                QSet<int> validIds;
                for (const auto &t : tasks)
                    validIds.insert(t.id);

                // 取交集，移除已不存在的任务 ID
                m_notifiedIds.intersect(validIds);
            }
        }

        QThread::msleep(CHECK_INTERVAL_MS);     // 挂起 5 秒
    }
}
