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

//退出程序：待实现🔴 
MainWindow::~MainWindow()
{

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
//登陆：未实现🔴
void MainWindow::onLoginClicked()
{

}

//注册：未实现🔴
void MainWindow::onRegisterClicked()
{

}

//退出登录：未实现🔴
void MainWindow::onLogoutClicked()
{

}

//进入主界面：未实现🔴
void MainWindow::switchToMainPage(const QString &username)
{

}







//主页
//添加任务：未实现🔴
void MainWindow::onAddTaskClicked()
{

}

//删除任务：未实现🔴
void MainWindow::onDeleteTaskClicked()
{

}

//查询当天任务：未实现🔴
void MainWindow::onQueryDayClicked()
{

}

//查询当月任务：未实现🔴
void MainWindow::onQueryMonthClicked()
{

}

//提醒接收
//显示提醒：未实现🔴
void MainWindow::showReminder(const QString &taskName, const QString &timeStr)
{

}
