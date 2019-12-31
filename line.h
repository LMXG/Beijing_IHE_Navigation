#ifndef LINE_H
#define LINE_H
#include <QString>
#include <QColor>
#include <QPair>
#include <QSet>
#include <QVector>

//定义边类型
typedef QPair<int,int> Edge;
class GradenGraph;
class QTextStream;
//路线类
class Line
{
protected:
    int id;                     //路线ID
    QString name;               //路线名称
    QColor color;               //路线颜色
    QVector <QString> fromTo;   //路线起始景点
    QSet<int> spotsSet;         //路线景点集合
    QSet<Edge> edges;           //路线景点连接关系
public:
    //构造函数
    Line(){};
    Line(QString lineName, QColor lineColor):name(lineName), color(lineColor)
    {};
    //声明友元
    friend class GradenGraph;
    friend class QTextStream;
};

#endif // LINE_H
