#include "TaskManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <algorithm>

// ============================================================
// TaskManager.cpp — 任务管理器实现
// 核心设计：两步操作模式
//   第一步（加锁）：校验 + 修改内存中的 m_tasks
//   第二步（解锁）：将修改后的数据写入磁盘文件
// 为什么分两步：文件 I/O 可能很慢（几十毫秒），如果在锁内写文件，
//   提醒线程在这段时间内无法读取任务列表，导致提醒延迟。
// ============================================================

// ---------- 构造 ----------

TaskManager::TaskManager(const QString &dataDir)
    : m_dataDir(dataDir)
{
    QDir().mkpath(m_dataDir);   // 确保数据目录存在（首次运行时创建）
}

// 拼装当前用户的任务文件路径
QString TaskManager::taskFilePath() const {
    return m_dataDir + "/" + m_currentUser + "_tasks.json";
}

// ---------- 用户切换 ----------

void TaskManager::setCurrentUser(const QString &username) {
    m_currentUser = username;   // 只记录用户名，不加载数据
}

// ---------- 文件加载 ----------

void TaskManager::loadTasks(const QString &username) {
    QMutexLocker locker(&m_mutex);  // 上锁，函数退出时自动解锁

    m_tasks.clear();
    m_currentUser = username;

    // 打开用户专属的任务文件
    QFile file(taskFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        // 首次登录，没有任务文件是正常的，从 ID=1 开始
        m_nextId = 1;
        return;
    }

    // 解析 JSON 数组，逐条反序列化为 Task 对象
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray arr = doc.array();
    for (const auto &val : arr) {
        m_tasks.append(Task::fromJson(val.toObject()));
    }

    // 根据已有任务的最大 ID 恢复 nextId
    // 例如：已有任务 ID 为 1,2,5，则 nextId = 6
    m_nextId = 1;
    for (const auto &t : m_tasks) {
        if (t.id >= m_nextId)
            m_nextId = t.id + 1;
    }
}

// ---------- 文件保存 ----------
// 设计要点：此函数在 addTask/removeTask 的锁外被调用，
// 所以不能直接读取 m_tasks，必须先在锁内拷贝一份快照。

void TaskManager::saveTasks() {
    // 第一步：锁内拷贝快照
    QVector<Task> snapshot;
    {
        QMutexLocker locker(&m_mutex);
        snapshot = m_tasks;
    }   // ← 锁在这里释放

    // 第二步：锁外写文件（可能慢，但不阻塞提醒线程）
    QJsonArray arr;
    for (const auto &t : snapshot) {
        arr.append(t.toJson());
    }

    QJsonDocument doc(arr);
    QFile file(taskFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();   // 显式关闭，确保数据写入磁盘
    }
}

// ---------- 内部校验 ----------
// 以下两个函数都在锁内调用，不需要额外加锁。

// 将时间截断到分钟精度（去掉秒和毫秒）
// 为什么要截断：GUI 的 QDateTimeEdit 包含秒数，用户两次选"14:00"
// 可能实际值是 14:00:23 和 14:00:47，直接比较会认为不同。
// 作业要求精确到分钟，截断后再比较才符合要求。
static QDateTime toMinutePrecision(const QDateTime &dt) {
    QDate d = dt.date();
    QTime t = dt.time();
    QTime truncated(t.hour(), t.minute());   // 只保留时和分
    return QDateTime(d, truncated);
}

// 检查开始时间冲突
bool TaskManager::hasTimeConflict(const Task &task, int excludeId) const {
    QDateTime taskMin = toMinutePrecision(task.startTime);
    for (const auto &t : m_tasks) {
        if (t.id == excludeId) continue;
        if (toMinutePrecision(t.startTime) == taskMin) return true;
    }
    return false;
}

// 检查名称+开始时间的唯一性
bool TaskManager::hasNameTimeConflict(const Task &task, int excludeId) const {
    QDateTime taskMin = toMinutePrecision(task.startTime);
    for (const auto &t : m_tasks) {
        if (t.id == excludeId) continue;
        if (t.name == task.name && toMinutePrecision(t.startTime) == taskMin)
            return true;
    }
    return false;
}

// ---------- 任务操作 ----------

bool TaskManager::addTask(const Task &task) {
    Task newTask = task;

    // 第一步：加锁，做校验和内存修改
    {
        QMutexLocker locker(&m_mutex);

        // 双重校验：开始时间不冲突 + 名称时间组合唯一
        if (hasTimeConflict(task) || hasNameTimeConflict(task))
            return false;   // 锁自动释放

        // 自动分配 ID（调用方传入 -1 表示需要自动分配）
        if (newTask.id < 0) {
            newTask.id = m_nextId++;
        }

        m_tasks.append(newTask);
    }   // ← 锁释放，提醒线程此时已能看到新任务

    // 第二步：解锁后持久化到文件
    saveTasks();
    return true;
}

bool TaskManager::removeTask(int id) {
    bool found = false;

    // 第一步：加锁查找并删除
    {
        QMutexLocker locker(&m_mutex);
        auto it = std::find_if(m_tasks.begin(), m_tasks.end(),
                               [id](const Task &t) { return t.id == id; });
        if (it != m_tasks.end()) {
            m_tasks.erase(it);
            found = true;
        }
    }   // ← 锁释放

    // 第二步：持久化（只在确实删除了任务时才写文件）
    if (found)
        saveTasks();
    return found;
}

bool TaskManager::updateTask(int id, const Task &newData) {
    // 第一步：加锁校验 + 更新
    {
        QMutexLocker locker(&m_mutex);

        // 校验：时间冲突和名称唯一性（排除自身ID）
        if (hasTimeConflict(newData, id) || hasNameTimeConflict(newData, id))
            return false;

        // 查找并覆盖
        for (auto &t : m_tasks) {
            if (t.id == id) {
                t.name       = newData.name;
                t.startTime  = newData.startTime;
                t.priority   = newData.priority;
                t.category   = newData.category;
                t.remindTime = newData.remindTime;
                break;
            }
        }
    }
    // 第二步：持久化
    saveTasks();
    return true;
}

Task* TaskManager::findTask(int id) {
    QMutexLocker locker(&m_mutex);      // 加锁保护遍历
    for (auto &t : m_tasks) {
        if (t.id == id) return &t;      // 注意：返回的指针只在锁内有效！
    }
    return nullptr;
}

// ---------- 查询 ----------

// 返回任务拷贝（而非引用），线程安全
QVector<Task> TaskManager::allTasks() const {
    QMutexLocker locker(&m_mutex);
    return m_tasks;                     // 触发拷贝构造，锁释放后拷贝仍然有效
}

QVector<Task> TaskManager::tasksForDate(const QDate &date) const {
    QMutexLocker locker(&m_mutex);
    QVector<Task> result;
    for (const auto &t : m_tasks) {
        if (t.startTime.date() == date) // date() 只取日期部分，忽略时间
            result.append(t);
    }
    // 按开始时间升序排列
    std::sort(result.begin(), result.end(), [](const Task &a, const Task &b) {
        return a.startTime < b.startTime;
    });
    return result;
}

QVector<Task> TaskManager::tasksForMonth(int year, int month) const {
    QMutexLocker locker(&m_mutex);
    QVector<Task> result;
    for (const auto &t : m_tasks) {
        if (t.startTime.date().year() == year &&
            t.startTime.date().month() == month)
            result.append(t);
    }
    std::sort(result.begin(), result.end(), [](const Task &a, const Task &b) {
        return a.startTime < b.startTime;
    });
    return result;
}
