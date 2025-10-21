#include "login.h"
#include "ui_login.h"
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

login::login(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::login),
      l_sql(new MySql(this))  // 初始化数据库对象
{
    ui->setupUi(this);

    // 原有初始化代码保持不变
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setFixedSize(420, 420);  // 增加高度以容纳注册表单

    const QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    move((screenGeometry.width() - width()) / 2,
         (screenGeometry.height() - height()) / 2);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(0, 0, 0, 80));
    ui->frameCard->setGraphicsEffect(shadow);

    auto *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(300);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    this->setStyleSheet(R"(
        login {
            background: transparent;
        }
        QFrame#frameCard {
            background-color: rgba(255,255,255,245);
            border-radius: 15px;
        }
        QLabel {
            color: #333;
            font-size: 14px;
            font-weight: bold;
        }
        #backButton {
            background-color: #6c757d;
        }
        #backButton:hover {
            background-color: #5a6268;
        }
    )");


    // 创建注册界面
    createRegisterUI();

    // 连接信号槽
    connect(this, &login::reg_str, l_sql, &MySql::logon_register);
    connect(l_sql, &MySql::registerResult, this, &login::onRegisterResult);
    connect(this, &login::check_pwd, l_sql, &MySql::login_check_pwd);
}

login::~login()
{
    delete ui;
}

void login::createRegisterUI()
{
    // 先清空 frameCard 上旧布局（保留 Designer 里的控件指针）
    if (auto *old = ui->frameCard->layout()) {
        delete old;   // Qt 会自动解绑布局项；控件仍然存在
    }

    // 外层：堆叠容器（登录页 / 注册页）
    m_stack = new QStackedWidget(ui->frameCard);
    auto *outer = new QVBoxLayout(ui->frameCard);
    outer->setContentsMargins(16, 16, 16, 16);
    outer->addWidget(m_stack);

    // ================= 登录页（复用 Designer 控件）=================
    QWidget *loginPage = new QWidget;
    auto *loginV = new QVBoxLayout(loginPage);
    loginV->setContentsMargins(24, 20, 24, 20);
    loginV->setSpacing(14);

    auto *title = new QLabel("用户登录", loginPage);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:20px;font-weight:bold;");
    loginV->addWidget(title);

    // 用表单布局让“标签-输入框”成对排布，行距更稳
    auto *formLogin = new QFormLayout;
    formLogin->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLogin->setFormAlignment(Qt::AlignHCenter);
    formLogin->setHorizontalSpacing(16);
    formLogin->setVerticalSpacing(12);

    // 把 Designer 的控件放进来（会自动改父对象为 loginPage）
    formLogin->addRow(ui->lab_name,   ui->line_username);
    formLogin->addRow(ui->label_pwd,  ui->line_password);
    loginV->addLayout(formLogin);

    // 登录/注册按钮（沿用 Designer 按钮，统一高度）
    ui->btn_sign_in->setMinimumHeight(36);
    ui->btn_sign_in->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->btn_sign_up->setMinimumHeight(36);
    ui->btn_sign_up->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    loginV->addWidget(ui->btn_sign_in);
    loginV->addWidget(ui->btn_sign_up);

    // 登录页“关闭”按钮（保留）
    auto *btnCloseLogin = new QPushButton("关闭", loginPage);
    btnCloseLogin->setMinimumHeight(36);
    btnCloseLogin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnCloseLogin->setStyleSheet(
        "QPushButton{background:#e81123;color:#fff;border:none;border-radius:6px;font-weight:bold}"
        "QPushButton:hover{background:#c50f1f}"
    );
    loginV->addWidget(btnCloseLogin);
    connect(btnCloseLogin, &QPushButton::clicked, this, &QDialog::reject);

    // ================= 注册页（代码创建）=================
    QWidget *registerPage = new QWidget;
    auto *regV = new QVBoxLayout(registerPage);
    regV->setContentsMargins(24, 20, 24, 20);
    regV->setSpacing(16);

    auto *regTitle = new QLabel("用户注册", registerPage);
    regTitle->setAlignment(Qt::AlignCenter);
    regTitle->setStyleSheet("font-size:20px;font-weight:bold;");
    regV->addWidget(regTitle);

    // 表单布局
    auto *formReg = new QFormLayout;
    formReg->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formReg->setFormAlignment(Qt::AlignHCenter);
    formReg->setHorizontalSpacing(18);
    formReg->setVerticalSpacing(12);

    m_regUser = new QLineEdit(registerPage);
    m_regPwd1 = new QLineEdit(registerPage);
    m_regPwd2 = new QLineEdit(registerPage);
    m_regUser->setPlaceholderText("请输入用户名");
    m_regPwd1->setPlaceholderText("请输入密码");
    m_regPwd2->setPlaceholderText("请再次输入密码");
    m_regPwd1->setEchoMode(QLineEdit::Password);
    m_regPwd2->setEchoMode(QLineEdit::Password);

    for (auto *e : {m_regUser, m_regPwd1, m_regPwd2}) {
        e->setMinimumWidth(260);
        e->setMinimumHeight(34);
    }

    formReg->addRow(new QLabel("用户名：", registerPage), m_regUser);
    formReg->addRow(new QLabel("密　码：", registerPage), m_regPwd1);
    formReg->addRow(new QLabel("确　认：", registerPage), m_regPwd2);
    regV->addLayout(formReg);

    // 按钮区（只保留“注册 / 返回登录”）
    auto *btnRegister = new QPushButton("注册", registerPage);
    auto *btnBack     = new QPushButton("返回登录", registerPage);
    for (auto *b : {btnRegister, btnBack}) {
        b->setMinimumHeight(36);
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        regV->addWidget(b);
    }
    // 若想底部留一点空白，可打开下一行：
    // regV->addStretch();

    // 放入堆叠并默认显示登录页
    m_stack->addWidget(loginPage);
    m_stack->addWidget(registerPage);
    m_stack->setCurrentWidget(loginPage);

    // 切页
    connect(ui->btn_sign_up, &QPushButton::clicked, this, [=]{
        m_regUser->clear(); m_regPwd1->clear(); m_regPwd2->clear();
        m_stack->setCurrentWidget(registerPage);
        m_regUser->setFocus();
    });
    connect(btnBack, &QPushButton::clicked, this, [=]{
        m_stack->setCurrentWidget(loginPage);
        ui->line_username->setFocus();
    });

    // 注册逻辑
    connect(btnRegister, &QPushButton::clicked, this, [=]{
        const QString u  = m_regUser->text().trimmed();
        const QString p1 = m_regPwd1->text();
        const QString p2 = m_regPwd2->text();
        if (u.isEmpty() || p1.isEmpty()) {
            QMessageBox::warning(this, "提示", "用户名或密码不能为空");
            return;
        }
        if (p1 != p2) {
            QMessageBox::warning(this, "提示", "两次密码不一致");
            m_regPwd2->clear();
            m_regPwd2->setFocus();
            return;
        }
        if (p1.size() < 6) {
            QMessageBox::warning(this, "提示", "密码长度不能少于6位");
            return;
        }
        emit reg_str(u, p1);
    });
}


