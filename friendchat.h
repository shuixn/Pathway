#ifndef FRIENDCHAT_H
#define FRIENDCHAT_H

#include <QDialog>
#include <QtNetwork>
#include <QtGui>
#include "tcpclient.h"
#include "tcpserver.h"

namespace Ui {
class FriendChat;
}

enum FMessageType
{
    FMessage,
    FNewParticipant,
    FParticipantLeft,
    FileName,
    Refuse,
    Xchat
};

class FriendChat : public QDialog
{
    Q_OBJECT
public:
    ~FriendChat();
    FriendChat(QString username, QString userip,qint32 port);
    QString frienduserip;
    QString friendusername;

    QUdpSocket *fchat;
    qint32 fport;
    void sendMessage(FMessageType type,QString serverAddress="");
    quint16 a;

    bool is_opened;

public slots:


protected:
    void hasPendingFile(QString userName,QString serverAddress,                 //接收文件
                                QString clientAddress,QString fileName);
    void participantLeft(QString userName,QString localHostName,QString time);  //好友离开
    bool eventFilter(QObject *target, QEvent *event);                           //事件过滤器

private:
    Ui::FriendChat *ui;

    TcpServer *server;

    bool saveFile(const QString& fileName); //保存聊天记录
    QString getMessage();                   //获取发送信息
    QString getIP();                        //获取本地IP
    QString getUserName();                  //获取用户名
    QString message;                        //信息
    QString fileName;                       //文件名

private slots:
    void sentFileName(QString);             //发送文件名
    void sendfile();                        //点击发送文件按钮
    void processPendingDatagrams();         //接收数据
    void send();                            //发送
    void close();                           //关闭
    void clear();                           //清空聊天记录
    void save();                            //保存聊天记录

};

#endif // FRIENDCHAT_H
