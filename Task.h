#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

// ============================================================
// Task.h — 任务数据结构定义
// 职责：纯数据层，不含业务逻辑，只定义"任务是什么"和"怎么序列化"
// ============================================================

// 优先级枚举（int 值对应 JSON 存储的数字，缺省为 Medium=1）
enum class Priority { Low = 0, Medium = 1, High = 2 };

// 分类枚举（int 值对应 JSON 存储的数字，缺省为 Life=2）
enum class Category { Study = 0, Entertainment = 1, Life = 2 };

// 任务数据结构 — 纯数据容器，不含任何业务逻辑
// 所有字段都是 public，直接访问即可
struct Task {
    int  id = -1;                          // 任务唯一ID，由 TaskManager 自动分配，-1 表示尚未分配
    QString name;                          // 任务名称（不能为空）
    QDateTime startTime;                   // 开始时间，精确到分钟
    Priority priority = Priority::Medium;  // 优先级，默认"中"
    Category category = Category::Life;    // 分类，默认"生活"
    QDateTime remindTime;                  // 提醒时间，允许为空（isValid()==false 表示不需要提醒）

    // --- JSON 序列化（用于文件持久化）---
    QJsonObject toJson() const;            // 将任务转为 JSON 对象
    static Task fromJson(const QJsonObject &obj);  // 从 JSON 对象构造任务
};

// --- 枚举 ↔ 字符串工具函数 ---
// 为什么抽出来：mainwindow.cpp 中多处需要把 Priority/Category 转成中文显示，
// 如果到处写 switch-case 会导致代码重复且难维护。统一放在这里，一处修改全局生效。
Priority parsePriority(const QString &s);  // 字符串 → 枚举（支持中英文简写）
Category parseCategory(const QString &s);
QString  priorityToStr(Priority p);       // 枚举 → 中文显示字符串
QString  categoryToStr(Category c);

#endif // TASK_H
