#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("MySchedule日程管理器");

    // 从 UI 文件获取控件指针
    m_stackedWidget = ui->m_stackedWidget;
    m_usernameEdit = ui->m_usernameEdit;
    m_passwordEdit = ui->m_passwordEdit;
    m_loginBtn = ui->m_loginBtn;
    m_registerBtn = ui->m_registerBtn;
    m_loginStatus = ui->m_loginStatus;
    m_userLabel = ui->m_userLabel;
    m_logoutBtn = ui->m_logoutBtn;
    m_taskNameEdit = ui->m_taskNameEdit;
    m_startTimeEdit = ui->m_startTimeEdit;
    m_remindTimeEdit = ui->m_remindTimeEdit;
    m_priorityCombo = ui->m_priorityCombo;
    m_categoryCombo = ui->m_categoryCombo;
    m_addBtn = ui->m_addBtn;
    m_modifyBtn = ui->m_modifyBtn;
    m_deleteBtn = ui->m_deleteBtn;
    m_dateEdit = ui->m_dateEdit;
    m_queryDayBtn = ui->m_queryDayBtn;
    m_queryMonthBtn = ui->m_queryMonthBtn;
    m_taskTable = ui->m_taskTable;

    // 初始化业务对象
    m_accountMgr = new AccountManager("data");
    m_taskMgr = new TaskManager("data");

    // 设置默认显示登录页
    m_stackedWidget->setCurrentWidget(ui->m_loginPage);

    // 连接信号槽
    connect(m_loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_registerBtn, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
    connect(m_logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    connect(m_addBtn, &QPushButton::clicked, this, &MainWindow::onAddTaskClicked);
    connect(m_modifyBtn, &QPushButton::clicked, this, &MainWindow::onModifyTaskClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteTaskClicked);
    connect(m_queryDayBtn, &QPushButton::clicked, this, &MainWindow::onQueryDayClicked);
    connect(m_queryMonthBtn, &QPushButton::clicked, this, &MainWindow::onQueryMonthClicked);

    // 启动提醒线程
    m_reminderThread = new ReminderThread(nullptr, this);
    connect(m_reminderThread, &ReminderThread::remind, this, &MainWindow::showReminder);
    m_reminderThread->start();
}

MainWindow::~MainWindow()
{
    if (m_reminderThread) {
        m_reminderThread->stop();
        m_reminderThread->quit();
        m_reminderThread->wait();
    }
    delete ui;
}

// 登录
void MainWindow::onLoginClicked()
{
    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();
    if (user.isEmpty() || pass.isEmpty()) {
        m_loginStatus->setText("用户名和密码不能为空");
        return;
    }
    if (m_accountMgr->login(user, pass)) {
        m_loginStatus->setText("");
        switchToMainPage(user);
    } else {
        m_loginStatus->setText("登录失败，请检查用户名或密码");
    }
}

// 注册
void MainWindow::onRegisterClicked()
{
    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();
    if (user.isEmpty() || pass.isEmpty()) {
        m_loginStatus->setText("用户名和密码不能为空");
        return;
    }
    if (m_accountMgr->registerUser(user, pass)) {
        m_loginStatus->setText("注册成功，请登录");
        m_passwordEdit->clear();
    } else {
        m_loginStatus->setText("注册失败：用户名已存在");
    }
}

// 退出登录
void MainWindow::onLogoutClicked()
{
    if (m_reminderThread) {
        m_reminderThread->stop();
        m_reminderThread->quit();
        m_reminderThread->wait();
    }
    m_currentUser.clear();
    m_taskMgr->setCurrentUser("");
    m_stackedWidget->setCurrentWidget(ui->m_loginPage);
    m_usernameEdit->clear();
    m_passwordEdit->clear();
    m_loginStatus->clear();

    m_reminderThread = new ReminderThread(nullptr, this);
    connect(m_reminderThread, &ReminderThread::remind, this, &MainWindow::showReminder);
    m_reminderThread->start();
}

// 切换到主页面
void MainWindow::switchToMainPage(const QString &username)
{
    m_currentUser = username;
    m_userLabel->setText("当前用户: " + username);
    m_taskMgr->loadTasks(username);
    m_stackedWidget->setCurrentWidget(ui->m_mainPage);

    if (m_reminderThread) {
        m_reminderThread->stop();
        m_reminderThread->quit();
        m_reminderThread->wait();
        delete m_reminderThread;
    }
    m_reminderThread = new ReminderThread(m_taskMgr, this);
    connect(m_reminderThread, &ReminderThread::remind, this, &MainWindow::showReminder);
    m_reminderThread->start();

    loadTableForDate(QDate::currentDate());
}

// 添加任务
void MainWindow::onAddTaskClicked()
{
    if (m_currentUser.isEmpty()) return;

    Task task;
    task.name = m_taskNameEdit->text().trimmed();
    if (task.name.isEmpty()) {
        QMessageBox::warning(this, "错误", "任务名称不能为空");
        return;
    }
    QDateTime rawStart = m_startTimeEdit->dateTime();
    task.startTime = QDateTime(rawStart.date(),
                     QTime(rawStart.time().hour(), rawStart.time().minute()));
    QDateTime rawRemind = m_remindTimeEdit->dateTime();
    task.remindTime = QDateTime(rawRemind.date(),
                      QTime(rawRemind.time().hour(), rawRemind.time().minute()));
    task.priority   = static_cast<Priority>(m_priorityCombo->currentIndex());
    task.category   = static_cast<Category>(m_categoryCombo->currentIndex());

    if (m_taskMgr->addTask(task)) {
        clearTaskInput();
        loadTableForDate(m_dateEdit->date());
    } else {
        QMessageBox::warning(this, "错误",
            "添加失败：任务的开始时间不能与其他任务相同，任务名称+开始时间必须唯一");
    }
}

// 修改任务
void MainWindow::onModifyTaskClicked()
{
    int row = m_taskTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选中要修改的行");
        return;
    }
    int id = m_taskTable->item(row, 0)->text().toInt();

    if (m_editingId != id) {
        m_editingId = id;
        m_modifyBtn->setText("保存修改");

        Task *t = m_taskMgr->findTask(id);
        if (!t) return;
        m_taskNameEdit->setText(t->name);
        m_startTimeEdit->setDateTime(t->startTime);
        m_remindTimeEdit->setDateTime(t->remindTime.isValid() ? t->remindTime : QDateTime::currentDateTime());
        m_priorityCombo->setCurrentIndex(static_cast<int>(t->priority));
        m_categoryCombo->setCurrentIndex(static_cast<int>(t->category));
        return;
    }

    Task newData;
    newData.id       = id;
    newData.name     = m_taskNameEdit->text().trimmed();
    if (newData.name.isEmpty()) {
        QMessageBox::warning(this, "错误", "任务名称不能为空");
        return;
    }
    QDateTime rawStart = m_startTimeEdit->dateTime();
    newData.startTime  = QDateTime(rawStart.date(),
                         QTime(rawStart.time().hour(), rawStart.time().minute()));
    QDateTime rawRemind = m_remindTimeEdit->dateTime();
    newData.remindTime  = QDateTime(rawRemind.date(),
                          QTime(rawRemind.time().hour(), rawRemind.time().minute()));
    newData.priority    = static_cast<Priority>(m_priorityCombo->currentIndex());
    newData.category    = static_cast<Category>(m_categoryCombo->currentIndex());

    if (m_taskMgr->updateTask(id, newData)) {
        m_editingId = -1;
        m_modifyBtn->setText("修改选中任务");
        clearTaskInput();
        loadTableForDate(m_dateEdit->date());
    } else {
        QMessageBox::warning(this, "错误",
            "修改失败：时间冲突或名称+时间不唯一");
    }
}

