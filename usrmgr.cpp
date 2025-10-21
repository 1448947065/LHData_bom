#include "usrmgr.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QUuid>
#include <QHeaderView>
#include <QDateTime>
#include <QIcon>
#include <QSize>
#include <QApplication>
#include <QStyle>

UsrMgr::UsrMgr(QWidget *parent)
    : QDialog(parent),
      m_model(nullptr),
      m_tableView(nullptr),
      m_addButton(nullptr),
      m_editButton(nullptr),
      m_deleteButton(nullptr),
      m_resetButton(nullptr),
      m_closeButton(nullptr)
{
    setWindowTitle("用户管理");
    setWindowIcon(QIcon(":/icons/user.png")); // 添加窗口图标
    resize(900, 600);
    setupUI();
    if(!setupDatabase()) {
        QMessageBox::critical(this, "数据库错误",
                            "无法连接数据库，部分功能将受限");
        // 禁用依赖数据库的功能按钮
        m_addButton->setEnabled(false);
        m_editButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
        m_resetButton->setEnabled(false);
    } else {
        refreshUserList();
    }
}

UsrMgr::~UsrMgr()
{
    qDebug() << "正在销毁用户管理界面";
    if (m_db.isOpen()) {
        m_db.close();
    }
    QSqlDatabase::removeDatabase(m_db.connectionName());
    delete m_model;
}

