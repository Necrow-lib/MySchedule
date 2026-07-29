#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "Task.h"
#include <QVector>
#include <QString>
#include <QDate>
#include <QMutex>



class TaskManager {
public:
    explicit TaskManager(const QString &dataDir = "data/");


    void setCurrentUser(const QString &username);
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

    // 根据 ID 更新任务（修改名称、时间、优先级、分类、提醒时间）
    // 返回 false 表示更新失败（冲突或 ID 不存在）
    // 内部会排除自身进行冲突检测
    bool updateTask(int id, const Task &newData);

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

    int  nextId() const { return m_nextId; }
    void setNextId(int id) { m_nextId = id; }

    QString currentUser() const { return m_currentUser; }

private:
    QString m_dataDir; 
    QString m_currentUser; 
    QVector<Task> m_tasks;  
    int m_nextId = 1; 
    mutable QMutex m_mutex; 

    bool hasTimeConflict(const Task &task, int excludeId = -1) const;

    bool hasNameTimeConflict(const Task &task, int excludeId = -1) const;

    QString taskFilePath() const;
};

#endif 
