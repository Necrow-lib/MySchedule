#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>


enum class Priority { Low = 0, Medium = 1, High = 2 };

enum class Category { Study = 0, Entertainment = 1, Life = 2 };

struct Task {
    int  id = -1;                         
    QString name;
    QDateTime startTime; 
    Priority priority = Priority::Medium; 
    Category category = Category::Life;   
    QDateTime remindTime; 

    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject &obj);
};

Priority parsePriority(const QString &s);
Category parseCategory(const QString &s);
QString  priorityToStr(Priority p);
QString  categoryToStr(Category c);

#endif 
