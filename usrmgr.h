#ifndef USRMGR_H
#define USRMGR_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QCryptographicHash>
#include <QUuid>
#include <QHeaderView>
#include <QDebug>
#include <QDateTime>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QIcon>
#include <QStyle>
#include <QRegularExpressionValidator>

class UsrMgr : public QDialog
{
    Q_OBJECT

public:
    explicit UsrMgr(QWidget *parent = nullptr);
    ~UsrMgr();

    bool isDatabaseConnected() const { return m_db.isOpen(); }

private slots:
    void addUser();
        void editUser();
    void deleteUser();
    void resetPassword();
    void updateButtonStates();
protected:
    void updatePasswordStrength(const QString &password, QLabel *label);

private:
    QSqlDatabase m_db;
    QSqlTableModel *m_model = nullptr;
    QTableView *m_tableView = nullptr;
    QPushButton *m_addButton = nullptr;
    QPushButton *m_editButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QPushButton *m_resetButton = nullptr;
    QPushButton *m_closeButton = nullptr;

    void setupUI();
    bool setupDatabase();
    void refreshUserList();
    bool validatePassword(const QString &password);
    QString generatePasswordHash(const QString &password, const QString &salt);
    QString generateSalt();
};

#endif // USRMGR_H
