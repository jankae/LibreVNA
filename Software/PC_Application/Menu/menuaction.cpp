#include "menuaction.h"

#include <QLabel>
#include <QStyle>
#include <QSizePolicy>

MenuAction::MenuAction(const QString &l, MenuAction::ArrowType arrow)
{
    subline = nullptr;
    auto label = new QLabel(l, this);
    label->setAlignment(Qt::AlignCenter);
    auto labelLayout = new QHBoxLayout();
    if(arrow == ArrowType::Left) {
            auto lIcon = new QLabel(this);
            lIcon->setPixmap(style()->standardIcon(QStyle::SP_ArrowLeft).pixmap(16));
            lIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            labelLayout->addWidget(lIcon);
    }
    labelLayout->addWidget(label);
    if(arrow == ArrowType::Right) {
        auto lIcon = new QLabel(this);
        lIcon->setPixmap(style()->standardIcon(QStyle::SP_ArrowRight).pixmap(16));
        lIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        labelLayout->addWidget(lIcon);
    }
    layout.addLayout(labelLayout);
    setLayout(&layout);
}

void MenuAction::AddSubline(const QString &l)
{
    if(!subline) {
        subline = new QLabel(this);
        subline->setAlignment(Qt::AlignCenter);
        layout.addWidget(subline);
    }
    QFont f( "Arial", 8);
    subline->setFont( f);
    subline->setText(l);
}

void MenuAction::RemoveSubline()
{
    if(subline) {
        layout.removeWidget(subline);
        delete subline;
        subline = nullptr;
    }
}

void MenuAction::userSelected()
{
    emit triggered();
}

void MenuAction::mouseReleaseEvent(QMouseEvent *me)
{
    setFrameStyle(QFrame::Raised | QFrame::Panel);
    MenuItem::mouseReleaseEvent(me);
}

void MenuAction::mousePressEvent(QMouseEvent *)
{
    setFrameStyle(QFrame::Sunken | QFrame::Panel);
}
