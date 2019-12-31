#include "ui_mainwindow.h"
#include "ui_managelines.h"
#include "mainwindow.h"

#include <synchapi.h>
#include <QMovie>
#include <QGraphicsItem>
#include <QMessageBox>
#include <QColorDialog>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    myView = new Graphics_view_zoom(ui->graphicsView);
    myView->set_modifiers(Qt::NoModifier);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    scene=new QGraphicsScene;
    scene->setSceneRect(-LINE_INFO_WIDTH,0,SCENE_WIDTH,SCENE_HEIGHT);
    ui->graphicsView->setScene(scene);
    //ui->graphicsView->setStyleSheet("background-image:url(:/icon/icon/bg.png)");
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    initStatusBar();
    manageLines=new ManageLines(this);
    gradenGraph=new GradenGraph;
    appHelp=new AppHelp();
    bool flag = gradenGraph->readFileData(":/data/data/outLine.txt");
    if (!flag)
    {
        QMessageBox box;
        box.setWindowTitle(tr("error information"));
        box.setIcon(QMessageBox::Warning);
        box.setText("读取数据错误!\n将无法展示内置路线！");
        box.addButton(tr("确定"), QMessageBox::AcceptRole);
        if (box.exec() == QMessageBox::Accepted)
        {
            box.close();
        }
    }
    myConnect();
    updateTranserQueryInfo();
    on_actionLineMap_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete myView;
    delete scene;
    delete gradenGraph;
    delete manageLines;
    delete appHelp;
}

//连接信号和槽函数
void MainWindow::myConnect()
{
    connect(manageLines->ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabWidgetCurrentChanged(int)));
    connect(manageLines->ui->pushButtonAddLine, SIGNAL(clicked()), this, SLOT(addLine()));
    connect(manageLines->ui->pushButtonAddSpot, SIGNAL(clicked()), this, SLOT(addSpot()));
    connect(manageLines->ui->pushButtonConnect, SIGNAL(clicked()), this, SLOT(addConnection()));
    connect(manageLines->ui->pushButtonAddByText, SIGNAL(clicked()), this, SLOT(addByText()));
    connect(ui->comboBoxStartLine, SIGNAL(currentIndexChanged(QString)), this, SLOT(transferStartLineChanged(QString)));
    connect(ui->comboBoxDstLine, SIGNAL(currentIndexChanged(QString)), this, SLOT(transferDstLineChanged(QString)));
    connect(ui->pushButtonTransfer, SIGNAL(clicked()), this, SLOT(transferQuery()));
    QTimer *timer = new QTimer(this);//新建定时器
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));//关联定时器计满信号和相应的槽函数
    timer->start(1000);//定时器开始计时，其中1000表示1000ms即1秒
}

//时间更新槽函数
void MainWindow::timerUpdate()
{
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm:ss dddd");
    statusLabel2->setText(str);
}

//初始状态栏
void MainWindow::initStatusBar()
{
    QStatusBar* bar = ui->statusBar;
    statusLabel1 = new QLabel;
    statusLabel1->setMinimumSize(200,15);
    statusLabel1->setFrameShape(QFrame::Box);
    statusLabel1->setFrameShadow(QFrame::Sunken);

    statusLabel2 = new QLabel;
    statusLabel2->setMinimumSize(200,15);
    statusLabel2->setFrameShape(QFrame::Box);
    statusLabel2->setFrameShadow(QFrame::Sunken);

    statusLabel3 = new QLabel;
    statusLabel3->setMinimumSize(200,15);
    statusLabel3->setFrameShape(QFrame::Box);
    statusLabel3->setFrameShadow(QFrame::Sunken);

    bar->addWidget(statusLabel1);
    bar->addWidget(statusLabel2);
    bar->addWidget(statusLabel3);

    statusLabel1->setText(tr("制作者：马浩洋"));
    statusLabel2->setText(tr("0000-00-00 00:00::00 星期 "));
    statusLabel3->setText(tr("欢迎使用北京世园会游览导航"));
}