void login::showLoginForm()
{
    if (m_stack) m_stack->setCurrentIndex(0);
}
void login::showRegisterForm()
{
    if (m_stack) m_stack->setCurrentIndex(1);
}
void login::on_btn_sign_in_clicked()
{
    UserName = ui->line_username->text().trimmed();
    PassWord = ui->line_password->text();

    if (UserName.isEmpty() || PassWord.isEmpty()) {
        QMessageBox::warning(this, "警告", "用户名和密码不能为空");
        return;
    }

    // 原有的登录逻辑
    bool result =  l_sql->login_check_pwd(UserName, PassWord);
    if (result) {
        QMessageBox::information(this, "提示", "成功登录");
        m_ui.updateButtonPermissions();
        this->accept();
    }
    else {
        QMessageBox::warning(this, "警告", "用户密码错误");
    }
}

void login::on_btn_sign_up_clicked()
{
    showRegisterForm();  // 切换到注册界面
}

void login::on_register_clicked()
{
    QWidget *registerWidget = ui->frameCard->findChild<QWidget*>("registerWidget");
    if (!registerWidget) return;

    QLineEdit *usernameEdit = registerWidget->property("usernameEdit").value<QLineEdit*>();
    QLineEdit *passwordEdit = registerWidget->property("passwordEdit").value<QLineEdit*>();
    QLineEdit *confirmPasswordEdit = registerWidget->property("confirmPasswordEdit").value<QLineEdit*>();

    if (!usernameEdit || !passwordEdit || !confirmPasswordEdit) return;

    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    // 输入验证
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "警告", "用户名和密码不能为空");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "警告", "两次输入的密码不一致");
        passwordEdit->clear();
        confirmPasswordEdit->clear();
        passwordEdit->setFocus();
        return;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, "警告", "密码长度不能少于6位");
        return;
    }

    // 发送注册信号
    emit reg_str(username, password);
}

void login::suc_result(bool result)
{
    if (result) {
        QMessageBox::information(this, "提示", "注册成功！");
        showLoginForm();  // 返回登录界面

        // 自动填充用户名到登录框
        QWidget *registerWidget = ui->frameCard->findChild<QWidget*>("registerWidget");
        if (registerWidget) {
            QLineEdit *usernameEdit = registerWidget->property("usernameEdit").value<QLineEdit*>();
            if (usernameEdit) {
                ui->line_username->setText(usernameEdit->text());
                ui->line_password->clear();
                ui->line_username->setFocus();
            }
        }
    } else {
        QMessageBox::warning(this, "警告", "注册失败，用户名可能已存在");
    }
}

// 原有的鼠标事件和关闭按钮代码保持不变
void login::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragOffset = e->globalPos() - frameGeometry().topLeft();
        e->accept();
    }
}

void login::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging && (e->buttons() & Qt::LeftButton)) {
        move(e->globalPos() - m_dragOffset);
        e->accept();
    }
}

void login::mouseReleaseEvent(QMouseEvent *e)
{
    m_dragging = false;
    QDialog::mouseReleaseEvent(e);
}

void login::on_btnClose_clicked()
{
    this->close();
}
void login::onRegisterResult(bool success, const QString &msg)
{
    if (success) {
        QMessageBox::information(this, "提示", msg);

        // 注册成功后自动切回登录界面
        showLoginForm();

        // 自动填充用户名
        QWidget *registerWidget = ui->frameCard->findChild<QWidget*>("registerWidget");
        if (registerWidget) {
            QLineEdit *usernameEdit = registerWidget->findChild<QLineEdit*>("regUsernameEdit");
            if (usernameEdit)
                ui->line_username->setText(usernameEdit->text());
        }
        ui->line_password->clear();
        ui->line_username->setFocus();
    }
    else {
        QMessageBox::warning(this, "注册失败", msg);
    }
}
