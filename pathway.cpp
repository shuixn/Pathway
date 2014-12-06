#include "pathway.h"
#include "ui_pathway.h"
#include "QMessageBox"
#include "QFileDialog"
#include "QScrollBar"
#include <QKeyEvent>
#include <QTimer>

pathway::pathway(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::pathway)
{
    ui->setupUi(this);

    //读取本地好友列表
    XmlOperator("friends.xml");

    //设置鼠标跟踪为真
    setMouseTracking( true );

    ui->closePushButton->setStyleSheet("QPushButton{background-color:rgb(255, 255, 255);\
                                       border-radius:8px;\
                                       border:1px;}\
                                       QPushButton:hover{\
                                           background-color:red;\
                                           color:rgb(255, 255, 255);\
                                       }");
    ui->minPushButton->setStyleSheet("QPushButton{background-color:rgb(255, 255, 255);\
                                       border-radius:8px;\
                                       border:1px;}\
                                       QPushButton:hover{\
                                           background-color:rgb(48, 133, 206);\
                                           color:rgb(255, 255, 255);\
                                       }");
    innerchat = new InnerChat(this);

    //设置窗体无边框
    innerchat->setWindowFlags(Qt::Window|
                     Qt::FramelessWindowHint|
                     Qt::WindowSystemMenuHint|
                     Qt::WindowMinimizeButtonHint|
                     Qt::WindowMaximizeButtonHint
                    );

    connect(innerchat,SIGNAL(sendData(QString,QString,QString)),
            this,SLOT(getData(QString,QString,QString)));

    connect(innerchat,SIGNAL(NewParticipanted()),
            this,SLOT(newparticipant()));
    connect(innerchat,SIGNAL(ParticipantLefted()),
            this,SLOT(participantleft()));

    connect(innerchat,SIGNAL(refuced()),this,SLOT(refuced()));
    connect(innerchat,SIGNAL(addFriend(QString,QString,QString)),
            this,SLOT(addFriend(QString,QString,QString)));
    connect(innerchat,SIGNAL(friendAdded(QString)),
            this,SLOT(friendAdded(QString)));
    connect(innerchat,SIGNAL(newUdpSocket(QString)),
            this,SLOT(newUdpSocket(QString)));

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

pathway::~pathway()
{
    delete ui;
}

void pathway::getData(QString username, QString ipaddress, QString localhostname)
{
    this->newUsername = username;
    this->newIpaddress = ipaddress;
    this->newLocalhostname = localhostname;
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

void pathway::closeEvent(QCloseEvent *)
{

}

//新用户加入
void pathway::newparticipant()
{
    //判断该人是否已存在
    bool bb = ui->peopleTableWidget->findItems(this->newLocalhostname,Qt::MatchExactly).isEmpty();
    //不存在就添加进“附近的人”
    if(bb)
    {
        QTableWidgetItem *user = new QTableWidgetItem(this->newUsername);
        QTableWidgetItem *ip = new QTableWidgetItem(this->newIpaddress);
        QTableWidgetItem *host = new QTableWidgetItem(this->newLocalhostname);
        ui->peopleTableWidget->insertRow(0);
        ui->peopleTableWidget->setItem(0,0,user);
        ui->peopleTableWidget->setItem(0,1,ip);
        ui->peopleTableWidget->setItem(0,2,host);

        ui->peopleLabel->setText(tr("附近的人：%1").arg(ui->peopleTableWidget->rowCount()));

        //如果是好友就高亮显示
        //friendEnter();

        //发送自己在线
        innerchat->sendMessage(NewParticipant);
    }
}

//判断新用户是否是好友并操作
/*
void pathway::friendEnter()
{
    //判断是否存在“我的好友”
    if(ui->friendToolBox->itemText(1) == "我的好友")
    {
        //遍历所有toolbtn
        QObjectList list = children();
        QToolButton *tb;
        foreach (QObject *obj, list) {
            tb = qobject_cast<QToolButton*>(obj);
            if(tb->text() == this->newLocalhostname)
            {
                //设置高亮
                //tb->setToolButtonStyle(Qt::white);
                tb->setEnabled(true);
            }
            else
            {
                tb->setEnabled(false);
            }
        }
    }
}

//好友离开操作
void pathway::friendLeft()
{
    //判断是否存在“我的好友”
    if(ui->friendToolBox->itemText(1) == "我的好友")
    {
        //遍历所有toolbtn
        QObjectList list = children();
        QToolButton *tb;
        foreach (QObject *obj, list) {
            tb = qobject_cast<QToolButton*>(obj);
            if(tb->text() == this->newLocalhostname)
            {
                //设置变暗
                //tb->setToolButtonStyle(Qt::black);
                //设置按钮不可用
                tb->setEnabled(false);

            }
        }
    }
}
*/
//用户离开
void pathway::participantleft()
{
    int rowNum = ui->peopleTableWidget->findItems(this->newLocalhostname,Qt::MatchExactly).first()->row();
    ui->peopleTableWidget->removeRow(rowNum);

    ui->peopleLabel->setText(tr("附近的人：%1").arg(ui->peopleTableWidget->rowCount()));

    //好友离开操作
    //friendLeft();
}

//关闭
void pathway::on_closePushButton_clicked()
{
    switch(QMessageBox::information( this, tr("Pathway温馨提示"),
      tr("确定离开Pathway?"),
      tr("是"), tr("否"),
      0, 1 ) )
     {
        case 0:
        {
            //发送离开
            innerchat->sendMessage(ParticipantLeft);

            //关闭程序
            emit closed();
            break;
        }
        case 1:  
            break;
     }
}

//最小化
void pathway::on_minPushButton_clicked()
{
    pathway::showMinimized();
}

//添加好友
void pathway::on_peopleTableWidget_doubleClicked(QModelIndex index)
{
    if(ui->peopleTableWidget->item(index.row(),0)->text() == innerchat->getUserName() &&
        ui->peopleTableWidget->item(index.row(),1)->text() == innerchat->getIP())
    {
        //查看本人信息
    }
    else
    {
        switch(QMessageBox::information( this, tr("Pathway温馨提示"),
          tr("请求添加 %1?")
                .arg(ui->peopleTableWidget->item(index.row(),2)->text()),
          tr("是"), tr("否"),
          0, 1 ) )
         {
            case 0:
            {
                QString username      = ui->peopleTableWidget->item(index.row(),0)->text();
                QString ip            = ui->peopleTableWidget->item(index.row(),1)->text();
                QString localhostname = ui->peopleTableWidget->item(index.row(),2)->text();

                //进入添加好友列表
                addFriendList.append(username);
                addFriendList.append(ip);
                addFriendList.append(localhostname);

                //发送添加好友
                innerchat->sendMessage(Fadd,ip);

                break;
            }
            case 1:
                break;
         }
    }

}

//读取好友列表
void pathway::XmlOperator(QString fileName){

    if("" == fileName){
        qDebug()<<"Filename is Null";
        return;
    }
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

    groupBox = new QGroupBox();
    layout = new QVBoxLayout(groupBox);
    layout->setAlignment(Qt::AlignLeft);

    //遍历friend节点
    for(int i=0; i < count; i++)
    {
        //new一个toolbtn，在下面设置该toolbtn的参数
        toolBtn = new QToolButton();

        QDomNode dom_node = list.item(i);
        QDomElement element = dom_node.toElement();

        //读取XML并保存到链表
        QDomElement ele = element.firstChild().toElement();
        while(!ele.isNull())
        {
            friendsList.append(ele.text());
            ele = ele.nextSiblingElement();
        }
        //好友名
        QString friendname = element.firstChild().nextSibling().nextSibling().toElement().text();

        //设置按钮文本
        toolBtn->setText(friendname);

        toolBtn->setIcon( QPixmap( ":/images/friend.jpg") );
        toolBtn->setIconSize( QPixmap( ":/images/friend.jpg").size());

        toolBtn->setToolButtonStyle( Qt::ToolButtonTextBesideIcon);

        layout->addWidget(toolBtn);

        connect(toolBtn,SIGNAL(clicked()),this,SLOT(friendInformation()));

     }
     layout->addStretch();

     ui->friendToolBox->addItem((QWidget*)groupBox,tr("我的好友"));

     ui->friendToolBox->setCurrentIndex(1);

     file.close();
}

//点击好友出现好友信息
void pathway::friendInformation()
{
    //获取当前点击toolbutton的指针
    QToolButton *clickedToolBtn = qobject_cast<QToolButton *>(sender());

    QString currentFriendUsername;      // 当前好友用户名
    QString currentFriendLocalName;     // 当前好友主机名
    QString currentFriendIp;            // 当前好友IP
    qint32 currentFriendPort;           // 当前好友端口

    //遍历链表
    for(int i = 0; i < friendsList.size(); ++i) {
        if(clickedToolBtn->text() == friendsList.at(i).toUtf8().data())
        {
            currentFriendLocalName = clickedToolBtn->text();
            currentFriendUsername = friendsList.at(i-2).toUtf8().data();
            currentFriendIp   = friendsList.at(i-1).toUtf8().data();
            currentFriendPort = (qint32)friendsList.at(i+1).toUtf8().data();
        }
    }

    //查看好友信息
    fo = new FriendOperator(currentFriendUsername,
                            currentFriendIp,
                            currentFriendLocalName,
                            currentFriendPort);
    fo->show();
    connect(fo,SIGNAL(reloadXML()),this,SLOT(reloadXML()));
    connect(fo,SIGNAL(fChat(QString)),this,SLOT(fChat(QString)));

}

//重新载入XML
void pathway::reloadXML()
{
    //刷新好友列表
    ui->friendToolBox->removeItem(1);
    XmlOperator("friends.xml");
}

//好友聊天
void pathway::fChat(QString ip)
{
    innerchat->sendMessage(Fchat,ip);
}

//对方拒绝添加
void pathway::refuced()
{
    QMessageBox::warning(0,tr("Pathway温馨提示"),tr("对方拒绝添加！"),QMessageBox::Ok);
}

//同意添加对方
void pathway::addFriend(QString username, QString ipaddress, QString localhostname)
{
    int i               = friendsList.size();
    QString adduser     = username;
    QString addip       = ipaddress;
    QString addhostname = localhostname;
    QString addport     = (QString)((int)friendsList.at(i-1).data() + 1);

    QFile file("friends.xml");
    if (!file.open(QIODevice::ReadOnly)) return;

    QDomDocument doc;
    if (!doc.setContent(&file)){file.close();return;}
    file.close();

    QDomElement root        = doc.documentElement();

    QDomElement newfriend   = doc.createElement(tr("friend"));
    QDomAttr    id          = doc.createAttribute(tr("id"));
    QDomElement newusername = doc.createElement(tr("username"));
    QDomElement newip       = doc.createElement(tr("ip"));
    QDomElement newhostname = doc.createElement(tr("localhostname"));
    QDomElement newport     = doc.createElement(tr("port"));
    QDomText text;

    QString num = root.lastChild().toElement().attribute(tr("id"));
    int count = num.toInt() +1;
    id.setValue(QString::number(count));                //获得了最后一个孩子结点的id，然后加1，便是新的id

    newfriend.setAttributeNode(id);                     //设置好友id

    text = doc.createTextNode(adduser);
    newusername.appendChild(text);                      //用户名
    text = doc.createTextNode(addip);
    newip.appendChild(text);                            //ip
    text = doc.createTextNode(addhostname);
    newhostname.appendChild(text);                      //主机名
    text = doc.createTextNode(addport);
    newport.appendChild(text);                          //端口

    newfriend.appendChild(newusername);                 //插入子节点
    newfriend.appendChild(newip);
    newfriend.appendChild(newhostname);
    newfriend.appendChild(newport);
    root.appendChild(newfriend);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return ;
    QTextStream out(&file);
    doc.save(out,4);                                    //将文档保存到文件，4为子元素缩进字符数
    file.close();

    //重载好友列表
    reloadXML();
}

//被同意添加:插入本地XML
void pathway::friendAdded(QString ipadress)
{
    int i = friendsList.size();
    QString usernameAdded;      // 好友用户名
    QString localNameAdded;     // 好友主机名
    QString ipAdded;            // 好友IP
    QString portAdded = (QString)((int)friendsList.at(i-1).data() + 1);          // 好友端口

    //遍历添加好友列表
    for(int i = 0; i < addFriendList.size(); ++i) {
        if(ipadress == addFriendList.at(i).toUtf8().data())
        {
            usernameAdded  = addFriendList.at(i-1).toUtf8().data();
            ipAdded        = addFriendList.at(i).toUtf8().data();
            localNameAdded = addFriendList.at(i+1).toUtf8().data();
        }
    }

    QFile file("friends.xml");
    if (!file.open(QIODevice::ReadOnly)) return;

    QDomDocument doc;
    if (!doc.setContent(&file)){file.close();return;}
    file.close();

    QDomElement root        = doc.documentElement();

    QDomElement newfriend   = doc.createElement(tr("friend"));
    QDomAttr    id          = doc.createAttribute(tr("id"));
    QDomElement newusername = doc.createElement(tr("username"));
    QDomElement newip       = doc.createElement(tr("ip"));
    QDomElement newhostname = doc.createElement(tr("localhostname"));
    QDomElement newport     = doc.createElement(tr("port"));
    QDomText text;

    QString num = root.lastChild().toElement().attribute(tr("id"));
    int count = num.toInt() +1;
    id.setValue(QString::number(count));                //获得了最后一个孩子结点的id，然后加1，便是新的id

    newfriend.setAttributeNode(id);                     //设置好友id

    text = doc.createTextNode(usernameAdded);
    newusername.appendChild(text);                      //用户名
    text = doc.createTextNode(ipAdded);
    newip.appendChild(text);                            //ip
    text = doc.createTextNode(localNameAdded);
    newhostname.appendChild(text);                      //主机名
    text = doc.createTextNode(portAdded);
    newport.appendChild(text);                          //端口

    newfriend.appendChild(newusername);                 //插入子节点
    newfriend.appendChild(newip);
    newfriend.appendChild(newhostname);
    newfriend.appendChild(newport);
    root.appendChild(newfriend);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return ;
    QTextStream out(&file);
    doc.save(out,4);                                    //将文档保存到文件，4为子元素缩进字符数
    file.close();

    //重载好友列表
    reloadXML();
}

//新建好友udp进行聊天
void pathway::newUdpSocket(QString ip)
{
    QString fLocalHostname;      //和主机名聊天
    QString fIpaddress = ip;
    qint32  fPort;

    //遍历好友列表
    for(int i = 0; i < friendsList.size(); ++i) {
        if(fIpaddress == friendsList.at(i).toUtf8().data())
        {
            fLocalHostname = friendsList.at(i+1).toUtf8().data();
            fPort          = (qint32)friendsList.at(i+2).toUtf8().data();
        }
    }
    //新建好友udp
    friendchat = new FriendChat(fLocalHostname,fIpaddress,fPort);

}

//小区聊天
void pathway::on_innerPushButton_clicked()
{
    innerchat->show();
}
