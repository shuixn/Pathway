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
    ParticipantLeft         //用户离开
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

    QString getUserName();                                  //获取本地用户名
    QString getIP();                                        //获取本地IP
    QString getMessage();                                   //获取要发送的文本信息

    void sendMessage(MessageType type,QString serverAddress="");

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

private slots:
    void send();                                  //发送
    void processPendingDatagrams();               //接收数据
    void clear();                                 //清除聊天记录
    void save();                                  //保存历史
    void on_closePushButton_clicked();            //关闭
    void on_minPushButton_clicked();              //最小化

};

#endif // INNERCHAT_H
