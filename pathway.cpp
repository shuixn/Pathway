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

    connect(innerchat,SIGNAL(sendUserName(QString)),
            this,SLOT(getNewUsername(QString)));
    connect(innerchat,SIGNAL(sendIpaddress(QString)),
            this,SLOT(getNewIpaddress(QString)));
    connect(innerchat,SIGNAL(sendLocalHostname(QString)),
            this,SLOT(getNewLocalHostname(QString)));
    connect(innerchat,SIGNAL(NewParticipanted()),
            this,SLOT(newparticipant()));
    connect(innerchat,SIGNAL(ParticipantLefted()),
            this,SLOT(participantleft()));
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

//获取用户名
void pathway::getNewUsername(QString username)
{
    this->newUsername = username;
}

//获取IP
void pathway::getNewIpaddress(QString ipaddress)
{
    this->newIpaddress = ipaddress;
}

//获取主机名
void pathway::getNewLocalHostname(QString localhostname)
{
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
    //sendMessage(ParticipantLeft);
}

//新用户加入
void pathway::newparticipant()
{
    bool bb = ui->peopleTableWidget->findItems(this->newLocalhostname,Qt::MatchExactly).isEmpty();
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
    }
}

//用户离开
void pathway::participantleft()
{
    int rowNum = ui->peopleTableWidget->findItems(this->newLocalhostname,Qt::MatchExactly).first()->row();
    ui->peopleTableWidget->removeRow(rowNum);

    ui->peopleLabel->setText(tr("附近的人：%1").arg(ui->peopleTableWidget->rowCount()));
}

//关闭
void pathway::on_closePushButton_clicked()
{
    this->close();
}

//最小化
void pathway::on_minPushButton_clicked()
{
    pathway::showMinimized();
}

//查看
void pathway::on_peopleTableWidget_doubleClicked(QModelIndex index)
{
    if(ui->peopleTableWidget->item(index.row(),0)->text() == this->newUsername &&
        ui->peopleTableWidget->item(index.row(),1)->text() == this->newIpaddress)
    {
        QMessageBox::warning(0,tr("警告"),tr("你不可以跟自己聊天！！！"),QMessageBox::Ok);
    }
    else
    {
        //查看
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

        connect(toolBtn,SIGNAL(clicked()),this,SLOT(chat()));

     }
     layout->addStretch();

     ui->friendToolBox->addItem((QWidget*)groupBox,tr("我的好友"));

     file.close();
}

//
void pathway::chat()
{
    //获取当前点击toolbutton的指针
    QToolButton *clickedToolBtn = qobject_cast<QToolButton *>(sender());

    QString currentFriendName;  // 当前好友名
    QString currentFriendIp;    // 当前好友IP
    QString currentFriendPort;  // 当前好友端口

    //遍历链表
    for(int i = 0; i < friendsList.size(); ++i) {
        if(clickedToolBtn->text() == friendsList.at(i).toUtf8().data())
        {
            currentFriendName = clickedToolBtn->text();
            currentFriendIp   = friendsList.at(i-1).toUtf8().data();
            currentFriendPort = friendsList.at(i+1).toUtf8().data();
        }
    }



}

void pathway::on_innerPushButton_clicked()
{
    innerchat->show();
}
