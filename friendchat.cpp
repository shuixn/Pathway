#include "friendchat.h"
#include "ui_friendchat.h"
#include "QMessageBox"
#include "QFileDialog"
#include "QScrollBar"
#include "QColorDialog"
#include <QMenu>


FriendChat::FriendChat(QString username,QString userip,qint32 port) : ui(new Ui::FriendChat)
{
    ui->setupUi(this);
    ui->lineEdit->setFocusPolicy(Qt::StrongFocus);
    ui->textBrowser->setFocusPolicy(Qt::NoFocus);

    ui->closePushButton->setStyleSheet("QPushButton{background-color:rgb(255, 255, 255);\
                                       border-radius:8px;\
                                       border:1px;}\
                                       QPushButton:hover{\
                                           background-color:red;\
                                           color:rgb(255, 255, 255);\
                                       }");

    ui->lineEdit->setFocus();
    ui->lineEdit->installEventFilter(this);

    //获得菜单，并向上面添加菜单
    QMenu * menu = ui->menuPushButton->getmenu();

    QAction* actionSendFile = menu->addAction("发送文件");
    QAction* actionSave     = menu->addAction("保存历史");
    QAction* actionDelete   = menu->addAction("删除历史");

    connect(actionSendFile,SIGNAL(triggered()),
            this,SLOT(sendfile()));
    connect(actionSave,SIGNAL(triggered()),
            this,SLOT(save()));
    connect(actionDelete,SIGNAL(triggered()),
            this,SLOT(clear()));

    a = 0;
    is_opened = false;

    friendusername = username;
    frienduserip   = userip;

    ui->friendNameLabel->setText(tr("%1").arg(friendusername));
    ui->friendIpLabel->setText(tr("%1").arg(frienduserip));

    //UDP部分
    fchat = new QUdpSocket(this);
    fport = port;

    fchat->bind(QHostAddress(getIP()), fport);

    connect(fchat, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

    //TCP部分
    server = new TcpServer(this);
    connect(server,SIGNAL(sendFileName(QString)),this,SLOT(sentFileName(QString)));

}

FriendChat::~FriendChat()
{
    is_opened = false;
    delete ui;
}

//窗体移动：鼠标按下事件
void FriendChat::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        mMoving = true;
        mLastMousePosition = event->globalPos();
    }
}
//窗体移动：鼠标移动事件
void FriendChat::mouseMoveEvent(QMouseEvent* event)
{
    if( event->buttons().testFlag(Qt::LeftButton) && mMoving)
    {
        this->move(this->pos() + (event->globalPos() - mLastMousePosition));
        mLastMousePosition = event->globalPos();
    }
}
//窗体移动：鼠标松开事件
void FriendChat::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        mMoving = false;
    }
}
bool FriendChat::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->lineEdit)
    {
        if(event->type() == QEvent::KeyPress)//按下键盘某键
        {
             QKeyEvent *k = static_cast<QKeyEvent *>(event);
             if(k->key() == Qt::Key_Return)//回车键
             {
                 send();
                 return true;
             }
        }
    }
    return QWidget::eventFilter(target,event);
}

//处理好友离开
void FriendChat::participantLeft(QString userName,QString localHostName,QString time)
{
    ui->textBrowser->setTextColor(Qt::gray);
    ui->textBrowser->setCurrentFont(QFont("Times New Roman",10));
    ui->textBrowser->append(tr("%1 于 %2 离开！").arg(localHostName).arg(time));

    switch(QMessageBox::information( this, tr("Pathway温馨提示"),
      tr("对方已结束通话，是否保存聊天记录？"),
      tr("是"), tr("否"),
      0, 1 ) )
     {
        case 0:
        {
            save();
            break;
        }
        case 1:

            break;
     }

    a = 1;
    ui->textBrowser->clear();
    this->~FriendChat();
}

 //获取用户名
QString FriendChat::getUserName()
{
    QStringList envVariables;
    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*"
                 << "HOSTNAME.*" << "DOMAINNAME.*";
    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables)
    {
        int index = environment.indexOf(QRegExp(string));
        if (index != -1)
        {

            QStringList stringList = environment.at(index).split('=');
            if (stringList.size() == 2)
            {
                return stringList.at(1);
                break;
            }
        }
    }
    return false;
}

//获取ip地址
QString FriendChat::getIP()
{
    QString localHostName = QHostInfo::localHostName();

    QHostInfo info = QHostInfo::fromName(localHostName);
    foreach(QHostAddress address,info.addresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();
    }
}

//接收文件
void FriendChat::hasPendingFile(QString userName,QString serverAddress,
                            QString clientAddress,QString fileName)
{
    QString ipAddress = getIP();
    if(ipAddress == clientAddress)
    {
        int btn = QMessageBox::information(this,tr("接受文件"),
                                 tr("来自%1(%2)的文件：%3,是否接收？")
                                 .arg(userName).arg(serverAddress).arg(fileName),
                                 QMessageBox::Yes,QMessageBox::No);
        if(btn == QMessageBox::Yes)
        {
            QString name = QFileDialog::getSaveFileName(0,tr("保存文件"),fileName);
            if(!name.isEmpty())
            {
                TcpClient *client = new TcpClient(this);
                client->setFileName(name);
                client->setHostAddress(QHostAddress(serverAddress));
                client->show();

            }

        }
        else{
            sendMessage(Refuse,serverAddress);
        }
    }
}

