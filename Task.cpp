#include "Task.h"

QJsonObject Task::toJson() const {
    QJsonObject obj;
    obj["id"]        = id;
    obj["name"]      = name;
    obj["startTime"] = startTime.toString(Qt::ISODate); 
    obj["priority"]  = static_cast<int>(priority);
    obj["category"]  = static_cast<int>(category);
    if (remindTime.isValid())         
        obj["remindTime"] = remindTime.toString(Qt::ISODate);
    return obj;
}

Task Task::fromJson(const QJsonObject &obj) {
    Task t;
    t.id        = obj["id"].toInt();
    t.name      = obj["name"].toString();
    t.startTime = QDateTime::fromString(obj["startTime"].toString(), Qt::ISODate);
    t.priority  = static_cast<Priority>(obj["priority"].toInt(1)); 
    t.category  = static_cast<Category>(obj["category"].toInt(2)); 
    QString rt  = obj["remindTime"].toString();
    if (!rt.isEmpty())
        t.remindTime = QDateTime::fromString(rt, Qt::ISODate);
    return t;
}

Priority parsePriority(const QString &s) {
    QString lower = s.toLower();
    if (lower == "low"  || lower == "低" || lower == "l") return Priority::Low;
    if (lower == "high" || lower == "高" || lower == "h") return Priority::High;
    return Priority::Medium;  // 默认"中"
}

Category parseCategory(const QString &s) {
    QString lower = s.toLower();
    if (lower == "study"         || lower == "学习" || lower == "s") return Category::Study;
    if (lower == "entertainment" || lower == "娱乐" || lower == "e") return Category::Entertainment;
    return Category::Life;  // 默认"生活"
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
