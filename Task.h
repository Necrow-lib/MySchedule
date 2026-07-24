#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

enum class Priority { Low = 0, Medium = 1, High = 2 };
enum class Category { Study = 0, Entertainment = 1, Life = 2 };

struct Task {
    int id = -1;                            // 唯一ID（TaskManager 自动分配）
    QString name;                           // 任务名称
    QDateTime startTime;                    // 启动时间（精确到分钟）
    Priority priority = Priority::Medium;   // 缺省：中
    Category category = Category::Life;     // 缺省：生活
    QDateTime remindTime;                   // 提醒时间（<= startTime，可为空）

    // JSON 序列化
    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject &obj);
};

// 枚举 ↔ 字符串工具（避免调用方重复写 switch-case）
Priority parsePriority(const QString &s);
Category parseCategory(const QString &s);
QString  priorityToStr(Priority p);
QString  categoryToStr(Category c);

#endif // TASK_H