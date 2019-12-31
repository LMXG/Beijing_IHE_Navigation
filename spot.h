#ifndef SPOT_H
#define SPOT_H
#include <QString>
#include <QPointF>
#include <QSet>

class GradenGraph;
class QTextStream;

//世园会景点类定义
class Spot
{
protected:
    int id;                     //景点ID
    QString name;               //景点名称
    double longitude, latitude; //景点经纬度
    QSet<int> linesInfo;        //景点所属路线

    //所有景点的边界位置
    static double minLongitude, minLatitude, maxLongitude, maxLatitude;

public:
    //构造函数
    Spot();
    Spot(QString nameStr, double longi, double lati, QList<int> linesList);

protected:
    //求取景点间实地直线距离
    int distance(Spot other);

    //声明友元
    friend class GradenGraph;
    friend class QTextStream;
};

#endif // SPOT_H