bool UsrMgr::setupDatabase()
{
    QString connName = QString("conn_%1").arg(QDateTime::currentMSecsSinceEpoch());
    qDebug() << "正在连接数据库，连接名:" << connName;

    m_db = QSqlDatabase::addDatabase("QMYSQL", connName);
    m_db.setHostName("192.168.1.56");
    m_db.setPort(3306);
    m_db.setUserName("remote_admin");
    m_db.setPassword("123456");
    m_db.setDatabaseName("lhlist");

    if (!m_db.open()) {
        qCritical() << "数据库连接失败:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "数据库连接成功";
    return true;
}

void UsrMgr::setupUI()
{
    // 设置全局样式
    this->setStyleSheet(R"(
        QWidget {
            font-family: "Segoe UI", "Microsoft YaHei";
            font-size: 10pt;
            background-color: #f5f7fa;
        }
        QPushButton {
            background-color: #4a9ff9;
            color: white;
            border-radius: 4px;
            padding: 6px 12px;
            min-width: 80px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #3a8ee6;
        }
        QPushButton:pressed {
            background-color: #2a7dd6;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #888888;
        }
        QTableView {
            background-color: white;
            alternate-background-color: #f9f9f9;
            gridline-color: #e0e0e0;
            border: 1px solid #d1d5db;
            border-radius: 6px;
        }
        QHeaderView::section {
            background-color: #e5e7eb;
            padding: 8px;
            border: none;
            font-weight: 600;
            color: #374151;
        }
        QLineEdit {
            border: 1px solid #d1d5db;
            border-radius: 4px;
            padding: 6px;
            background-color: white;
            font-size: 10pt;
        }
        QLineEdit:focus {
            border: 1px solid #4a9ff9;
        }
        QLabel {
            color: #374151;
            font-weight: 500;
        }
        QDialog {
            background-color: #f5f7fa;
        }
        QCheckBox {
            color: #4b5563;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
    )");

    qDebug() << "setupUI 开始";

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);
    qDebug() << "主布局创建完成";

    // 标题标签
    QLabel *titleLabel = new QLabel("用户管理", this);
    titleLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(titleLabel);

    // 创建表格视图
    m_tableView = new QTableView(this);
    m_tableView->setStyleSheet("QTableView { border: 1px solid #d1d5db; border-radius: 6px; }");
    qDebug() << "表格视图创建完成";

    // 创建按钮
    m_addButton = new QPushButton(QIcon(":/icons/add.png"), "添加用户", this);
    m_editButton = new QPushButton(QIcon(":/icons/edit.png"), "编辑用户", this);
    m_deleteButton = new QPushButton(QIcon(":/icons/delete.png"), "删除用户", this);
    m_resetButton = new QPushButton(QIcon(":/icons/reset.png"), "重置密码", this);
    m_closeButton = new QPushButton(QIcon(":/icons/close.png"), "关闭", this);

    // 设置图标大小
    QSize iconSize(18, 18);
    m_addButton->setIconSize(iconSize);
    m_editButton->setIconSize(iconSize);
    m_deleteButton->setIconSize(iconSize);
    m_resetButton->setIconSize(iconSize);
    m_closeButton->setIconSize(iconSize);

    // 设置按钮样式
    m_closeButton->setStyleSheet("background-color: #ef4444;");
    m_closeButton->setIcon(QIcon(":/icons/close_white.png"));

    qDebug() << "所有按钮创建完成";

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    qDebug() << "按钮布局创建完成";

    // 添加到主布局
    mainLayout->addWidget(m_tableView, 1); // 表格占据剩余空间
    mainLayout->addLayout(buttonLayout);
    qDebug() << "布局设置完成";

    // 连接所有按钮的信号槽
    connect(m_addButton, &QPushButton::clicked, this, &UsrMgr::addUser);
    connect(m_editButton, &QPushButton::clicked, this, &UsrMgr::editUser);
    connect(m_deleteButton, &QPushButton::clicked, this, &UsrMgr::deleteUser);
    connect(m_resetButton, &QPushButton::clicked, this, &UsrMgr::resetPassword);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    qDebug() << "所有信号连接完成";

    qDebug() << "setupUI 结束";
}

void UsrMgr::refreshUserList()
{
    if (m_model) {
        delete m_model;
    }

    m_model = new QSqlTableModel(this, m_db);
    m_model->setTable("users");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置排序
    m_model->setSort(m_model->fieldIndex("username"), Qt::AscendingOrder);

    if (!m_model->select()) {
        QMessageBox::critical(this, "错误", "无法加载用户列表: " + m_model->lastError().text());
        return;
    }

    m_tableView->setModel(m_model);

    // 美化表格
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->verticalHeader()->setDefaultSectionSize(32);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setSortingEnabled(true);

    // 设置更友好的列名
    m_model->setHeaderData(m_model->fieldIndex("username"), Qt::Horizontal, "用户名");
    m_model->setHeaderData(m_model->fieldIndex("created_at"), Qt::Horizontal, "创建时间");
    m_model->setHeaderData(m_model->fieldIndex("last_login"), Qt::Horizontal, "最后登录");

    // 隐藏密码列
    int passwordCol = m_model->fieldIndex("password");
    if (passwordCol >= 0) {
        m_tableView->setColumnHidden(passwordCol, true);
    }

    // 连接选择变化信号
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &UsrMgr::updateButtonStates);
}

bool UsrMgr::validatePassword(const QString &password)
{
    if (password.length() < 8) {
        QMessageBox::warning(this, "密码不符合要求", "密码长度至少需要8个字符");
        return false;
    }

    // 检查密码复杂度：必须包含数字和字母
    bool hasDigit = false;
    bool hasLetter = false;

    for (QChar ch : password) {
        if (ch.isDigit()) hasDigit = true;
        else if (ch.isLetter()) hasLetter = true;
    }

    if (!hasDigit || !hasLetter) {
        QMessageBox::warning(this, "密码不符合要求",
                           "密码必须包含数字和字母组合");
        return false;
    }

    return true;
}

QString UsrMgr::generatePasswordHash(const QString &password, const QString &salt)
{
    QByteArray hash = QCryptographicHash::hash(
        (password + salt).toUtf8(),
        QCryptographicHash::Sha256
    );
    return salt + "|" + hash.toHex();
}

QString UsrMgr::generateSalt()
{
    return QUuid::createUuid().toString().mid(1, 8);
}

