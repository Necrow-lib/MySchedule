#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "Task.h"
#include <QVector>
#include <QString>
#include <QDate>
#include <QMutex>

// 任务管理器 —— CRUD + 文件持久化 + 查询，线程安全
class TaskManager {
public:
    explicit TaskManager(const QString &dataDir = "data/");

    // ── 用户数据切换 ──
    void setCurrentUser(const QString &username);
    void loadTasks(const QString &username);
    void saveTasks();

    // ── 任务操作 ──
    bool addTask(const Task &task);      // 失败返回 false
    bool removeTask(int id);             // 根据 ID 删除
    Task* findTask(int id);              // 返回 nullptr 表示未找到

    // ── 查询（返回拷贝，线程安全）──
    QVector<Task> tasksForDate(const QDate &date) const;
    QVector<Task> tasksForMonth(int year, int month) const;
    QVector<Task> allTasks() const;      // 提醒线程用

    // ── ID 管理 ──
    int  nextId() const { return m_nextId; }
    void setNextId(int id) { m_nextId = id; }

    // ── 当前用户 ──
    QString currentUser() const { return m_currentUser; }

private:
    QString m_dataDir;
    QString m_currentUser;
    QVector<Task> m_tasks;
    int m_nextId = 1;
    mutable QMutex m_mutex;              // 保护 m_tasks 读写

    // 内部校验（调用前需持有锁）
    bool hasTimeConflict(const Task &task, int excludeId = -1) const;
    bool hasNameTimeUnique(const Task &task, int excludeId = -1) const;

    // 用户任务文件路径
    QString taskFile() const;
};

#endif // TASKMANAGER_H
