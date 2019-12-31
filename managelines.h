#ifndef MANAGELINES_H
#define MANAGELINES_H
#include <QDialog>
#include <QVector>
#include <QTabWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QString>

class MainWindow;

namespace Ui {
class ManageLines;
}

class ManageLines : public QDialog
{
    Q_OBJECT

private slots:
    //路线编辑内容改变
    void on_lineEditLineName_textChanged(const QString &arg1);
    //点击选择颜色按钮
    void on_pushButtonChooseColor_clicked();
    //景点编辑内容改变
    void on_lineEditSpotName_textChanged(const QString &arg1);
    //经度编辑内容改变
    void on_doubleSpinBoxLatitude_valueChanged(double arg1);
    //纬度编辑内容改变
    void on_doubleSpinBoxLongitude_valueChanged(double arg1);
    //列表部件选择项改变
    void on_listWidget_itemClicked(QListWidgetItem *item);

public:
    //构造函数
    explicit ManageLines(QWidget *parent = 0);
    //析构函数
    ~ManageLines();
    //设置所有部件可见
    void setAllVisible();
    //设置添加路线部件可见
    void setAddLineVisible();
    //设置添加景点部件可见
    void setAddSpotVisible();
    //设置添加连接部件可见
    void setAddConnectionVisible();
    //设置文本添加部件可见
    void setAddByTextVisible();
    //更新路线列表信息
    void updateLinesListWidget();
    //更新选择部件
    void updateComboBox();

protected:
    Ui::ManageLines *ui;                //UI
    QVector<QWidget*> tabWigetsVector;  //保存tab部件指针
    QVector<QIcon> tabIconVector;       //保存tab部件Icon
    QVector<QString> tabTextVector;     //保存tab部件标题

    QString lineName;                   //保存输入路线名
    QColor lineColor;                   //保存输入路线颜色
    QString spotName;                //保存输入景点名
    double longitude;                   //保存输入景点经度
    double latitude;                    //保存输入景点纬度
    QList<QString> linesNameList;       //保存选择路线名表
    QList<QString> linesSelected;       //保存选择的路线名
    QList<QString> spotsNameList;    //保存选择景点名表

    //声明友元
    friend class MainWindow;
};

#endif // MANAGELINES_H
