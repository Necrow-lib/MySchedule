#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("MySchedule日程管理器");

    m_accountMgr = new AccountManager("data");
    m_taskMgr = new TaskManager("data");

    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    setupLoginPage();
    setupMainPage();

    m_stackedWidget->setCurrentWidget(m_loginPage);

    m_reminderThread = new ReminderThread(nullptr, this);
    connect(m_reminderThread, &ReminderThread::remind, this, &MainWindow::showReminder);
    m_reminderThread->start();
}

//退出程序 
MainWindow::~MainWindow()
{
    if (m_reminderThread) {
        m_reminderThread->stop();
        m_reminderThread->quit();
        m_reminderThread->wait();
    }
    delete ui;
}

//登陆/注册页
void MainWindow::setupLoginPage()
{
    m_loginPage = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(m_loginPage);
    vbox->addStretch();

    QFormLayout *form = new QFormLayout;
    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("请输入密码");
    form->addRow("用户名:", m_usernameEdit);
    form->addRow("密码:", m_passwordEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    m_loginBtn = new QPushButton("登录");
    m_registerBtn = new QPushButton("注册");
    btnLayout->addWidget(m_loginBtn);
    btnLayout->addWidget(m_registerBtn);

    m_loginStatus = new QLabel;
    m_loginStatus->setStyleSheet("color: red;");

    vbox->addLayout(form);
    vbox->addLayout(btnLayout);
    vbox->addWidget(m_loginStatus, 0, Qt::AlignCenter);
    vbox->addStretch();

    connect(m_loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_registerBtn, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);

    m_stackedWidget->addWidget(m_loginPage);
}

//主页
void MainWindow::setupMainPage()
{
    m_mainPage = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainPage);

    QHBoxLayout *topLayout = new QHBoxLayout;
    m_userLabel = new QLabel;
    m_logoutBtn = new QPushButton("退出登录");
    topLayout->addWidget(m_userLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_logoutBtn);
    mainLayout->addLayout(topLayout);
    connect(m_logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);

    QGroupBox *addGroup = new QGroupBox("新增任务");
    QFormLayout *addForm = new QFormLayout(addGroup);
    m_taskNameEdit = new QLineEdit;
    m_taskNameEdit->setPlaceholderText("任务名称");
    m_startTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    m_startTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_remindTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    m_remindTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_priorityCombo = new QComboBox;
    m_priorityCombo->addItems({"低", "中", "高"});
    m_priorityCombo->setCurrentIndex(1); // 默认中
    m_categoryCombo = new QComboBox;
    m_categoryCombo->addItems({"学习", "娱乐", "生活"});
    m_categoryCombo->setCurrentIndex(2); // 默认生活
    m_addBtn = new QPushButton("添加任务");
    m_deleteBtn = new QPushButton("删除选中任务");

    addForm->addRow("名称:", m_taskNameEdit);
    addForm->addRow("启动时间:", m_startTimeEdit);
    addForm->addRow("提醒时间:", m_remindTimeEdit);
    addForm->addRow("优先级:", m_priorityCombo);
    addForm->addRow("分类:", m_categoryCombo);
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_deleteBtn);
    addForm->addRow(btnLayout);
    mainLayout->addWidget(addGroup);

    connect(m_addBtn, &QPushButton::clicked, this, &MainWindow::onAddTaskClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteTaskClicked);

    QGroupBox *queryGroup = new QGroupBox("查询任务");
    QHBoxLayout *queryLayout = new QHBoxLayout(queryGroup);
    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_queryDayBtn = new QPushButton("查询当天");
    m_queryMonthBtn = new QPushButton("查询当月");
    queryLayout->addWidget(m_dateEdit);
    queryLayout->addWidget(m_queryDayBtn);
    queryLayout->addWidget(m_queryMonthBtn);
    mainLayout->addWidget(queryGroup);

    connect(m_queryDayBtn, &QPushButton::clicked, this, &MainWindow::onQueryDayClicked);
    connect(m_queryMonthBtn, &QPushButton::clicked, this, &MainWindow::onQueryMonthClicked);

    m_taskTable = new QTableWidget;
    m_taskTable->setColumnCount(6);
    m_taskTable->setHorizontalHeaderLabels({"ID", "名称", "开始时间", "优先级", "分类", "提醒时间"});
    m_taskTable->horizontalHeader()->setStretchLastSection(true);
    m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_taskTable);

    m_stackedWidget->addWidget(m_mainPage);
}


