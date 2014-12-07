#ifndef INNERCHAT_H
#define INNERCHAT_H

#include <QtGui>
#include <QtNetwork>
#include <QDialog>
#include <QString>
#include "pathway.h"
#include <QFile>
#include <QMouseEvent>

namespace Ui {
class InnerChat;
}


enum MessageType
{
    Message,                //信息
    NewParticipant,         //新用户
    ParticipantLeft,        //用户离开
    Fchat,                  //聊天
    Fadd,                   //添加好友
    Fagree,                 //同意添加
    Frefused                //拒绝添加
};

class InnerChat : public QDialog
{
    Q_OBJECT

public:
    explicit InnerChat(QWidget *parent = 0);
    InnerChat(QString);
    ~InnerChat();

    QString time;

    QString userName,localHostName,ipAddress;
    QString addUser,addHostName,addIp;

    QString getUserName();                                  //获取本地用户名
    QString getIP();                                        //获取本地IP
    QString getMessage();                                   //获取要发送的文本信息

    void sendMessage(MessageType type,QString ipAddress="",QString iport="");

    int port;

protected:
    //声明移动窗体事件
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void changeEvent(QEvent *e);

    void closeEvent(QCloseEvent *);
    //事件过滤器
    bool eventFilter(QObject *target, QEvent *event);


private:
    Ui::InnerChat *ui;

    QUdpSocket *udpSocket;

    //移动窗体参数
    QPoint mLastMousePosition;
    bool mMoving;

    //保存文件
    bool saveFile(const QString& fileName);

signals:
    void sendData(QString username,QString ipadress,QString localhostname);

    void NewParticipanted();
    void ParticipantLefted();

    void refuced();
    //同意添加好友
    void addFriend(QString username,QString ipadress,QString localhostname);

    //被同意添加
    void friendAdded(QString ipadress);

    //新建udp好友聊天
    void newUdpSocket(QString ipadress);

private slots:
    void send();                                  //发送
    void processPendingDatagrams();               //接收数据
    void clear();                                 //清除聊天记录
    void save();                                  //保存历史
    void on_closePushButton_clicked();            //关闭
    void on_minPushButton_clicked();              //最小化

};

#endif // INNERCHAT_H
