#pragma once
#include <QWidget>

class QLabel;
class QPushButton;

class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);
    void syncMaxButton(bool isMaximized);
signals:
    void minimizeClicked();
    void maximizeClicked();
    void closeClicked();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

private:
    QPoint dragOffset;
    QLabel *titleLabel{nullptr};
    QPushButton *minBtn{nullptr};
    QPushButton *maxBtn{nullptr};
    QPushButton *closeBtn{nullptr};
};
