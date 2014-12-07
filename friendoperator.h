#ifndef FRIENDOPERATOR_H
#define FRIENDOPERATOR_H

#include <QDialog>
#include <QFile>
#include <QDomDocument>
#include "friendchat.h"

namespace Ui {
class FriendOperator;
}

class FriendOperator : public QDialog
{
    Q_OBJECT

public:
    FriendOperator(QString fusername,
                   QString fipaddress,
                   QString flocalhostname,
                   QString fport,
                   QString iport);

    ~FriendOperator();

    FriendChat *friendchat;

    QString fusername,fipaddress,flocalhostname;
    QString fport,iport;

private:
    Ui::FriendOperator *ui;
signals:
    void reloadXML();
    void fChat(QString ip);

private slots:
    void removeFriend();
    void on_removePushButton_clicked();
    void on_chatPushButton_clicked();
};

#endif // FRIENDOPERATOR_H
