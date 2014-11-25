#include "pathway.h"
#include "ui_pathway.h"
#include "QMessageBox"
#include "QFileDialog"
#include "QScrollBar"
#include <QKeyEvent>

pathway::pathway(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::pathway)
{
    ui->setupUi(this);

    //设置鼠标跟踪为真
    setMouseTracking( true );

    ui->textEdit->setFocus();
    //设置完后自动调用其eventFilter函数
    ui->textEdit->installEventFilter(this);

    udpSocket = new QUdpSocket(this);
    port = 45454;
    bb = 0;

    udpSocket->bind(port,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
    //发送自己在线
    sendMessage(NewParticipant);

    server = new TcpServer(this);
    connect(server,SIGNAL(sendFileName(QString)),
            this,SLOT(sentFileName(QString)));

}

//窗体移动：鼠标按下事件
void pathway::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        mMoving = true;
        mLastMousePosition = event->globalPos();
    }
}
//窗体移动：鼠标移动事件
void pathway::mouseMoveEvent(QMouseEvent* event)
{
    if( event->buttons().testFlag(Qt::LeftButton) && mMoving)
    {
        this->move(this->pos() + (event->globalPos() - mLastMousePosition));
        mLastMousePosition = event->globalPos();
    }
}
//窗体移动：鼠标松开事件
void pathway::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        mMoving = false;
    }
}

//接收数据
void pathway::processPendingDatagrams()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram,QIODevice::ReadOnly);
        int messageType;
        in >> messageType;
        QString userName,localHostName,ipAddress,message;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch(messageType)
        {
            case Message:
                {
                    in >>userName >>localHostName >>ipAddress >>message;
                    ui->textBrowser->setTextColor(Qt::blue);
                    ui->textBrowser->setCurrentFont(QFont("Times New Roman",12));
                    ui->textBrowser->append("[ " +localHostName+" ] "+ time);
                    ui->textBrowser->append(message);
                    break;
                }
            case NewParticipant:
                {
                    in >>userName >>localHostName >>ipAddress;
                    newParticipant(userName,localHostName,ipAddress);

                    break;
                }
            case ParticipantLeft:
                {
                    in >>userName >>localHostName;
                    participantLeft(userName,localHostName,time);
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
            case Xchat:
                {
                    in >>userName >>localHostName >>ipAddress;
                    //showxchat(localHostName,ipAddress);//显示与主机名聊天中，不是用户名
                    break;
                }
        }
    }
}

//处理新用户加入
void pathway::newParticipant(QString userName,QString localHostName,QString ipAddress)
{
    bool bb = ui->peopleTableWidget->findItems(localHostName,Qt::MatchExactly).isEmpty();
    if(bb)
    {
        QTableWidgetItem *user = new QTableWidgetItem(userName);
        QTableWidgetItem *host = new QTableWidgetItem(localHostName);
        QTableWidgetItem *ip = new QTableWidgetItem(ipAddress);
        ui->peopleTableWidget->insertRow(0);
        ui->peopleTableWidget->setItem(0,0,user);
        ui->peopleTableWidget->setItem(0,1,ip);
        ui->peopleTableWidget->setItem(0,2,host);
        ui->textBrowser->setTextColor(Qt::gray);
        ui->textBrowser->setCurrentFont(QFont("Times New Roman",10));

        ui->peopleLabel->setText(tr("附近的人：%1").arg(ui->peopleTableWidget->rowCount()));

        sendMessage(NewParticipant);
    }
}

//处理用户离开
void pathway::participantLeft(QString userName,QString localHostName,QString time)
{
    int rowNum = ui->peopleTableWidget->findItems(localHostName,Qt::MatchExactly).first()->row();
    ui->peopleTableWidget->removeRow(rowNum);

    ui->peopleLabel->setText(tr("附近的人：%1").arg(ui->peopleTableWidget->rowCount()));
}

pathway::~pathway()
{
    delete ui;
}

void pathway::changeEvent(QEvent *e)
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
QString pathway::getIP()
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
void pathway::sendMessage(MessageType type, QString serverAddress)
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
                if(ui->textEdit->toPlainText() == "")
                {
                    QMessageBox::warning(0,"警告","发送内容不能为空",QMessageBox::Ok);
                    return;
                }
               out << address << getMessage();
               ui->textBrowser->verticalScrollBar()->setValue(ui->textBrowser->verticalScrollBar()->maximum());
               break;

            }
        case FileName:
            {
                int row = ui->peopleTableWidget->currentRow();
                QString clientAddress = ui->peopleTableWidget->item(row,2)->text();
                out << address << clientAddress << fileName;
                break;
            }
        case Refuse:
            {
                out << serverAddress;
                break;
            }
    }
    //私聊
    udpSocket->writeDatagram(data,data.length(),QHostAddress::Broadcast, port);

}

QString pathway::getUserName()  //获取用户名
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

//获得要发送的信息
QString pathway::getMessage()
{
    QString msg = ui->textEdit->toHtml();

    ui->textEdit->clear();
    ui->textEdit->setFocus();
    return msg;
}

void pathway::closeEvent(QCloseEvent *)
{
    sendMessage(ParticipantLeft);
}

void pathway::sentFileName(QString fileName)
{
    this->fileName = fileName;
    sendMessage(FileName);
}

//接收文件
void pathway::hasPendingFile(QString userName,QString serverAddress,
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

//发送
void pathway::on_send_clicked()
{
    sendMessage(Message);
}

//选择用户发送文件
void pathway::on_sendfile_clicked()
{
    if(ui->peopleTableWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(0,tr("选择用户"),tr("请先从用户列表选择要传送的用户！"),QMessageBox::Ok);
        return;
    }
    server->show();
    server->initServer();
}

//关闭
void pathway::on_closePushButton_clicked()
{
    this->close();
}

//最小化
void pathway::on_minPushButton_clicked()
{

}
bool pathway::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->textEdit)
    {
        if(event->type() == QEvent::KeyPress)//回车键
        {
             QKeyEvent *k = static_cast<QKeyEvent *>(event);
             if(k->key() == Qt::Key_Return)
             {
                 on_send_clicked();
                 return true;
             }
        }
    }
    return QMainWindow::eventFilter(target,event);
}

//保存聊天记录
void pathway::on_save_clicked()
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

//保存文件
bool pathway::saveFile(const QString &fileName)
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

//清空聊天记录
void pathway::on_clear_clicked()
{
    ui->textBrowser->clear();
}

//未完成，双击后出现在主界面进行聊天
void pathway::on_peopleTableWidget_doubleClicked(QModelIndex index)
{
    if(ui->peopleTableWidget->item(index.row(),0)->text() == getUserName() &&
        ui->peopleTableWidget->item(index.row(),2)->text() == getIP())
    {
        QMessageBox::warning(0,tr("警告"),tr("你不可以跟自己聊天！！！"),QMessageBox::Ok);
    }
    else
    {
        //if(!privatechat){

        //privatechat = new chat(ui->peopleTableWidget->item(index.row(),1)->text(), //接收主机名
        //                       ui->peopleTableWidget->item(index.row(),2)->text()) ;//接收用户IP
        //}

        QByteArray data;
        QDataStream out(&data,QIODevice::WriteOnly);
        QString localHostName = QHostInfo::localHostName();
        QString address = getIP();
        out << Xchat << getUserName() << localHostName << address;
        udpSocket->writeDatagram(data,
                                 data.length(),
                                 QHostAddress(ui->peopleTableWidget->item(index.row(),2)->text()),
                                 port
                                );

        //rivatechat->show();
        //privatechat->is_opened = true;
    }

}

