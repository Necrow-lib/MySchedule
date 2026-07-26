#include "Task.h"

// ============================================================
// Task.cpp — 任务序列化实现 + 枚举转换工具
// ============================================================

// ---------- Task 序列化 ----------

// 将任务对象转为 JSON，用于写入文件
// 注意：remindTime 只在有效时才写入，避免 JSON 中出现无意义的空字段
QJsonObject Task::toJson() const {
    QJsonObject obj;
    obj["id"]        = id;
    obj["name"]      = name;
    obj["startTime"] = startTime.toString(Qt::ISODate);   // ISO 格式保证跨平台一致
    obj["priority"]  = static_cast<int>(priority);         // 枚举值直接存为整数
    obj["category"]  = static_cast<int>(category);
    if (remindTime.isValid())                               // 有提醒时间才写入
        obj["remindTime"] = remindTime.toString(Qt::ISODate);
    return obj;
}

// 从 JSON 对象反序列化任务
// 缺省值处理：如果 JSON 中没有某字段，用 toInt(default) 兜底
Task Task::fromJson(const QJsonObject &obj) {
    Task t;
    t.id        = obj["id"].toInt();
    t.name      = obj["name"].toString();
    t.startTime = QDateTime::fromString(obj["startTime"].toString(), Qt::ISODate);
    t.priority  = static_cast<Priority>(obj["priority"].toInt(1));  // 缺省中
    t.category  = static_cast<Category>(obj["category"].toInt(2));  // 缺省生活
    QString rt  = obj["remindTime"].toString();
    if (!rt.isEmpty())
        t.remindTime = QDateTime::fromString(rt, Qt::ISODate);
    return t;
}

// ---------- 枚举 ↔ 字符串工具 ----------
// 支持中英文简写，用户输入 "high"/"高"/"h" 都能识别

Priority parsePriority(const QString &s) {
    QString lower = s.toLower();
    if (lower == "low"  || lower == "低" || lower == "l") return Priority::Low;
    if (lower == "high" || lower == "高" || lower == "h") return Priority::High;
    return Priority::Medium;  // 所有无法识别的输入都当作"中"
}

Category parseCategory(const QString &s) {
    QString lower = s.toLower();
    if (lower == "study"         || lower == "学习" || lower == "s") return Category::Study;
    if (lower == "entertainment" || lower == "娱乐" || lower == "e") return Category::Entertainment;
    return Category::Life;  // 无法识别的输入默认"生活"
}

QString priorityToStr(Priority p) {
    switch (p) {
        case Priority::Low:    return "低";
        case Priority::Medium: return "中";
        case Priority::High:   return "高";
    }
    return "中";
}

QString categoryToStr(Category c) {
    switch (c) {
        case Category::Study:         return "学习";
        case Category::Entertainment: return "娱乐";
        case Category::Life:          return "生活";
    }
    return "生活";
}
