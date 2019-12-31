#ifndef GRADENGRAPH_H
#define GRADENGRAPH_H
#include "spot.h"
#include "line.h"
#include <QString>
#include <QPoint>
#include <QVector>
#include <QHash>
#include <iostream>
#include <deque>
using namespace std;

//图的邻接表结构
class Node{
public:
    int spotID;         //邻接表ID
    double distance;    //两点距离
    //构造函数
    Node(){};
    Node(int s, double dist) :spotID(s), distance(dist)
    {};
    //">"运算重载，用于小顶堆
    bool operator > (const Node& n) const
    {
        return this->distance>n.distance;
    }
};

template<class T>
class Greater
{
public:
	bool operator()(const T& x, const T& y)const
	{
		return x > y;
	}
};

template<class T>
class Less
{
public:
	bool operator()(const T& x, const T& y)const
	{
		return x < y;
	}
};
template<class T,class Container = deque<T>,class Compare = Greater<T>>
class Priority_Queue
{
private:
	void AdjustUp(size_t size)
	{
		size_t child = size;
		size_t parent = (child - 1) / 2;
		while (child > 0)
		{
			Compare com;
			if (com(_con[child], _con[parent]))
			{
				swap(_con[child], _con[parent]);
				child = parent;
				parent = (child - 1) / 2;
			}
			else
			{
				break;
			}
		}
	}

	void AdjustDown(size_t size)
	{
		size_t parent = size;
		size_t child = parent * 2 + 1;
		while (child < _con.size())
		{
			Compare com;
			if (child + 1 < _con.size() && com(_con[child + 1], _con[child]))
			{
				child++;
			}
			if (com(_con[child], _con[parent]))
			{
				swap(_con[child], _con[parent]);
				parent = child;
				child = parent * 2 + 1;
			}
			else
			{
				break;
			}
		}
	}

public:

	void Push(const T& x)
	{
		_con.push_back(x);
		AdjustUp(_con.size() - 1);
		_size++;
	}

	void Pop()
	{
		swap(_con[0], _con[_size - 1]);
		_con.pop_back();
		AdjustDown(0);
		_size--;
	}

	size_t  Size()
	{
		return _size;
	}

	bool Empty()
	{
		return _con.empty();
	}

	T& Top()
	{
		return _con[0];
	}

private:
	Container _con;
	size_t _size = 0;
};


//后端管理类
class GradenGraph
{
protected:
    QVector<Spot> spots;                //存储所有景点
    QVector<Line> lines;                //存储所有路线
    QHash<QString, int> spotsHash;      //景点名到存储位置的hash
    QHash<QString, int> linesHash;      //路线名到存储位置的hash
    QSet<Edge> edges;                   //所有边的集合
    QVector<QVector<Node>> graph;       //世园会路线网络图

public:
    //构造函数
    GradenGraph();
    //获取路线名
    QString getLineName(int l);
    //获取路线颜色
    QColor getLineColor(int l);
    //获取路线hash值
    int getLineHash(QString lineName);
    //获取路线集合hash值
    QList<int> getLinesHash(QList<QString> linesList);
    //获取路线名集合
    QList<QString> getLinesNameList();
    //获取路线的所有包含景点
    QList<QString> getLineSpotsList(int l);
    //获取景点名
    QString getSpotName(int s);
    //获取景点地理坐标
    QPointF getSpotCoord(int s);
    //获取景点最小坐标
    QPointF getMinCoord();
    //获取景点最大坐标
    QPointF getMaxCoord();
    //获取景点所属路线信息
    QList<int> getSpotLinesInfo(int s);
    //获取两个景点的公共所属路线
    QList<int> getCommonLines(int s1, int s2);
    //获取景点hash值
    int getSpotHash(QString spotName);
    //获取景点集合hash值
    QList<QString> getSpotsNameList();
    //添加新路线
    void addLine(QString lineName, QColor color);
    //添加新景点
    void addSpot(Spot s);
    //添加景点连接关系
    void addConnection(int s1, int s2, int l);
    //获取网络结构，用于前端显示
    void getGraph(QList<int>&spotsList, QList<Edge>&edgesList);
    //获取最少时间的路线
    bool queryTransferMinTime(int s1, int s2, QList<int>&spotsList, QList<Edge>&edgesList);
    //获取候选路线一
    bool queryTransferUparrowTransfer_1(int s1, int s2, QList<int>&spotsList, QList<Edge>&edgesList);
    //获取候选路线二
    bool queryTransferUparrowTransfer_2(int s1, int s2, QList<int>&spotsList, QList<Edge>&edgesList);
    //获取候选路线三
    bool queryTransferUparrowTransfer_3(int s1, int s2, QList<int>&spotsList, QList<Edge>&edgesList);
    //从文件读取数据
    bool readFileData(QString fileName);

private:
    //清空数据
    void clearData();
    //插入一条边
    bool insertEdge(int s1, int s2);
    //更新边界经纬度
    void updateMinMaxLongiLati();
    //生成图结构
    void makeGraph();
};

#endif // GRADENGRAPH_H