void UsrMgr::addUser()
{
    // 创建自定义对话框
    QDialog dialog(this);
    dialog.setWindowTitle("添加用户");
    dialog.setFixedSize(450, 380);

    // 设置对话框样式
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #f8fafc;
        }
        QLabel {
            color: #334155;
            font-weight: 500;
        }
        QLineEdit {
            border: 1px solid #cbd5e1;
            border-radius: 4px;
            padding: 8px;
            background-color: white;
            font-size: 10pt;
        }
        QLineEdit:focus {
            border: 1px solid #4a9ff9;
        }
        QCheckBox {
            color: #475569;
        }
        QPushButton {
            background-color: #4a9ff9;
            color: white;
            border-radius: 4px;
            padding: 8px 16px;
            min-width: 80px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #3a8ee6;
        }
        QPushButton:pressed {
            background-color: #2a7dd6;
        }
        QPushButton#cancelButton {
            background-color: #94a3b8;
        }
        QPushButton#cancelButton:hover {
            background-color: #7c8ba1;
        }
    )");

    QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->setContentsMargins(20, 20, 20, 20);
    dialogLayout->setSpacing(16);

    // 标题
    QLabel *titleLabel = new QLabel("添加新用户", &dialog);
    titleLabel->setStyleSheet("font-size: 12pt; font-weight: bold; color: #1e293b;");
    dialogLayout->addWidget(titleLabel);

    // 表单布局
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // 用户名输入
    QLabel *usernameLabel = new QLabel("用户名:", &dialog);
    QLineEdit *usernameEdit = new QLineEdit(&dialog);
    usernameEdit->setPlaceholderText("请输入用户名");
    formLayout->addRow(usernameLabel, usernameEdit);

    // 密码输入
    QLabel *passwordLabel = new QLabel("密码:", &dialog);
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setPlaceholderText("至少8位，包含数字和字母");
    passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(passwordLabel, passwordEdit);

    // 密码确认
    QLabel *confirmLabel = new QLabel("确认密码:", &dialog);
    QLineEdit *confirmEdit = new QLineEdit(&dialog);
    confirmEdit->setPlaceholderText("请再次输入密码");
    confirmEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(confirmLabel, confirmEdit);

    // 显示密码选项
    QCheckBox *showPasswordCheck = new QCheckBox("显示密码", &dialog);

    // 密码强度指示器
    QLabel *strengthLabel = new QLabel("密码强度: 未设置", &dialog);
    strengthLabel->setStyleSheet("color: #64748b; font-weight: 500;");

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定", &dialog);
    QPushButton *cancelButton = new QPushButton("取消", &dialog);
    cancelButton->setObjectName("cancelButton");
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);

    // 添加到对话框布局
    dialogLayout->addLayout(formLayout);
    dialogLayout->addWidget(showPasswordCheck);
    dialogLayout->addWidget(strengthLabel);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(buttonLayout);

    // 连接信号槽
    QObject::connect(showPasswordCheck, &QCheckBox::toggled, [&](bool checked) {
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        confirmEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    QObject::connect(passwordEdit, &QLineEdit::textChanged, [&](const QString &text) {
        updatePasswordStrength(text, strengthLabel);
    });

    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text();
        QString confirm = confirmEdit->text();

        // 验证输入
        if (username.isEmpty()) {
            QMessageBox::warning(&dialog, "输入错误", "用户名不能为空");
            return;
        }

        if (password.length() < 8) {
            QMessageBox::warning(&dialog, "密码不符合要求", "密码长度至少需要8个字符");
            return;
        }

        // 检查密码复杂度
        bool hasDigit = false;
        bool hasLetter = false;

        for (QChar ch : password) {
            if (ch.isDigit()) hasDigit = true;
            else if (ch.isLetter()) hasLetter = true;
        }

        if (!hasDigit || !hasLetter) {
            QMessageBox::warning(&dialog, "密码不符合要求",
                               "密码必须包含数字和字母组合");
            return;
        }

        if (password != confirm) {
            QMessageBox::warning(&dialog, "密码不匹配", "两次输入的密码不一致");
            return;
        }

        // 检查用户名是否已存在
        QSqlQuery checkQuery(m_db);
        checkQuery.prepare("SELECT 1 FROM users WHERE username = ?");
        checkQuery.addBindValue(username);
        if (!checkQuery.exec() || checkQuery.next()) {
            QMessageBox::warning(&dialog, "添加用户失败", "用户名已存在");
            return;
        }

        // 生成密码哈希
        QString salt = generateSalt();
        QString passwordHash = generatePasswordHash(password, salt);

        // 动态构建INSERT语句
        QStringList columnNames;
        QSqlRecord record = m_db.record("users");
        for (int i = 0; i < record.count(); ++i) {
            columnNames << record.fieldName(i);
        }

        QString insertSql;
        if (columnNames.contains("created_at")) {
            insertSql = "INSERT INTO users (username, password, created_at) VALUES (?, ?, NOW())";
        } else {
            insertSql = "INSERT INTO users (username, password) VALUES (?, ?)";
        }

        // 插入新用户
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare(insertSql);
        insertQuery.addBindValue(username);
        insertQuery.addBindValue(passwordHash);

        if (!insertQuery.exec()) {
            QMessageBox::critical(&dialog, "添加用户失败", "数据库错误: " + insertQuery.lastError().text());
            return;
        }

        QMessageBox::information(&dialog, "成功", "用户添加成功");
        dialog.accept();
    });

    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    // 显示对话框
    if (dialog.exec() == QDialog::Accepted) {
        refreshUserList();
    }
}