//由路线表计算混合颜色
QColor MainWindow::getLinesColor(const QList<int>& linesList)
{
    QColor color1=QColor(255,255,255);
    QColor color2;
    for (int i=0; i<linesList.size(); ++i)
    {
        color2=gradenGraph->getLineColor(linesList[i]);
        color1.setRed(color1.red()*color2.red()/255);
        color1.setGreen(color1.green()*color2.green()/255);
        color1.setBlue(color1.blue()*color2.blue()/255);
    }
    return color1;
}

//获得路线表的名字集
QString MainWindow::getLinesName(const QList<int>& linesList)
{
    QString str;
    str+="\t";
    for (int i=0; i<linesList.size(); ++i)
    {
        str+=" ";
        str+=gradenGraph->getLineName(linesList[i]);
    }
    return str;
}

//将景点的经纬度地理坐标转为视图坐标
QPointF MainWindow::transferCoord(QPointF coord)
{
    QPointF minCoord=gradenGraph->getMinCoord();
    QPointF maxCoord=gradenGraph->getMaxCoord();
    double x = (coord.x()-minCoord.x())/(maxCoord.x()-minCoord.x())*NET_WIDTH+MARGIN;
    double y = (maxCoord.y()-coord.y())/(maxCoord.y()-minCoord.y())*NET_HEIGHT+MARGIN;
    return QPointF(x,y);
}

//绘制网络图的边
void MainWindow::drawEdges(const QList<Edge>& edgesList)
{
    for(int i=0; i<edgesList.size(); ++i)
    {
        int s1=edgesList[i].first;
        int s2=edgesList[i].second;
        QList<int> linesList=gradenGraph->getCommonLines(s1, s2);
        QColor color=getLinesColor(linesList);
        QString tip="途经： "+gradenGraph->getSpotName(s1)+"--"+gradenGraph->getSpotName(s2)+"\n路线：";
        tip+=getLinesName(linesList);
        QPointF s1Pos=transferCoord(gradenGraph->getSpotCoord(s1));
        QPointF s2Pos=transferCoord(gradenGraph->getSpotCoord(s2));
        QGraphicsLineItem* edgeItem=new QGraphicsLineItem;
        edgeItem->setPen(QPen(color, EDGE_PEN_WIDTH));
        edgeItem->setCursor(Qt::PointingHandCursor);
        edgeItem->setToolTip(tip);
        edgeItem->setPos(s1Pos);
        edgeItem->setLine(0, 0, s2Pos.x()-s1Pos.x(), s2Pos.y()-s1Pos.y());
        scene->addItem(edgeItem);
    }
}

//绘制网络图的景点节点
void MainWindow::drawSpots (const QList<int>& spotsList)
{
    for (int i=0; i<spotsList.size(); ++i)
    {
        int s=spotsList[i];
        QString name=gradenGraph->getSpotName(s);
        QList<int> linesList=gradenGraph->getSpotLinesInfo(s);
        QColor color=getLinesColor(linesList);
        QPointF longiLati=gradenGraph->getSpotCoord(s);
        QPointF coord=transferCoord(longiLati);
        QString tip="景点：  "+name+"\n"+
                    "经度：  "+QString::number(longiLati.x(),'f',7)+"\n"+
                    "纬度：  "+QString::number(longiLati.y(),'f',7)+"\n"+
                    "路线："+getLinesName(linesList);
        QGraphicsEllipseItem* spotItem=new QGraphicsEllipseItem;
        spotItem->setRect(-NODE_HALF_WIDTH, -NODE_HALF_WIDTH, NODE_HALF_WIDTH<<1, NODE_HALF_WIDTH<<1);
        spotItem->setPos(coord);
        spotItem->setPen(color);
        spotItem->setCursor(Qt::PointingHandCursor);
        spotItem->setToolTip(tip);

        if(linesList.size()<=1)
        {
            spotItem->setBrush(QColor(QRgb(0xffffff)));
        }

        scene->addItem(spotItem);

        QGraphicsTextItem* textItem=new QGraphicsTextItem;
        textItem->setPlainText(name);
        textItem->setFont(QFont("consolas",30,30));
        textItem->setPos(coord.x(),coord.y()-NODE_HALF_WIDTH*2);
        scene->addItem(textItem);
    }
}


