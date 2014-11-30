#include "qmenubutton.h"

#include <QMenu>

QMenuButton::QMenuButton(QWidget *parent) :
    QPushButton(parent)
{
    menu = new QMenu(this);
    connect(this,SIGNAL(clicked()),this,SLOT(popupmenu()));
}

QMenu *QMenuButton::getmenu()
{
     return menu;
}

void QMenuButton::popupmenu()
{
    //获取按键菜单的坐标
    QPoint pos;

    int x = pos.x();
    int y = pos.y();

    //下拉菜单出现坐标位置
    pos.setX(x - this->geometry().width() - 60);
    pos.setY(y + this->geometry().height());

    menu->exec(this->mapToGlobal(pos));
}