void UsrMgr::updatePasswordStrength(const QString &password, QLabel *label)
{
    if (password.isEmpty()) {
        label->setText("密码强度: 未设置");
        label->setStyleSheet("color: #64748b;");
        return;
    }

    int strength = 0;

    // 长度检查
    if (password.length() >= 8) strength++;
    if (password.length() >= 12) strength++;

    // 复杂度检查
    bool hasDigit = false;
    bool hasLower = false;
    bool hasUpper = false;
    bool hasSpecial = false;

    for (QChar ch : password) {
        if (ch.isDigit()) hasDigit = true;
        else if (ch.isLower()) hasLower = true;
        else if (ch.isUpper()) hasUpper = true;
        else if (!ch.isLetterOrNumber()) hasSpecial = true;
    }

    if (hasDigit) strength++;
    if (hasLower) strength++;
    if (hasUpper) strength++;
    if (hasSpecial) strength++;

    QString strengthText;
    QString color;

    if (strength <= 2) {
        strengthText = "密码强度: 弱";
        color = "#ef4444"; // 红色
    } else if (strength <= 4) {
        strengthText = "密码强度: 中";
        color = "#f59e0b"; // 橙色
    } else {
        strengthText = "密码强度: 强";
        color = "#10b981"; // 绿色
    }

    // 添加复杂度提示
    QStringList requirements;
    if (password.length() < 8) requirements << "至少8位";
    if (!hasDigit) requirements << "需要数字";
    if (!hasLower && !hasUpper) requirements << "需要字母";

    if (!requirements.isEmpty()) {
        strengthText += " (" + requirements.join(", ") + ")";
    }

    label->setText(strengthText);
    label->setStyleSheet(QString("color: %1; font-weight: 500;").arg(color));
}

void UsrMgr::editUser()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "未选择用户", "请先选择一个用户");
        return;
    }

    int row = selected.first().row();
    QString oldUsername = m_model->data(m_model->index(row, m_model->fieldIndex("username"))).toString();

    bool ok;
    QString newUsername = QInputDialog::getText(this, "修改用户名",
                                               "新用户名:", QLineEdit::Normal,
                                               oldUsername, &ok);
    if (!ok || newUsername.isEmpty() || newUsername == oldUsername) {
        return;
    }

    // 检查新用户名是否已存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT 1 FROM users WHERE username = ?");
    checkQuery.addBindValue(newUsername);
    if (!checkQuery.exec() || checkQuery.next()) {
        QMessageBox::warning(this, "修改失败", "用户名已存在");
        return;
    }

    // 更新用户名
    QSqlQuery updateQuery(m_db);
    updateQuery.prepare("UPDATE users SET username = ? WHERE username = ?");
    updateQuery.addBindValue(newUsername);
    updateQuery.addBindValue(oldUsername);

    if (!updateQuery.exec()) {
        QMessageBox::critical(this, "修改失败", "数据库错误: " + updateQuery.lastError().text());
        return;
    }

    QMessageBox::information(this, "成功", "用户名修改成功");
    refreshUserList();
}

