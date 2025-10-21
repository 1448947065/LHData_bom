#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include "mysql.h"
#include <QFormLayout>
#include <QStackedWidget>
#include <mainwindow.h>

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();

signals:
    void check_pwd(QString userName, QString pwd);
    void reg_str(QString reg_usr, QString reg_pwd);  // 新增注册信号

private slots:
    void on_btn_sign_in_clicked();
    void on_btn_sign_up_clicked();
    void on_btnClose_clicked();
    void on_register_clicked();  // 注册按钮点击
    void suc_result(bool result);  // 注册结果处理
    void onRegisterResult(bool success, const QString &msg);
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    Ui::login *ui;
    QString UserName;
    QString PassWord;
    MySql *l_sql;
    bool m_dragging = false;
    QPoint m_dragOffset;
    MainWindow m_ui;
    QStackedWidget *m_stack = nullptr;   // 堆叠容器（登录页/注册页）
    QLineEdit *m_regUser = nullptr;      // 注册页用户名
    QLineEdit *m_regPwd1 = nullptr;      // 注册页密码
    QLineEdit *m_regPwd2 = nullptr;      // 注册页确认密码
    void createRegisterUI();  // 创建注册界面
    void showLoginForm();     // 显示登录表单
    void showRegisterForm();  // 显示注册表单
};

#endif // LOGIN_H
