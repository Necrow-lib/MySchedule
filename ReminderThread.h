#ifndef REMINDERTHREAD_H
#define REMINDERTHREAD_H

#include <QThread>
#include <QSet>
#include <atomic>

class TaskManager;

// ============================================================
// ReminderThread.h — 后台提醒线程
//
// 职责：在独立线程中周期性检查所有任务，发现到达提醒时间的任务时，
//       通过 Qt 信号通知前端（GUI 或控制台）。
//
// 为什么用独立线程：主线程(GUI)需要响应用户操作（按钮点击等），
//   如果也在主线程做周期性检查，会导致界面卡顿。
//   用 QThread 把检查逻辑放到后台，互不阻塞。
//
// 通信方式：Qt 信号槽机制
//   - 提醒线程 emit remind() → 主线程的槽函数接收并显示提醒
//   - 使用 Qt::AutoConnection（默认），Qt 会自动跨线程投递信号
// ============================================================

class ReminderThread : public QThread {
    Q_OBJECT
public:
    // 构造函数：mgr 指向 TaskManager 实例，用于读取任务列表
    explicit ReminderThread(TaskManager *mgr, QObject *parent = nullptr);

    // 安全停止线程：设置标志位，等待 run() 自然退出
    void stop();

signals:
    // 提醒信号，taskName=任务名称，startTimeStr=任务开始时间（格式化后）
    void remind(const QString &taskName, const QString &startTimeStr);

protected:
    void run() override;    // 线程主循环

private:
    TaskManager *m_taskMgr;              // 任务管理器指针（只读，无需加锁）
    std::atomic<bool> m_running{true};   // 运行标志，stop() 将其设为 false
    QSet<int> m_notifiedIds;             // 已提醒过的任务 ID 集合，防止同一次运行中重复提醒
};

#endif // REMINDERTHREAD_H