void UsrMgr::deleteUser()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    int row = selected.first().row();
    QString username = m_model->data(m_model->index(row, m_model->fieldIndex("username"))).toString();

    if (QMessageBox::question(this, "确认删除",
                             QString("确定要删除用户 '%1' 吗？").arg(username),
                             QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QSqlQuery deleteQuery(m_db);
    deleteQuery.prepare("DELETE FROM users WHERE username = ?");
    deleteQuery.addBindValue(username);

    if (!deleteQuery.exec()) {
        QMessageBox::critical(this, "删除失败", "数据库错误: " + deleteQuery.lastError().text());
        return;
    }

    QMessageBox::information(this, "成功", "用户删除成功");
    refreshUserList();
}

void UsrMgr::resetPassword()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "未选择用户", "请先选择一个用户");
        return;
    }

    int row = selected.first().row();
    QString username = m_model->data(m_model->index(row, m_model->fieldIndex("username"))).toString();

    // 创建重置密码对话框 - 增加宽度以容纳完整文本
    QDialog dialog(this);
    dialog.setWindowTitle("重置密码");
    dialog.setFixedSize(520, 350); // 增加宽度以显示完整文本
    dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowCloseButtonHint);

    // 设置对话框样式
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #f8fafc;
            border-radius: 8px;
        }
        QLabel#titleLabel {
            font-size: 14pt;
            font-weight: bold;
            color: #1e293b;
            padding: 10px 0px;
        }
        QLabel {
            color: #374151;
            font-weight: 500;
            min-width: 90px; /* 增加标签宽度 */
        }
        QLineEdit {
            border: 1px solid #d1d5db;
            border-radius: 4px;
            padding: 6px 12px;
            background-color: white;
            font-size: 10pt;
            min-height: 28px;
            max-height: 28px;
        }
        QLineEdit:focus {
            border: 1px solid #3b82f6;
            outline: none;
        }
        QLineEdit::placeholder {
            color: #9ca3af;
            font-size: 9pt; /* 减小占位符字体大小 */
        }
        QCheckBox {
            color: #4b5563;
            spacing: 5px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
        QPushButton {
            border-radius: 4px;
            padding: 6px 16px;
            min-width: 80px;
            font-weight: 500;
            border: none;
        }
        QPushButton#okButton {
            background-color: #3b82f6;
            color: white;
        }
        QPushButton#okButton:hover {
            background-color: #2563eb;
        }
        QPushButton#cancelButton {
            background-color: #9ca3af;
            color: white;
        }
        QPushButton#cancelButton:hover {
            background-color: #6b7280;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(25, 15, 25, 15);
    mainLayout->setSpacing(12);

    // 标题区域
    QLabel *titleLabel = new QLabel(QString("重置用户 '%1' 的密码").arg(username), &dialog);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 添加分隔线
    QFrame *line = new QFrame(&dialog);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #e5e7eb;");
    mainLayout->addWidget(line);

    // 表单区域
    QVBoxLayout *formContainer = new QVBoxLayout();
    formContainer->setSpacing(15);

    // 新密码输入框 - 使用更简洁的提示文本
    QHBoxLayout *newPasswordLayout = new QHBoxLayout();
    QLabel *newPasswordLabel = new QLabel("新密码:", &dialog);
    newPasswordLabel->setFixedWidth(90); // 增加标签宽度
    QLineEdit *newPasswordEdit = new QLineEdit(&dialog);
    newPasswordEdit->setPlaceholderText("8-30位，含字母和数字"); // 简化提示文本
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordLayout->addWidget(newPasswordLabel);
    newPasswordLayout->addWidget(newPasswordEdit);
    formContainer->addLayout(newPasswordLayout);

    // 增加间距
    formContainer->addSpacing(8);

    // 确认密码输入框
    QHBoxLayout *confirmLayout = new QHBoxLayout();
    QLabel *confirmLabel = new QLabel("确认密码:", &dialog);
    confirmLabel->setFixedWidth(90); // 增加标签宽度
    QLineEdit *confirmEdit = new QLineEdit(&dialog);
    confirmEdit->setPlaceholderText("再次输入密码"); // 简化提示文本
    confirmEdit->setEchoMode(QLineEdit::Password);
    confirmLayout->addWidget(confirmLabel);
    confirmLayout->addWidget(confirmEdit);
    formContainer->addLayout(confirmLayout);

    mainLayout->addLayout(formContainer);

    // 显示密码选项
    QCheckBox *showPasswordCheck = new QCheckBox("显示密码", &dialog);
    mainLayout->addWidget(showPasswordCheck);

    // 密码强度指示器 - 使用更简洁的文本
    QLabel *strengthLabel = new QLabel("密码强度: 未设置", &dialog);
    strengthLabel->setStyleSheet("color: #6b7280; font-weight: 500; padding: 5px 0;");
    strengthLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(strengthLabel);

    mainLayout->addSpacing(10);

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    QPushButton *cancelButton = new QPushButton("取消", &dialog);
    cancelButton->setObjectName("cancelButton");
    QPushButton *okButton = new QPushButton("确定", &dialog);
    okButton->setObjectName("okButton");

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    QObject::connect(showPasswordCheck, &QCheckBox::toggled, [&](bool checked) {
        newPasswordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        confirmEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    QObject::connect(newPasswordEdit, &QLineEdit::textChanged, [&](const QString &text) {
        updatePasswordStrength(text, strengthLabel);
    });

    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        QString newPassword = newPasswordEdit->text();
        QString confirm = confirmEdit->text();

        // 验证密码长度
        if (newPassword.length() < 8 || newPassword.length() > 30) {
            // 使用更简洁的错误提示
            QMessageBox msgBox;
            msgBox.setWindowTitle("密码不符合要求");
            msgBox.setText("密码长度必须为8-30位");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStyleSheet("QMessageBox { min-width: 300px; }");
            msgBox.exec();
            return;
        }

        // 检查密码复杂度
        bool hasDigit = false;
        bool hasLetter = false;

        for (QChar ch : newPassword) {
            if (ch.isDigit()) hasDigit = true;
            else if (ch.isLetter()) hasLetter = true;
        }

        if (!hasDigit || !hasLetter) {
            QMessageBox::warning(this, "密码不符合要求",
                               "密码必须包含字母和数字");
            return;
        }

        if (newPassword != confirm) {
            QMessageBox::warning(this, "密码不匹配", "两次输入的密码不一致");
            return;
        }

        // 生成新密码哈希
        QString salt = generateSalt();
        QString passwordHash = generatePasswordHash(newPassword, salt);

        // 更新密码
        QSqlQuery updateQuery(m_db);
        updateQuery.prepare("UPDATE users SET password = ? WHERE username = ?");
        updateQuery.addBindValue(passwordHash);
        updateQuery.addBindValue(username);

        if (!updateQuery.exec()) {
            QMessageBox::critical(this, "重置密码失败",
                                "数据库错误: " + updateQuery.lastError().text());
            return;
        }

        QMessageBox::information(this, "成功", "密码重置成功");
        dialog.accept();
    });

    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        refreshUserList();
    }
}
void UsrMgr::updateButtonStates()
{
    bool hasSelection = !m_tableView->selectionModel()->selectedRows().isEmpty();
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_resetButton->setEnabled(hasSelection);
}