//更新路线选择信息
void MainWindow::updateTranserQueryInfo()
{
    statusLabel3->setText(tr("已更新数据"));
    QComboBox* comboL1=ui->comboBoxStartLine;
    QComboBox* comboL2=ui->comboBoxDstLine;

    comboL1->clear();
    comboL2->clear();
    QList<QString> linesList=gradenGraph->getLinesNameList();
    for(auto &a:linesList)
    {
        comboL1->addItem(a);
        comboL2->addItem(a);
    }
    transferStartLineChanged(comboL1->itemText(0));
    transferDstLineChanged(comboL2->itemText(0));
}

//更新出发路线改变槽函数
void MainWindow::transferStartLineChanged(QString lineName)
{
    QComboBox* comboS1=ui->comboBoxStartSpot;
    comboS1->clear();

    int lineHash=gradenGraph->getLineHash(lineName);
    if(lineHash==-1)
    {
        return ;
    }

    QList<QString> spotsList=gradenGraph->getLineSpotsList(lineHash);
    for(auto &a:spotsList)
    {
        comboS1->addItem(a);
    }
}

//更新目的路线改变槽函数
void MainWindow::transferDstLineChanged(QString lineName)
{
    QComboBox* comboS2=ui->comboBoxDstSpot;
    comboS2->clear();

    int lineHash=gradenGraph->getLineHash(lineName);
    if(lineHash==-1)
    {
        return ;
    }

    QList<QString> spotsList=gradenGraph->getLineSpotsList(lineHash);
    for(auto &a:spotsList)
    {
        comboS2->addItem(a);
    }
}

//动画睡眠函数
void MainWindow::sleep(int msec){    QTime reachTime=QTime::currentTime().addMSecs(msec);    while(QTime::currentTime()<reachTime)        QApplication::processEvents(QEventLoop::AllEvents,100);}