// 删除任务
void MainWindow::onDeleteTaskClicked()
{
    int row = m_taskTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选中要删除的行");
        return;
    }
    int id = m_taskTable->item(row, 0)->text().toInt();
    if (m_taskMgr->removeTask(id)) {
        loadTableForDate(m_dateEdit->date());
    }
}

// 查询当天任务
void MainWindow::onQueryDayClicked()
{
    loadTableForDate(m_dateEdit->date());
}

// 查询当月任务
void MainWindow::onQueryMonthClicked()
{
    QDate d = m_dateEdit->date();
    loadTableForMonth(d.year(), d.month());
}

// 显示提醒
void MainWindow::showReminder(const QString &taskName, const QString &timeStr)
{
    QString soundPath = QCoreApplication::applicationDirPath() + "/../sounds/remind.wav";
    PlaySound((LPCWSTR)soundPath.utf16(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);

    QMessageBox::information(this, "任务提醒",
        QString("任务「%1」将在 %2 开始，请做好准备！").arg(taskName, timeStr));
}

// 加载当天任务到表格
void MainWindow::loadTableForDate(const QDate &date)
{
    QVector<Task> tasks = m_taskMgr->tasksForDate(date);
    m_taskTable->setRowCount(tasks.size());
    for (int i = 0; i < tasks.size(); ++i) {
        const Task &t = tasks[i];
        m_taskTable->setItem(i, 0, new QTableWidgetItem(QString::number(t.id)));
        m_taskTable->setItem(i, 1, new QTableWidgetItem(t.name));
        m_taskTable->setItem(i, 2, new QTableWidgetItem(t.startTime.toString("yyyy-MM-dd HH:mm")));
        m_taskTable->setItem(i, 3, new QTableWidgetItem(priorityToStr(t.priority)));
        m_taskTable->setItem(i, 4, new QTableWidgetItem(categoryToStr(t.category)));
        m_taskTable->setItem(i, 5, new QTableWidgetItem(
            t.remindTime.isValid() ? t.remindTime.toString("yyyy-MM-dd HH:mm") : "无"));
    }
}

// 加载当月任务到表格
void MainWindow::loadTableForMonth(int year, int month)
{
    QVector<Task> tasks = m_taskMgr->tasksForMonth(year, month);
    m_taskTable->setRowCount(tasks.size());
    for (int i = 0; i < tasks.size(); ++i) {
        const Task &t = tasks[i];
        m_taskTable->setItem(i, 0, new QTableWidgetItem(QString::number(t.id)));
        m_taskTable->setItem(i, 1, new QTableWidgetItem(t.name));
        m_taskTable->setItem(i, 2, new QTableWidgetItem(t.startTime.toString("yyyy-MM-dd HH:mm")));
        m_taskTable->setItem(i, 3, new QTableWidgetItem(priorityToStr(t.priority)));
        m_taskTable->setItem(i, 4, new QTableWidgetItem(categoryToStr(t.category)));
        m_taskTable->setItem(i, 5, new QTableWidgetItem(
            t.remindTime.isValid() ? t.remindTime.toString("yyyy-MM-dd HH:mm") : "无"));
    }
}

// 清空输入框
void MainWindow::clearTaskInput()
{
    m_taskNameEdit->clear();
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_remindTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_priorityCombo->setCurrentIndex(1);   // 默认"中"
    m_categoryCombo->setCurrentIndex(2);   // 默认"生活"
}