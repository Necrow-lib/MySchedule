#include "TaskManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <algorithm>

// ─── 构造 / 工具 ──────────────────────────────────────

TaskManager::TaskManager(const QString &dataDir)
    : m_dataDir(dataDir)
{
    QDir().mkpath(m_dataDir);
}

QString TaskManager::taskFile() const {
    return m_dataDir + "/" + m_currentUser + "_tasks.json";
}

void TaskManager::setCurrentUser(const QString &username) {
    m_currentUser = username;
}

// ─── 文件加载 ────────────────────────────────────────

void TaskManager::loadTasks(const QString &username) {
    QMutexLocker locker(&m_mutex);

    m_tasks.clear();
    m_currentUser = username;

    QFile file(taskFile());
    if (!file.open(QIODevice::ReadOnly)) {
        m_nextId = 1;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray arr = doc.array();
    for (const auto &val : arr) {
        m_tasks.append(Task::fromJson(val.toObject()));
    }

    // 恢复 nextId = max(id) + 1
    m_nextId = 1;
    for (const auto &t : m_tasks) {
        if (t.id >= m_nextId) m_nextId = t.id + 1;
    }
}

// 注意：saveTasks 从 addTask/removeTask 在锁外调用
// 因此先拷贝快照再序列化，避免竞争
void TaskManager::saveTasks() {
    QVector<Task> snapshot;
    {
        QMutexLocker locker(&m_mutex);
        snapshot = m_tasks;
    }

    QJsonArray arr;
    for (const auto &t : snapshot) {
        arr.append(t.toJson());
    }

    QJsonDocument doc(arr);
    QFile file(taskFile());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

// ─── 内部校验（调用前需持有 m_mutex）─────────────────

bool TaskManager::hasTimeConflict(const Task &task, int excludeId) const {
    for (const auto &t : m_tasks) {
        if (t.id == excludeId) continue;
        if (t.startTime == task.startTime) return true;
    }
    return false;
}

bool TaskManager::hasNameTimeUnique(const Task &task, int excludeId) const {
    for (const auto &t : m_tasks) {
        if (t.id == excludeId) continue;
        if (t.name == task.name && t.startTime == task.startTime) return true;
    }
    return false;
}

// ─── 任务操作 ──────────────────────────────────────────

bool TaskManager::addTask(const Task &task) {
    Task newTask = task;

    // 第一步：加锁校验 + 写入内存
    {
        QMutexLocker locker(&m_mutex);

        if (hasTimeConflict(task) || hasNameTimeUnique(task))
            return false;

        if (newTask.id < 0) {
            newTask.id = m_nextId++;
        }
        m_tasks.append(newTask);
    }
    // 第二步：解锁后写文件，不阻塞提醒线程
    saveTasks();
    return true;
}

bool TaskManager::removeTask(int id) {
    bool found = false;
    {
        QMutexLocker locker(&m_mutex);
        auto it = std::find_if(m_tasks.begin(), m_tasks.end(),
                               [id](const Task &t) { return t.id == id; });
        if (it != m_tasks.end()) {
            m_tasks.erase(it);
            found = true;
        }
    }
    if (found)
        saveTasks();
    return found;
}

Task* TaskManager::findTask(int id) {
    QMutexLocker locker(&m_mutex);
    for (auto &t : m_tasks) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

// ─── 查询（返回拷贝，线程安全）────────────────────────

QVector<Task> TaskManager::allTasks() const {
    QMutexLocker locker(&m_mutex);
    return m_tasks;
}

QVector<Task> TaskManager::tasksForDate(const QDate &date) const {
    QMutexLocker locker(&m_mutex);
    QVector<Task> result;
    for (const auto &t : m_tasks) {
        if (t.startTime.date() == date)
            result.append(t);
    }
    std::sort(result.begin(), result.end(), [](const Task &a, const Task &b) {
        return a.startTime < b.startTime;
    });
    return result;
}

QVector<Task> TaskManager::tasksForMonth(int year, int month) const {
    QMutexLocker locker(&m_mutex);
    QVector<Task> result;
    for (const auto &t : m_tasks) {
        if (t.startTime.date().year() == year
            && t.startTime.date().month() == month)
            result.append(t);
    }
    std::sort(result.begin(), result.end(), [](const Task &a, const Task &b) {
        return a.startTime < b.startTime;
    });
    return result;
}