//中转查询槽函数
void MainWindow::transferQuery()
{
    int s1=gradenGraph->getSpotHash(ui->comboBoxStartSpot->currentText());
    int s2=gradenGraph->getSpotHash(ui->comboBoxDstSpot->currentText());
    int way=ui->radioButtonMinTime->isChecked() ? (1) :
            ui->radioButtonUparrawTransfer_1->isChecked() ? (2) :
            ui->radioButtonUparrawTransfer_2->isChecked() ? (3) : (4) ;

    if(s1==-1||s2==-1)
    {
        QMessageBox box;
        box.setWindowTitle(tr("查询"));
        box.setWindowIcon(QIcon(":/icon/icon/query.png"));
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("请选择有景点的路线"));
        box.addButton(tr("确定"),QMessageBox::AcceptRole);
        if(box.exec()==QMessageBox::Accepted)
        {
            box.close();
        }
    }
    else
    {
        QList<int> spotsList;
        QList<Edge> edgesList;
        bool flag=true;
        if(way==(1))
        {
            flag=gradenGraph->queryTransferMinTime(s1, s2, spotsList, edgesList);
        }
        else if(way==(2))
        {
            flag=gradenGraph->queryTransferUparrowTransfer_1(s1, s2, spotsList, edgesList);
        }
        else if(way==(3))
        {
            flag=gradenGraph->queryTransferUparrowTransfer_2(s1, s2, spotsList, edgesList);
        }
        else if(way==(4))
        {
            flag=gradenGraph->queryTransferUparrowTransfer_3(s1, s2, spotsList, edgesList);
        }

        if(flag)
        {
            statusLabel3->setText(tr("查询成功！"));
            on_actionLineMap_triggered();
            for(int i=0; i<edgesList.size(); ++i)
        {
            int s1=edgesList[i].first;
            int s2=edgesList[i].second;
            QList<int> linesList=gradenGraph->getCommonLines(s1, s2);
            QColor color=QColor(148, 0, 211);
            QString tip="途经： "+gradenGraph->getSpotName(s1)+"--"+gradenGraph->getSpotName(s2)+"\n路线：";
            tip+=getLinesName(linesList);
            sleep(1000);
            QPointF s1Pos=transferCoord(gradenGraph->getSpotCoord(s1));
            QPointF s2Pos=transferCoord(gradenGraph->getSpotCoord(s2));
            QGraphicsLineItem* edgeItem=new QGraphicsLineItem;
            edgeItem->setPen(QPen(color, 40));
            edgeItem->setCursor(Qt::PointingHandCursor);
            edgeItem->setToolTip(tip);
            edgeItem->setPos(s1Pos);
            edgeItem->setLine(0, 0, s2Pos.x()-s1Pos.x(), s2Pos.y()-s1Pos.y());
            scene->addItem(edgeItem);
        }
            drawSpots(spotsList);
            QString text ;
            if(way==(1))
            {
                text= ("以下路线时间最短，共经过"+QString::number(spotsList.size()-1)+"个景点\n\n");
            }
            else if(way==(2))
            {
                text= ("以下路线为备选路线一，共经过"+QString::number(spotsList.size()-1)+"个景点\n\n");
            }
            else if(way==(3))
            {
                text= ("以下路线为备选路线二，共经过"+QString::number(spotsList.size()-1)+"个景点\n\n");
            }
            else if(way==(4))
            {
                text= ("以下路线为备选路线三，共经过"+QString::number(spotsList.size()-1)+"个景点\n\n");
            }

            for(int i=0; i<spotsList.size(); ++i)
            {
                if(i)
                {
                    text+="\n  ↓\n";
                }
                text+=gradenGraph->getSpotName(spotsList[i]);
            }
            QTextBrowser* browser=ui->textBrowserRoute;
            browser->clear();
            browser->setText(text);
        }
        else
        {
            QMessageBox box;
            box.setWindowTitle(tr("查询"));
            box.setWindowIcon(QIcon(":/icon/icon/query.png"));
            box.setIcon(QMessageBox::Warning);
            box.setText(tr("您选择的起始和终止景点暂时无法到达！"));
            box.addButton(tr("确定"),QMessageBox::AcceptRole);
            if(box.exec()==QMessageBox::Accepted)
            {
                box.close();
            }
        }
    }
}

//添加列表视图部件变化槽函数
void MainWindow::tabWidgetCurrentChanged(int index)
{
    QWidget* widget=manageLines->ui->tabWidget->currentWidget();

    if(widget==manageLines->tabWigetsVector[1])
    {
        manageLines->linesNameList=gradenGraph->getLinesNameList();
        manageLines->updateLinesListWidget();
    }
    else if(widget==manageLines->tabWigetsVector[2])
    {
        manageLines->linesNameList=gradenGraph->getLinesNameList();
        manageLines->spotsNameList=gradenGraph->getSpotsNameList();
        manageLines->ui->comboBoxConnectSpot1->setMaxCount(manageLines->spotsNameList.size());
        manageLines->ui->comboBoxConnectSpot2->setMaxCount(manageLines->spotsNameList.size());
        manageLines->ui->comboBoxConnectLine->setMaxCount(manageLines->linesNameList.size());
        manageLines->updateComboBox();
    }
    Q_UNUSED(index);
}

