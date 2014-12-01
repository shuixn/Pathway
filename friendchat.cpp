#include "friendchat.h"
#include "ui_friendchat.h"
#include "QMessageBox"
#include "QFileDialog"
#include "QScrollBar"
#include "QColorDialog"


FriendChat::FriendChat(QString username,QString userip,qint32 port) : ui(new Ui::FriendChat)
{
    ui->setupUi(this);
    ui->lineEdit->setFocusPolicy(Qt::StrongFocus);
    ui->textBrowser->setFocusPolicy(Qt::NoFocus);

    ui->lineEdit->setFocus();
    ui->lineEdit->installEventFilter(this);

    a = 0;
    is_opened = false;

    friendusername = username;
    frienduserip = userip;

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

//处理用户离开
void FriendChat::participantLeft(QString userName,QString localHostName,QString time)
{
    ui->textBrowser->setTextColor(Qt::gray);
    ui->textBrowser->setCurrentFont(QFont("Times New Roman",10));
    ui->textBrowser->append(tr("%1 于 %2 离开！").arg(userName).arg(time));
}

QString FriendChat::getUserName()  //获取用户名
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

QString FriendChat::getIP()  //获取ip地址
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
       if(address.protocol() == QAbstractSocket::IPv4Protocol) //我们使用IPv4地址
            return address.toString();
    }
       return 0;
}

void FriendChat::hasPendingFile(QString userName,QString serverAddress,  //接收文件
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
            case Xchat:
            {
                break;
            }
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
    qDebug()<<msg;
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
        case FMessage :
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
    if(ui->textBrowser->document()->isEmpty())
        QMessageBox::warning(0,tr("警告"),tr("聊天记录为空，无法保存！"),QMessageBox::Ok);
    else
    {
       //获得文件名
       QString fileName = QFileDialog::getSaveFileName(this,tr("保存聊天记录"),tr("聊天记录"),tr("文本(*.txt);;All File(*.*)"));
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


void FriendChat::close()
{
    sendMessage(FParticipantLeft);
    a = 1;
    ui->textBrowser->clear();

    close();
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
