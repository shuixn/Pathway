#ifndef FRIENDOPERATOR_H
#define FRIENDOPERATOR_H

#include <QDialog>
#include <QFile>
#include <QDomDocument>

namespace Ui {
class FriendOperator;
}

class FriendOperator : public QDialog
{
    Q_OBJECT

public:
    FriendOperator(QString fusername,QString fipaddress,QString flocalhostname,qint32 fport);

    ~FriendOperator();



    QString fusername,fipaddress,flocalhostname;
    qint32 fport;

private:
    Ui::FriendOperator *ui;
signals:
    void reloadXML();

private slots:
    void removeFriend();
    void on_removePushButton_clicked();
};

#endif // FRIENDOPERATOR_H