//添加路线功能函数
void MainWindow::addLine()
{
    QMessageBox box;
    box.setWindowTitle(tr("添加路线"));
    box.setWindowIcon(QIcon(":/icon/icon/graden.png"));

    if(manageLines->lineName.isEmpty())
    {
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("请输入路线名称！"));
    }
    else if(gradenGraph->getLineHash(manageLines->lineName)==-1)
    {
        box.setIcon(QMessageBox::Information);
        box.setText(tr("路线：")+manageLines->lineName+tr(" 添加成功！"));
        gradenGraph->addLine(manageLines->lineName, manageLines->lineColor);
        updateTranserQueryInfo();
    }
    else
    {
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("添加失败！\n错误原因：路线名已存在！"));
    }

    box.addButton(tr("确定"),QMessageBox::AcceptRole);
    if(box.exec()==QMessageBox::Accepted)
    {
        box.close();
    }
    updateTranserQueryInfo();
}

//添加景点功能函数
void MainWindow::addSpot()
{
    QMessageBox box;
    box.setWindowTitle(tr("添加景点"));
    box.setWindowIcon(QIcon(":/icon/icon/spot.png"));

    if(manageLines->spotName.isEmpty())
    {
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("请输入景点名称！"));
    }
    else if(manageLines->linesSelected.isEmpty())
    {
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("请选择景点所属路线！"));
    }
    else
    {
        if(gradenGraph->getSpotHash(manageLines->spotName)!=-1)
        {
            box.setIcon(QMessageBox::Critical);
            box.setText(tr("添加失败！\n错误原因：景点已存在！"));
        }
        else
        {
            Spot s(manageLines->spotName, manageLines->longitude, manageLines->latitude,
                      gradenGraph->getLinesHash(manageLines->linesSelected));
            gradenGraph->addSpot(s);
            box.setText(tr("景点：")+manageLines->spotName+tr(" 添加成功！"));
            updateTranserQueryInfo();
        }
    }

    box.addButton(tr("确定"),QMessageBox::AcceptRole);
    if(box.exec()==QMessageBox::Accepted)
    {
        box.close();
    }
    updateTranserQueryInfo();
    on_actionLineMap_triggered();
}

//添加连接功能函数
void MainWindow::addConnection()
{
    QString spot1=manageLines->ui->comboBoxConnectSpot1->currentText();
    QString spot2=manageLines->ui->comboBoxConnectSpot2->currentText();
    int s1=gradenGraph->getSpotHash(spot1);
    int s2=gradenGraph->getSpotHash(spot2);
    int l=gradenGraph->getLineHash(manageLines->ui->comboBoxConnectLine->currentText());

    QMessageBox box;
    box.setWindowTitle(tr("添加连接"));
    box.setWindowIcon(QIcon(":/icon/icon/connect.png"));

    if(s1==-1||s2==-1||l==-1)
    {
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("请选择已有的景点和路线！"));
    }
    else if(s1==s2)
    {
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("同一景点不需要连接！"));
    }
    else if(!gradenGraph->getSpotLinesInfo(s1).contains(l))
    {
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("连接失败！\n错误原因：所属路线不包含景点1"));
    }
    else if(!gradenGraph->getSpotLinesInfo(s2).contains(l))
    {
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("连接失败！\n错误原因：所属路线不包含景点2"));
    }
    else
    {
        box.setIcon(QMessageBox::Information);
        box.setText(tr("添加连接成功！"));
        gradenGraph->addConnection(s1,s2,l);
    }
    if(box.exec()==QMessageBox::Accepted)
    {
        box.close();
    }
    updateTranserQueryInfo();
    on_actionLineMap_triggered();
}

