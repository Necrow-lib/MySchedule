#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>
#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QTableWidget>

#include "AccountManager.h"
#include "TaskManager.h"
#include "ReminderThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onLogoutClicked();
    void onAddTaskClicked();
    void onDeleteTaskClicked();
    void onModifyTaskClicked();
    void onQueryDayClicked();
    void onQueryMonthClicked();
    void showReminder(const QString &taskName, const QString &timeStr);

private:
    Ui::MainWindow *ui;

    QStackedWidget *m_stackedWidget;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_registerBtn;
    QLabel *m_loginStatus;
    QLabel *m_userLabel;
    QPushButton *m_logoutBtn;
    QLineEdit *m_taskNameEdit;
    QDateTimeEdit *m_startTimeEdit;
    QDateTimeEdit *m_remindTimeEdit;
    QComboBox *m_priorityCombo;
    QComboBox *m_categoryCombo;
    QPushButton *m_addBtn;
    QPushButton *m_modifyBtn;
    QPushButton *m_deleteBtn;
    QDateEdit *m_dateEdit;
    QPushButton *m_queryDayBtn;
    QPushButton *m_queryMonthBtn;
    QTableWidget *m_taskTable;

    AccountManager *m_accountMgr;
    TaskManager *m_taskMgr;
    ReminderThread *m_reminderThread;
    QString m_currentUser;
    int m_editingId = -1;

    void switchToMainPage(const QString &username);
    void loadTableForDate(const QDate &date);
    void loadTableForMonth(int year, int month);
    void clearTaskInput();
};

#endif