//登陆/注册页
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

//退出登录
void MainWindow::onLogoutClicked()
{
    if (m_reminderThread) {
        m_reminderThread->stop();
        m_reminderThread->quit();
        m_reminderThread->wait();
    }
    m_currentUser.clear();
    m_taskMgr->setCurrentUser("");
    m_stackedWidget->setCurrentWidget(m_loginPage);
    m_usernameEdit->clear();
    m_passwordEdit->clear();
    m_loginStatus->clear();

    // 重新启动提醒线程，无用户不检查
    m_reminderThread = new ReminderThread(nullptr, this);
    connect(m_reminderThread, &ReminderThread::remind, this, &MainWindow::showReminder);
    m_reminderThread->start();
}

//进入主界面
void MainWindow::switchToMainPage(const QString &username)
{
    m_currentUser = username;
    m_userLabel->setText("当前用户: " + username);
    m_taskMgr->loadTasks(username);
    m_stackedWidget->setCurrentWidget(m_mainPage);

    // 重新设置提醒线程
    if (m_reminderThread) {
        m_reminderThread->stop();
        m_reminderThread->quit();
        m_reminderThread->wait();
        delete m_reminderThread;
    }
    m_reminderThread = new ReminderThread(m_taskMgr, this);
    connect(m_reminderThread, &ReminderThread::remind, this, &MainWindow::showReminder);
    m_reminderThread->start();

    // 默认显示今天任务
    loadTableForDate(QDate::currentDate());
}



//主页
//添加任务：未实现🔴
// 添加任务：从界面控件读取数据，构建Task对象，调用TaskManager
void MainWindow::onAddTaskClicked()
{
    if (m_currentUser.isEmpty()) return;

    Task task;
    task.name = m_taskNameEdit->text().trimmed();
    if (task.name.isEmpty()) {
        QMessageBox::warning(this, "错误", "任务名称不能为空");
        return;
    }
    task.startTime  = m_startTimeEdit->dateTime();
    task.remindTime = m_remindTimeEdit->dateTime();
    task.priority   = static_cast<Priority>(m_priorityCombo->currentIndex());
    task.category   = static_cast<Category>(m_categoryCombo->currentIndex());

    if (m_taskMgr->addTask(task)) {
        clearTaskInput();
        loadTableForDate(m_dateEdit->date());
    } else {
        QMessageBox::warning(this, "错误",
            "添加失败：任务的开始时间不能与其他任务相同，且任务名称+开始时间必须唯一");
    }
}

//删除任务：未实现🔴
// 删除任务：获取表格选中行的任务ID，调用TaskManager删除
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

//查询当天任务：未实现🔴
// 查询当天任务
void MainWindow::onQueryDayClicked()
{
    loadTableForDate(m_dateEdit->date());
}

//查询当月任务：未实现🔴
// 查询当月任务
void MainWindow::onQueryMonthClicked()
{
    QDate d = m_dateEdit->date();
    loadTableForMonth(d.year(), d.month());
}

//提醒接收
//显示提醒：未实现🔴
// 显示提醒弹窗（由ReminderThread的信号触发）
void MainWindow::showReminder(const QString &taskName, const QString &timeStr)
{
    QMessageBox::information(this, "任务提醒",
        QString("任务「%1」将在 %2 开始，请做好准备！").arg(taskName, timeStr));
}


// ============================================================
// 以下为内部辅助函数 — 表格填充与输入清空
// ============================================================

// 按日期查询并填充任务表格（调用TaskManager::tasksForDate）
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

// 按月份查询并填充任务表格（调用TaskManager::tasksForMonth）
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

// 清空任务录入表单，恢复到默认值
void MainWindow::clearTaskInput()
{
    m_taskNameEdit->clear();
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_remindTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_priorityCombo->setCurrentIndex(1);   // 默认"中"
    m_categoryCombo->setCurrentIndex(2);   // 默认"生活"
}
