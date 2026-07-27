#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    void onModifyTaskClicked();   // 修改任务：选中行 → 回填 → 改完保存
    void onQueryDayClicked();
    void onQueryMonthClicked();
    void showReminder(const QString &taskName, const QString &timeStr);

private:
    Ui::MainWindow *ui;

    // 页面
    QStackedWidget *m_stackedWidget;
    QWidget *m_loginPage;
    QWidget *m_mainPage;

    // 登录页控件
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_registerBtn;
    QLabel *m_loginStatus;

    // 主页控件
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

    // 核心
    AccountManager *m_accountMgr;
    TaskManager *m_taskMgr;
    ReminderThread *m_reminderThread;
    QString m_currentUser;
    int m_editingId = -1;   // 正在修改的任务ID，-1 表示没有在修改

    void setupLoginPage();
    void setupMainPage();
    void switchToMainPage(const QString &username);
    void loadTableForDate(const QDate &date);
    void loadTableForMonth(int year, int month);
    void clearTaskInput();
};

#endif
