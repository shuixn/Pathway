#include "friendoperator.h"
#include "ui_friendoperator.h"
#include <QFileDialog>
#include <QtXml>
#include <QMessageBox>

FriendOperator::FriendOperator(QString fusername,QString fipaddress,
                               QString flocalhostname,qint32 fport):
    ui(new Ui::FriendOperator)
{
    ui->setupUi(this);

    this->fusername      = fusername;
    this->fipaddress     = fipaddress;
    this->flocalhostname = flocalhostname;
    this->fport          = fport;

    ui->usernameLabel->setText(this->fusername);
    ui->ipLabel->setText(this->fipaddress);
    ui->localhostnameLabel->setText(this->flocalhostname);


    //新建好友窗口
    //friendchat = new FriendChat(currentFriendName,currentFriendIp,currentFriendPort);
    //friendchat->show();
}

FriendOperator::~FriendOperator()
{
    delete ui;
}

//删除好友
void FriendOperator::removeFriend()
{
    switch(QMessageBox::information( this, tr("Pathway温馨提示"),
      tr("确认删除 %1").arg(this->flocalhostname),
      tr("是"), tr("否"),
      0, 1 ) )
     {
        case 0:
        {

            QString fileName = "friends.xml";
            QFile file(fileName);

            if(!file.open(QFile::ReadOnly | QFile::Text))
               qDebug()<<"open file"<<fileName<<"failed, error:"<<file.errorString();

            /*解析Dom节点*/
            QDomDocument    document;
            QString         strError;
            int             errLin = 0, errCol = 0;

            if(!document.setContent(&file, false, &strError, &errLin, &errCol) ) {
                qDebug()<<"parse file failed at line"<<errLin<<",column"<<errCol<<","<<strError;
                file.close();
                return;
            }

            if(document.isNull() ) {
                qDebug()<<"document is null !";
                return;
            }
            //friends节点
            QDomElement root = document.documentElement();

            //所有friend节点
            QDomNodeList list = root.childNodes();
            int count = list.count();

            //遍历friend节点
            for(int i=0; i < count; i++)
            {

                QDomNode dom_node = list.item(i);
                QDomElement element = dom_node.toElement();

                if(this->fipaddress == element.firstChild().nextSibling().toElement().text())
                {
                    //删除结点
                    root.removeChild(list.at(i));
                    //保存更改
                    QFile file(fileName);
                    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return ;
                    QTextStream out(&file);
                    document.save(out,4);
                    file.close();
                }
            }
            //关闭
            this->~FriendOperator();

            //重新载入XML
            emit reloadXML();

            break;
        }
        case 1:
            break;
     }
}

//点击删除
void FriendOperator::on_removePushButton_clicked()
{
    removeFriend();
}