//文本方式添加功能函数
void MainWindow::addByText()
{
    QString writeFile="userAdd.txt";
    QFile file(writeFile);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QMessageBox::critical(nullptr, "提示", "无法创建添加文件");
            return ;
    }
    QTextStream out(&file);
    out<<manageLines->ui->textEdit->toPlainText();
    file.close();

    QMessageBox box;
    box.setWindowTitle(tr("文本添加"));
    box.setWindowIcon(QIcon(":/icon/icon/add.png"));

    bool flag=gradenGraph->readFileData(writeFile);
    if(flag)
    {
        box.setIcon(QMessageBox::Information);
        box.setText(tr("添加成功"));
    }
    else
    {
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("添加失败，数据被擦除！"));
    }
    box.addButton(tr("确定"),QMessageBox::AcceptRole);
    if(box.exec()==QMessageBox::Accepted)
    {
        box.close();
    }
    updateTranserQueryInfo();
    on_actionLineMap_triggered();
    return ;
}

//视图放大槽函数
void MainWindow::on_toolEnlarge_triggered()
{
    statusLabel3->setText(tr("已放大"));
    ui->graphicsView->scale(1.5,1.5);
}

//动作视图缩小槽函数
void MainWindow::on_toolShrink_triggered()
{
    statusLabel3->setText(tr("已缩小"));
    ui->graphicsView->scale(2.0/3,2.0/3);
}

//动作添加所有槽函数
void MainWindow::on_actionAddAll_triggered()
{
    statusLabel3->setText(tr("添加路线、景点、连接关系"));
    manageLines->setAllVisible();
    manageLines->show();
}

//动作添加路线槽函数
void MainWindow::on_actionAddLine_triggered()
{
    statusLabel3->setText(tr("添加路线"));
    manageLines->setAddLineVisible();
    manageLines->show();
}

//动作添加景点槽函数
void MainWindow::on_actionAddSpot_triggered()
{
    statusLabel3->setText(tr("添加景点"));
    manageLines->setAddSpotVisible();
    manageLines->show();
}

//动作添加连接槽函数
void MainWindow::on_actionAddConnect_triggered()
{
    statusLabel3->setText(tr("添加连接关系"));
    manageLines->setAddConnectionVisible();
    manageLines->show();
}

//动作文本方式添加槽函数
void MainWindow::on_actionAddByText_triggered()
{
    statusLabel3->setText(tr("文本方式简易添加"));
    manageLines->setAddByTextVisible();
    manageLines->show();
}

//动作查看所有路线图槽函数
void MainWindow::on_actionLineMap_triggered()
{
    statusLabel3->setText(tr("图示：北京世园会网络路线图"));
    scene->clear();
    QList<int> spotsList;
    QList<Edge> edgesList;
    gradenGraph->getGraph(spotsList,edgesList);
    drawEdges(edgesList);
    drawSpots(spotsList);
}

//动作是否显示状态栏槽函数
void MainWindow::on_actionstatusBar_triggered(bool checked)
{
    if(checked)
    {
        ui->statusBar->show();
    }
    else
    {
        ui->statusBar->hide();
    }
}

//动作是否显示工具栏槽函数
void MainWindow::on_actiontoolBar_triggered(bool checked)
{
    if(checked)
    {
        ui->mainToolBar->show();
    }
    else
    {
        ui->mainToolBar->hide();
    }
}

//动作关于Qt槽函数
void MainWindow::on_actionQt_triggered()
{
    QMessageBox::aboutQt(this,tr("关于Qt"));
}

//动作关于作者槽函数
void MainWindow::on_actionAuthor_triggered()
{
    QMessageBox box;
    box.setWindowTitle(tr("关于制作者"));
    box.setIcon(QMessageBox::Information);
    box.setText(tr("Author : 马浩洋\n"
                   "School : BJUT\n"
                   "Student Number : 17071027\n"
                   "Major : 计算机科学与技术（实验班）\n"
                   "Emai : mhy18684300763@163.com \n"));
    box.addButton(tr("确定"),QMessageBox::AcceptRole);
    if(box.exec() == QMessageBox::Accepted)
        box.close();
}

//动作帮助菜单槽函数
void MainWindow::on_actionuseHelp_triggered()
{
    appHelp->show();
}

//动作关闭程序槽函数
void MainWindow::on_actionClose_triggered()
{
    close();
}
