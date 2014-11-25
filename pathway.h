#ifndef PATHWAY_H
#define PATHWAY_H

#include <QtNetwork>
#include "tcpclient.h"
#include "tcpserver.h"
#include <QMainWindow>
#include <QMouseEvent>

namespace Ui {
class pathway;
}

enum MessageType
{
    Message,                //信息
    NewParticipant,         //新用户
    ParticipantLeft,        //用户离开
    FileName,               //文件名
    Refuse,                 //拒绝接收
    Xchat                   //
};

class pathway : public QMainWindow
{
    Q_OBJECT

public:
    explicit pathway(QWidget *parent = 0);
    ~pathway();

    QString getUserName();
    QString getMessage();

protected:
    //声明移动窗体事件
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void changeEvent(QEvent *e);
    void sendMessage(MessageType type,QString serverAddress="");
    void newParticipant(QString userName,QString localHostName,QString ipAddress);
    void participantLeft(QString userName,QString localHostName,QString time);
    void closeEvent(QCloseEvent *);
    void hasPendingFile(QString userName,QString serverAddress,
                        QString clientAddress,QString fileName);
    //事件过滤器
    bool eventFilter(QObject *target, QEvent *event);

private:
    Ui::pathway *ui;

    //移动窗体参数
    QPoint mLastMousePosition;
    bool mMoving;

    QUdpSocket *udpSocket;
    qint32 port;
    qint32 bb;
    QString fileName;
    TcpServer *server;

    QString getIP();
    //保存聊天记录
    bool saveFile(const QString& fileName);

private slots:
    void on_peopleTableWidget_doubleClicked(QModelIndex index);     //双击附近的人
    void on_clear_clicked();                                        //清除聊天记录
    void on_save_clicked();                                         //
    void on_closePushButton_clicked();                              //关闭
    void on_minPushButton_clicked();                                //最小化
    void on_sendfile_clicked();                                     //发送文件
    void on_send_clicked();                                         //
    void processPendingDatagrams();                                 //接收数据槽函数
    void sentFileName(QString);                                     //发送文件名
};

#endif // PATHWAY_H
