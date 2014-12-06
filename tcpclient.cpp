#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QFileDialog>
#include <QMessageBox>

TcpClient::TcpClient(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    this->setFixedSize(350,180);

    TotalBytes = 0;
    bytesReceived = 0;
    fileNameSize = 0;

    tcpClient = new QTcpSocket(this);
    tcpPort = 6666;

    connect(tcpClient,SIGNAL(readyRead()),
            this,SLOT(readMessage()));
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),
            this,SLOT(displayError(QAbstractSocket::SocketError)));

}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//设置服务器地址并连接服务器
void TcpClient::setHostAddress(QHostAddress address)
{
    hostAddress = address;
    newConnect();
}
void TcpClient::newConnect()
{
    blockSize = 0;
    tcpClient->abort();
    tcpClient->connectToHost(hostAddress,tcpPort);
    time.start();
}

void TcpClient::readMessage()
{
    QDataStream in(tcpClient);
    //设置版本
    in.setVersion(QDataStream::Qt_5_3);

    float useTime = time.elapsed();

    if(bytesReceived <= sizeof(qint64)*2){
        if((tcpClient->bytesAvailable() >= sizeof(qint64)*2) && (fileNameSize == 0)){
            in>>TotalBytes>>fileNameSize;
            bytesReceived += sizeof(qint64)*2;
        }
        if((tcpClient->bytesAvailable() >= fileNameSize) && (fileNameSize != 0)){
            in>>fileName;
            bytesReceived +=fileNameSize;

            if(!localFile->open(QFile::WriteOnly)){
                QMessageBox::warning(this,tr("应用程序"),tr("无法读取文件 %1:\n%2.")
                                     .arg(fileName).arg(localFile->errorString()));
                return;
            }
        }else{
            return;
        }
    }
    if(bytesReceived < TotalBytes){
        bytesReceived += tcpClient->bytesAvailable();
        inBlock = tcpClient->readAll();
        localFile->write(inBlock);
        inBlock.resize(0);
    }

    ui->progressBar->setMaximum(TotalBytes);
    ui->progressBar->setValue(bytesReceived);

    double speed = bytesReceived / useTime;

    ui->tcpClientStatusLabel->setText(tr("已接收 %1MB (%2MB/s) \n共%3MB 已用时:%4秒\n估计剩余时间：%5秒")
                                   .arg(bytesReceived / (1024*1024))                    //已接收
                                   .arg(speed*1000/(1024*1024),0,'f',2)                 //速度
                                   .arg(TotalBytes / (1024 * 1024))                     //总大小
                                   .arg(useTime/1000,0,'f',0)                           //用时
                                   .arg(TotalBytes/speed/1000 - useTime/1000,0,'f',0)); //剩余时间
    //接收完毕
    if(bytesReceived == TotalBytes)
    {
        tcpClient->close();
        ui->tcpClientStatusLabel->setText(tr("接收文件 %1 完毕").arg(fileName));

        //关闭文件
        localFile->close();
    }
}

//错误处理
void TcpClient::displayError(QAbstractSocket::SocketError socketError)
{
    switch(socketError)
    {
        case QAbstractSocket::RemoteHostClosedError : break;
        default : qDebug() << tcpClient->errorString();
    }
}

//关闭
void TcpClient::on_tcpClientCloseBtn_clicked()
{
    tcpClient->abort();
    this->close();
}

//取消
void TcpClient::on_tcpClientCancleBtn_clicked()
{
   tcpClient->abort();
}
