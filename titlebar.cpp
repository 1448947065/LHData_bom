#include "titlebar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QWindow>

static inline QPoint globalPosOf(const QMouseEvent *e) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return e->globalPosition().toPoint();
#else
    return e->globalPos();
#endif
}

TitleBar::TitleBar(QWidget *parent) : QWidget(parent) {
    setFixedHeight(50);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setStyleSheet("background-color:white; color:black;");

    titleLabel = new QLabel(QStringLiteral("LinkHope"), this);
    minBtn = new QPushButton("-", this);
    maxBtn = new QPushButton(QChar(0x25A1), this);  // □
    closeBtn = new QPushButton(QChar(0x00D7), this);  // ×
    minBtn->setToolTip(QStringLiteral("最小化"));
    closeBtn->setToolTip(QStringLiteral("关闭"));

    auto btnStyle =
        "QPushButton { border:none; color:black; background:white; padding:0 10px; }"
        "QPushButton:hover { background-color:#e0e0e0; }"
        "QPushButton:pressed { background-color:#c0c0c0; }";

    minBtn->setStyleSheet(btnStyle);
    maxBtn->setStyleSheet(btnStyle);
    closeBtn->setStyleSheet(btnStyle);

    auto h = new QHBoxLayout(this);
    h->setContentsMargins(8, 0, 0, 0);
    h->setSpacing(0);
    h->addWidget(titleLabel);
    h->addStretch();
    h->addWidget(minBtn);
    h->addWidget(maxBtn);
    h->addWidget(closeBtn);
    setLayout(h);

    connect(minBtn, &QPushButton::clicked, this, &TitleBar::minimizeClicked);
    connect(maxBtn, &QPushButton::clicked, this, &TitleBar::maximizeClicked);
    connect(closeBtn, &QPushButton::clicked, this, &TitleBar::closeClicked);

    setCursor(Qt::ArrowCursor);
}

void TitleBar::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        dragOffset = globalPosOf(e) - window()->frameGeometry().topLeft();
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void TitleBar::mouseMoveEvent(QMouseEvent *e) {
    if (e->buttons() & Qt::LeftButton) {
        window()->move(globalPosOf(e) - dragOffset);
        e->accept();
        return;
    }
    QWidget::mouseMoveEvent(e);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        if (auto w = window()) {
            w->isMaximized() ? w->showNormal() : w->showMaximized();
        }
        e->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(e);
}
void TitleBar::syncMaxButton(bool isMaximized)
{
    if (isMaximized) {
        maxBtn->setText(QStringLiteral("\u2750"));
        maxBtn->setToolTip(QStringLiteral("还原"));
    } else {
        maxBtn = new QPushButton(QChar(0x25A1), this);
        maxBtn->setToolTip(QStringLiteral("最大化"));
    }
}
