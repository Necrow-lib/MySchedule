#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "Task.h"
#include <QVector>
#include <QString>
#include <QDate>
#include <QMutex>

// ============================================================
// TaskManager.h — 任务管理器
// 职责：CRUD 操作、文件持久化、数据查询、线程安全
//
// 线程模型说明：
//   主线程（GUI）：调用 addTask/removeTask/loadTasks/saveTasks
//   提醒线程：    周期性调用 allTasks() 读取任务列表
//   两个线程同时访问 m_tasks，必须用 QMutex 保护。
//
// 锁策略：
//   - 所有读取 m_tasks 的方法：加锁 → 拷贝数据 → 解锁 → 返回拷贝
//   - addTask/removeTask：加锁 → 校验+修改内存 → 解锁 → 写文件
//     文件 I/O 在锁外执行，避免阻塞提醒线程（写文件可能很慢）
// ============================================================

class TaskManager {
public:
    // 构造函数：dataDir 指定数据文件存放目录，默认为 "data/"
    explicit TaskManager(const QString &dataDir = "data/");

    // ---------- 用户数据切换 ----------

    // 设置当前用户名（不加载数据，只记录用户名）
    void setCurrentUser(const QString &username);

    // 从文件加载指定用户的任务列表到内存
    // 文件路径：dataDir/用户名_tasks.json
    void loadTasks(const QString &username);

    // 将当前用户的任务列表保存到文件
    // 注意：内部先拷贝快照再写文件，线程安全
    void saveTasks();

    // ---------- 任务操作 ----------

    // 添加任务，返回 false 表示校验失败：
    //   1. 开始时间与其他任务冲突（作业要求：每个任务的开始时间不能相同）
    //   2. 任务名称+开始时间不唯一
    // 成功后自动分配 ID 并立即保存到文件
    bool addTask(const Task &task);

    // 根据 ID 删除任务，返回 false 表示 ID 不存在
    // 删除后自动保存到文件
    bool removeTask(int id);

    // 根据 ID 查找任务，返回指针（调用者不应长期持有此指针，
    // 因为后续的 addTask/removeTask 可能导致 vector 重新分配内存）
    Task* findTask(int id);

    // ---------- 查询（均返回拷贝，多线程安全）----------

    // 查询指定日期的所有任务，按开始时间升序排列
    QVector<Task> tasksForDate(const QDate &date) const;

    // 查询指定年月的所有任务，按开始时间升序排列
    QVector<Task> tasksForMonth(int year, int month) const;

    // 返回全部任务的拷贝，供提醒线程使用
    // 返回拷贝而非引用：保证提醒线程遍历时不会因主线程修改而崩溃
    QVector<Task> allTasks() const;

    // ---------- ID 管理 ----------

    int  nextId() const { return m_nextId; }
    void setNextId(int id) { m_nextId = id; }

    // ---------- 工具 ----------

    QString currentUser() const { return m_currentUser; }

private:
    QString m_dataDir;               // 数据文件存放目录
    QString m_currentUser;           // 当前登录用户名
    QVector<Task> m_tasks;           // 内存中的任务列表
    int m_nextId = 1;                // 下一个可用的任务 ID
    mutable QMutex m_mutex;          // 保护 m_tasks 的多线程访问（mutable 允许在 const 方法中加锁）

    // 内部校验函数（调用前必须持有 m_mutex）
    // 检查开始时间是否与其他任务冲突
    bool hasTimeConflict(const Task &task, int excludeId = -1) const;

    // 检查任务名称+开始时间的组合是否唯一
    bool hasNameTimeConflict(const Task &task, int excludeId = -1) const;

    // 返回当前用户的任务文件完整路径
    QString taskFilePath() const;
};

#endif // TASKMANAGER_H
