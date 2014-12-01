#include "innerchat.h"
#include "ui_innerchat.h"
#include "QScrollBar"
#include "QFileDialog"
#include <QMessageBox>
#include <QKeyEvent>
#include <QMenu>


InnerChat::InnerChat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InnerChat)
{
    ui->setupUi(this);

    ui->closePushButton->setStyleSheet("QPushButton{background-color:rgb(177, 177, 177);\
                                       border-radius:8px;\
                                       border:1px;}\
                                       QPushButton:hover{\
                                           background-color:red;\
                                           color:rgb(255, 255, 255);\
                                       }");
    ui->minPushButton->setStyleSheet("QPushButton{background-color:rgb(177, 177, 177);\
                                       border-radius:8px;\
                                       border:1px;}\
                                       QPushButton:hover{\
                                           background-color:rgb(48, 133, 206);\
                                           color:rgb(255, 255, 255);\
                                       }");
    ui->menuPushButton->setStyleSheet("QPushButton{background-color:rgb(229, 229, 229);\
                                       border:1px;}\
                                       QPushButton:hover{\
                                           background-color:rgb(48, 133, 206);\
                                           color:rgb(255, 255, 255);\
                                       }");

    //设置鼠标跟踪为真
    setMouseTracking( true );

    //设置完后自动调用其eventFilter函数
    ui->lineEdit->installEventFilter(this);

    //获得菜单，并向上面添加菜单
    QMenu * menu = ui->menuPushButton->getmenu();

    QAction* actionSave = menu->addAction("保存历史");
    QAction* actionDelete = menu->addAction("删除历史");
    QAction* actionAbout = menu->addAction("关于小区");

    connect(actionSave,SIGNAL(triggered()),
            this,SLOT(save()));
    connect(actionDelete,SIGNAL(triggered()),
            this,SLOT(clear()));

    udpSocket = new QUdpSocket(this);
    port = 54545;                                       //网内端口

    udpSocket->bind(port,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));

    //发送自己在线
    sendMessage(NewParticipant);

}
InnerChat::InnerChat(QString)
{

}

InnerChat::~InnerChat()
{
    delete ui;
}
//窗体移动：鼠标按下事件
void InnerChat::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        mMoving = true;
        mLastMousePosition = event->globalPos();
    }
}
//窗体移动：鼠标移动事件
void InnerChat::mouseMoveEvent(QMouseEvent* event)
{
    if( event->buttons().testFlag(Qt::LeftButton) && mMoving)
    {
        this->move(this->pos() + (event->globalPos() - mLastMousePosition));
        mLastMousePosition = event->globalPos();
    }
}
//窗体移动：鼠标松开事件
void InnerChat::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        mMoving = false;
    }
}

//接收数据
void InnerChat::processPendingDatagrams()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram,QIODevice::ReadOnly);
        QString message;
        int messageType;
        time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        in >> messageType;

        switch(messageType)
        {
            case Message:
                {
                    in >>this->userName>>this->localHostName>>this->ipAddress >>message;
                    ui->textBrowser->setTextColor(Qt::black);
                    ui->textBrowser->setCurrentFont(QFont("Times New Roman",12));
                    ui->textBrowser->append("[ " +localHostName+" ] "+ time);
                    ui->textBrowser->append(message);
                    break;
                }
            case NewParticipant:
                {

                    in >>this->userName>>this->localHostName>>this->ipAddress;

                    emit sendData(this->userName,this->ipAddress,this->localHostName);

                    emit NewParticipanted();

                    break;
                }
            case ParticipantLeft:
                {
                    in >>this->userName >>this->localHostName;

                    emit sendData(this->userName,"",this->localHostName);
                    emit ParticipantLefted();

                    break;
                }

        }
    }
}


void InnerChat::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//获取ip地址
QString InnerChat::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
       if(address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();
    }
    return 0;
}

//发送信息
void InnerChat::sendMessage(MessageType type, QString serverAddress)
{
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    QString localHostName = QHostInfo::localHostName();
    QString address = getIP();
    out << type << getUserName() << localHostName;

    switch(type)
    {
        case ParticipantLeft:
            {
                break;
            }
        case NewParticipant:
            {
                out << address;
                break;
            }

        case Message :
            {
                if(ui->lineEdit->text() == "")
                {
                    QMessageBox::warning(0,"警告","发送内容不能为空",QMessageBox::Ok);
                    return;
                }
               out << address << getMessage();
               ui->textBrowser->verticalScrollBar()->setValue(ui->textBrowser->verticalScrollBar()->maximum());
               break;

            }

    }
    udpSocket->writeDatagram(data,data.length(),QHostAddress::Broadcast, port);

}

QString InnerChat::getUserName()  //获取用户名
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
//发送
void InnerChat::send()
{
    sendMessage(Message);
}

//获得要发送的信息
QString InnerChat::getMessage()
{
    QString msg = ui->lineEdit->text();

    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
    return msg;
}

void InnerChat::closeEvent(QCloseEvent *)
{

}

//关闭
void InnerChat::on_closePushButton_clicked()
{
    this->hide();
}

//最小化
void InnerChat::on_minPushButton_clicked()
{
    this->hide();
}
bool InnerChat::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->lineEdit)
    {
        if(event->type() == QEvent::KeyPress)//回车键
        {
             QKeyEvent *k = static_cast<QKeyEvent *>(event);
             if(k->key() == Qt::Key_Return)
             {
                 send();
                 return true;
             }
        }
    }
    return QDialog::eventFilter(target,event);
}
//保存文件
bool InnerChat::saveFile(const QString &fileName)
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

//保存聊天记录
void InnerChat::save()
{
    if(ui->textBrowser->document()->isEmpty())
        QMessageBox::warning(0,tr("警告"),tr("聊天记录为空，无法保存！"),QMessageBox::Ok);
    else
    {
       //获得文件名,注意getSaveFileName函数的格式即可
       QString fileName = QFileDialog::getSaveFileName(this,tr("保存聊天记录"),tr("聊天记录"),tr("文本(*.txt);;All File(*.*)"));
       if(!fileName.isEmpty())
           saveFile(fileName);
    }
}

//清空聊天记录
void InnerChat::clear()
{
    ui->textBrowser->clear();
}