//接收数据
void FriendChat::processPendingDatagrams()
{
    while(fchat->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(fchat->pendingDatagramSize());
        fchat->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram,QIODevice::ReadOnly);
        int FMessageType;
        in >> FMessageType;
        QString userName,localHostName,ipAddress,messagestr;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch(FMessageType)
        {

            case FMessage:
                {
                    in >>userName >>localHostName >>ipAddress >>messagestr;
                    ui->textBrowser->setTextColor(Qt::black);
                    ui->textBrowser->setCurrentFont(QFont("Times New Roman",12));
                    ui->textBrowser->append("[ " +localHostName+" ] "+ time);       //与主机名聊天中
                    ui->textBrowser->append(messagestr);

                    //收到好友消息后显示
                    this->show();

                    is_opened = true;

                    break;
                }
            case FileName:
                {
                    in >>userName >>localHostName >> ipAddress;
                    QString clientAddress,fileName;
                    in >> clientAddress >> fileName;
                    hasPendingFile(userName,ipAddress,clientAddress,fileName);
                    break;
                }
            case Refuse:
                {
                    in >> userName >> localHostName;
                    QString serverAddress;
                    in >> serverAddress;
                    QString ipAddress = getIP();

                    if(ipAddress == serverAddress)
                    {
                        server->refused();
                    }
                    break;
                }
            case FParticipantLeft:
                {
                    in >>userName >>localHostName;
                    participantLeft(userName,localHostName,time);

                    a = 1;

                    break;
                }
        }
    }
}

//发送文件名
void FriendChat::sentFileName(QString fileName)
{
    this->fileName = fileName;
    sendMessage(FileName);
}

//获得要发送的信息
QString FriendChat::getMessage()
{
    QString msg = ui->lineEdit->text();
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
    return msg;
}

//发送信息
void FriendChat::sendMessage(FMessageType type , QString serverAddress)
{
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    QString localHostName = QHostInfo::localHostName();
    QString address = getIP();
    out << type << getUserName() << localHostName;

    switch(type)
    {
        case FParticipantLeft:
            {
                break;
            }
        case FMessage:
            {
                if(ui->lineEdit->text() == "")
                {
                    QMessageBox::warning(0,tr("警告"),tr("发送内容不能为空"),QMessageBox::Ok);
                    return;
                }
                message = getMessage();
                out << address << message;
                ui->textBrowser->verticalScrollBar()->setValue(ui->textBrowser->verticalScrollBar()->maximum());
                break;
            }
        case FileName:
                {
                    QString clientAddress = frienduserip;
                    out << address << clientAddress << fileName;
                    break;
                }
        case Refuse:
                {
                    out << serverAddress;
                    break;
                }
    }

    fchat->writeDatagram(data,data.length(),QHostAddress(frienduserip), fport);

}

//保存聊天记录
void FriendChat::save()
{
    //获取当前时间
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyyMMdd");

    if(ui->textBrowser->document()->isEmpty())
        QMessageBox::warning(0,tr("Pathway温馨提示"),tr("聊天记录为空"),QMessageBox::Ok);
    else
    {
       //获得文件名
       QString fileName = QFileDialog::getSaveFileName(this,tr("保存聊天记录"),tr("Pathway.%1.%2")
                                                                                .arg(this->friendusername)
                                                                                .arg(current_date)
                                                       ,tr("文本(*.txt);;All File(*.*)"));
       if(!fileName.isEmpty())
           saveFile(fileName);
    }
}

//清空聊天记录
void FriendChat::clear()
{
    ui->textBrowser->clear();
}

//保存文件
bool FriendChat::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly | QFile::Text))

    {
        QMessageBox::warning(this,tr("保存文件"),
        tr("无法保存文件 %1:\n %2").arg(fileName)
        .arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out << ui->textBrowser->toPlainText();

    return true;
}


void FriendChat::closeChat()
{
    switch(QMessageBox::information( this, tr("Pathway温馨提示"),
      tr("确认结束聊天？"),
      tr("是"), tr("否"),
      0, 1 ) )
    {
        case 0:
        {
            switch(QMessageBox::information( this, tr("Pathway温馨提示"),
              tr("是否保存聊天记录？"),
              tr("是"), tr("否"),
              0, 1 ) )
             {
                case 0:
                {
                    save();
                    break;
                }
                case 1:

                    break;
             }
            sendMessage(FParticipantLeft);
            a = 1;
            ui->textBrowser->clear();
            this->~FriendChat();

            break;
        }
        case 1:
        {
            break;
        }
    }

}

//发送
void FriendChat::send()
{
    sendMessage(FMessage);

    QString localHostName = QHostInfo::localHostName();
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    ui->textBrowser->setTextColor(Qt::black);
    ui->textBrowser->setCurrentFont(QFont("Times New Roman",12));
    ui->textBrowser->append("[ " +localHostName+" ] "+ time);
    ui->textBrowser->append(message);

}

//发送文件
void FriendChat::sendfile()
{
    server->show();
    server->initServer();
}

//结束聊天
void FriendChat::on_closePushButton_clicked()
{
    closeChat();
}
