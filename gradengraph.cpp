#include "gradengraph.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStack>
#include <queue>
#include <stack>
#include <vector>

//构造函数
GradenGraph::GradenGraph()
{

}

//从文件读取数据
bool GradenGraph::readFileData(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
        return false;
    QTextStream in(&file);
    while(!in.atEnd())
    {
        Line line;
        QString id, name, colour, fromTo, totalSpots;
        QString color, froms, tos;
        bool ok;
        int total;
        Spot spot;
        int lvIndex, svIndex1, svIndex2;

        in>>id>>line.id;
        in>>name>>line.name;
        in>>colour>>color;
        line.color.setRgba(color.remove(0,1).toUInt(&ok, 16));
        in>>fromTo>>froms>>tos;
        in>>totalSpots>>total;

        line.fromTo.push_back(froms);
        line.fromTo.push_back(tos);
        if (linesHash.count(line.name))
        {
            lvIndex = linesHash[line.name];
            lines[lvIndex].fromTo.push_back(froms);
            lines[lvIndex].fromTo.push_back(tos);
        }
        else
        {
            lvIndex = linesHash[line.name] = lines.size();
            lines.push_back(line);
        }

        QString longlat;
        QStringList strList;
        for (int i=0; !in.atEnd()&&i<total; ++i)
        {
            in>>spot.id>>spot.name>>longlat;
            strList=longlat.split(QChar(','));
            spot.longitude=strList.first().toDouble();
            spot.latitude=strList.last().toDouble();
            if (spotsHash.count(spot.name))
            {
                svIndex2 = spotsHash[spot.name];
            }
            else
            {
                svIndex2 = spotsHash[spot.name] = spots.size();
                spots.push_back(spot);
            }

            spots[svIndex2].linesInfo.insert(lvIndex);
            lines[lvIndex].spotsSet.insert(svIndex2);

            if (i)
            {
                lines[lvIndex].edges.insert(Edge(svIndex1, svIndex2));
                lines[lvIndex].edges.insert(Edge(svIndex2, svIndex1));
                insertEdge(svIndex1, svIndex2);
            }
            svIndex1 = svIndex2;
        }

        bool flag = id=="id:" && name=="name:" && colour=="colour:" && fromTo=="fromTo:"
                && totalSpots=="totalSpots:" && ok && !in.atEnd();
        if(flag==false)
        {
            file.close();
            clearData();
            return false ;
        }
        in.readLine();
    }
    file.close();

    updateMinMaxLongiLati();

    return true;
}

//清空数据
void GradenGraph::clearData()
{
    spots.clear();
    lines.clear();
    spotsHash.clear();
    linesHash.clear();
    edges.clear();
    graph.clear();
}

//插入一条边
bool GradenGraph::insertEdge(int n1, int n2)
{
    if (edges.contains(Edge(n1, n2)) || edges.contains(Edge(n2, n1)))
    {
        return false;
    }
    edges.insert(Edge(n1, n2));
    return true;
}

//生成图结构
void GradenGraph::makeGraph()
{
    graph.clear();
    graph=QVector<QVector<Node>>(spots.size(), QVector<Node>());
    for (auto &a : edges)
    {
        double dist=spots[a.first].distance(spots[a.second]);
        graph[a.first].push_back(Node(a.second, dist));
        graph[a.second].push_back(Node(a.first, dist));
    }
}


//获取路线颜色
QColor GradenGraph::getLineColor(int l)
{
    return lines[l].color;
}

//获取路线名
QString GradenGraph::getLineName(int l)
{
    return lines[l].name;
}

//获取路线hash值
int GradenGraph::getLineHash(QString lineName)
{
    if(linesHash.contains(lineName))
    {
        return linesHash[lineName];
    }
    return -1;
}

//获取路线集合hash值
QList<int> GradenGraph::getLinesHash(QList<QString> linesList)
{
    QList<int> hashList;
    for (auto &a:linesList)
    {
        hashList.push_back(getLineHash(a));
    }
    return hashList;
}

//获取路线名集合
QList<QString> GradenGraph::getLinesNameList()
{
    QList<QString> linesNameList;
    for (auto a:lines)
    {
        linesNameList.push_back(a.name);
    }
    return linesNameList;
}

//获取路线的所有包含景点
QList<QString> GradenGraph::getLineSpotsList(int l)
{
    QList<QString> spotsList;
    for (auto &a:lines[l].spotsSet)
    {
        spotsList.push_back(spots[a].name);
    }
    return spotsList;
}

//更新边界经纬度
void GradenGraph::updateMinMaxLongiLati()
{
    double minLongitude=200, minLatitude=200;
    double maxLongitude=0, maxLatitude=0;
    for (auto &s : spots)
    {
        minLongitude = qMin(minLongitude, s.longitude);
        minLatitude = qMin(minLatitude, s.latitude);
        maxLongitude = qMax(maxLongitude, s.longitude);
        maxLatitude = qMax(maxLatitude, s.latitude);
    }
    Spot::minLongitude = minLongitude;
    Spot::minLatitude = minLatitude;
    Spot::maxLongitude = maxLongitude;
    Spot::maxLatitude = maxLatitude;
}

 //获取景点最小坐标
QPointF GradenGraph::getMinCoord()
{
    return QPointF(Spot::minLongitude, Spot::minLatitude);
}

//获取景点最大坐标
QPointF GradenGraph::getMaxCoord()
{
    return QPointF(Spot::maxLongitude, Spot::maxLatitude);
}

//获取两个景点的公共所属路线
QList<int> GradenGraph::getCommonLines(int s1, int s2)
{
    QList<int> linesList;
    for (auto &s : spots[s1].linesInfo)
    {
        if(spots[s2].linesInfo.contains(s))
            linesList.push_back(s);
    }
    return linesList;
}

//获取景点名
QString GradenGraph::getSpotName(int s)
{
    return spots[s].name;
}

//获取景点地理坐标
QPointF GradenGraph::getSpotCoord(int s)
{
    return QPointF(spots[s].longitude, spots[s].latitude);
}

//获取景点所属路线信息
QList<int> GradenGraph::getSpotLinesInfo(int s)
{
    return spots[s].linesInfo.toList();
}

//获取景点hash值
int GradenGraph::getSpotHash(QString spotName)
{
    if(spotsHash.contains(spotName))
    {
        return spotsHash[spotName];
    }
    return -1;
}

//获取景点集合hash值
QList<QString> GradenGraph::getSpotsNameList()
{
    QList<QString> list;
    for (auto &a: spots)
    {
        list.push_back(a.name);
    }
    return list;
}



//添加新路线
void GradenGraph::addLine(QString lineName, QColor color)
{
    linesHash[lineName]=lines.size();
    lines.push_back(Line(lineName,color));
}

//添加新景点
void GradenGraph::addSpot(Spot s)
{
    int hash=spots.size();
    spotsHash[s.name]=hash;
    spots.push_back(s);
    for (auto &a: s.linesInfo)
    {
        lines[a].spotsSet.insert(hash);
    }
    updateMinMaxLongiLati();
}

//添加景点连接关系
void GradenGraph::addConnection(int s1, int s2, int l)
{
    insertEdge(s1,s2);
    lines[l].edges.insert(Edge(s1,s2));
    lines[l].edges.insert(Edge(s2,s1));
}



//获取网络结构，用于前端显示
void GradenGraph::getGraph(QList<int>&spotsList, QList<Edge>&edgesList)
{
    spotsList.clear();
    for (int i=0; i<spots.size(); ++i)
    {
        spotsList.push_back(i);
    }
    edgesList=edges.toList();
    return ;
}

//获取最少时间的路线
bool GradenGraph::queryTransferMinTime(int s1, int s2, QList<int>&spotsList, QList<Edge>&edgesList)
{
    #define INF 999999999
    spotsList.clear();
    edgesList.clear();

    if(s1==s2)
    {
        spotsList.push_back(s2);
        spotsList.push_back(s1);
        return true;
    }
    makeGraph();
    std::vector<int> path(spots.size(), -1);
    std::vector<double> dist(spots.size(), INF);
    dist[s1]=0;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> priQ;
    priQ.push(Node(s1, 0));
    while(!priQ.empty())
    {
        Node top=priQ.top();
        priQ.pop();
        if(top.spotID==s2)
        {
            break ;
        }

        for (int i=0; i<graph[top.spotID].size(); ++i)
        {
            Node &adjNode=graph[top.spotID][i];
            if(top.distance+adjNode.distance<dist[adjNode.spotID])
            {
                path[adjNode.spotID]=top.spotID;
                dist[adjNode.spotID]=top.distance+adjNode.distance;
                priQ.push(Node(adjNode.spotID, dist[adjNode.spotID]));
            }
        }
        
    }

    if(path[s2]==-1)
    {
        return false;
    }
    int p=s2;
    while(path[p]!=-1)
    {
        spotsList.push_front(p);
        edgesList.push_front(Edge(path[p],p));
        p=path[p];
    }
    spotsList.push_front(s1);
    return true;
}

//获取备选路线一
bool GradenGraph::queryTransferUparrowTransfer_1(int s1, int s2, QList<int>&spotsList, QList<Edge>&edgesList)
{
#define INF 999999999
spotsList.clear();
edgesList.clear();

if(s1==s2)
{
    spotsList.push_back(s2);
    spotsList.push_back(s1);
    return true;
}
makeGraph();
std::vector<int> path(spots.size(), -1);
std::vector<double> dist(spots.size(), INF);
dist[s1]=0;
std::priority_queue<Node, std::vector<Node>, std::greater<Node>> priQ;
priQ.push(Node(s1, 0));
while(!priQ.empty())
{
    Node top=priQ.top();
    priQ.pop();
    if(top.spotID==s2)
    {
        break ;
    }

    for (int i=1; i<graph[top.spotID].size(); ++i)
    {
        Node &adjNode=graph[top.spotID][i];
        if(top.distance+adjNode.distance<=dist[adjNode.spotID])
        {
            graph[top.spotID][i-1].distance= adjNode.distance*10;
        }
    }

    for (int j=1; j<graph[top.spotID].size(); ++j)
    {
        Node &adjNode=graph[top.spotID][j];
        if(top.distance+adjNode.distance<=dist[adjNode.spotID])
        {
            graph[top.spotID][j-1].distance= adjNode.distance*10;
        }
    }

    for (int l=1; l<graph[top.spotID].size(); ++l)
    {
        Node &adjNode=graph[top.spotID][l];
        if(top.distance+adjNode.distance<=dist[adjNode.spotID])
        {
            graph[top.spotID][l-1].distance= adjNode.distance*10;
        }
    }

    for (int z=1; z<graph[top.spotID].size(); ++z)
    {
        Node &adjNode=graph[top.spotID][z];
        if(top.distance+adjNode.distance<=dist[adjNode.spotID])
        {
            path[adjNode.spotID]=top.spotID;
            dist[adjNode.spotID]=top.distance+adjNode.distance;
            priQ.push(Node(adjNode.spotID, dist[adjNode.spotID]));
        }
    }
}

if(path[s2]==-1)
{
    return false;
}
int p=s2;
while(path[p]!=-1)
{
    spotsList.push_front(p);
    edgesList.push_front(Edge(path[p],p));
    p=path[p];
}
spotsList.push_front(s1);
return true;
}

//获取候选路线二
bool GradenGraph::queryTransferUparrowTransfer_2(int s1, int s2, QList<int>&spotsList, QList<Edge>&edgesList)
{
    #define INF 999999999
    spotsList.clear();
    edgesList.clear();

    if(s1==s2)
    {
        spotsList.push_back(s2);
        spotsList.push_back(s1);
        return true;
    }
    makeGraph();
    std::vector<int> path(spots.size(), -1);
    std::vector<double> dist(spots.size(), INF);
    dist[s1]=0;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> priQ;
    priQ.push(Node(s1, 0));
    while(!priQ.empty())
    {
        Node top=priQ.top();
        priQ.pop();
        if(top.spotID==s2)
        {
            break ;
        }

        for (int i=1; i<graph[top.spotID].size(); ++i)
        {
            Node &adjNode=graph[top.spotID][i];
            if(top.distance+adjNode.distance<=dist[adjNode.spotID])
            {
                graph[top.spotID][i-1].distance= adjNode.distance*100;
            }
        }

        for (int j=1; j<graph[top.spotID].size(); ++j)
        {
            Node &adjNode=graph[top.spotID][j];
            if(top.distance+adjNode.distance<=dist[adjNode.spotID])
            {
                path[adjNode.spotID]=top.spotID;
                dist[adjNode.spotID]=top.distance+adjNode.distance;
                priQ.push(Node(adjNode.spotID, dist[adjNode.spotID]));
            }
        }

    }

    if(path[s2]==-1)
    {
        return false;
    }
    int p=s2;
    while(path[p]!=-1)
    {
        spotsList.push_front(p);
        edgesList.push_front(Edge(path[p],p));
        p=path[p];
    }
    spotsList.push_front(s1);
    return true;
}

//获取候选路线三
bool GradenGraph::queryTransferUparrowTransfer_3(int s1, int s2, QList<int>&spotsList, QList<Edge>&edgesList)
{
    #define INF 999999999
    spotsList.clear();
    edgesList.clear();

    if(s1==s2)
    {
        spotsList.push_back(s2);
        spotsList.push_back(s1);
        return true;
    }
    makeGraph();
    std::vector<int> path(spots.size(), -1);
    std::vector<double> dist(spots.size(), INF);
    dist[s1]=0;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> priQ;
    priQ.push(Node(s1, 0));
    while(!priQ.empty())
    {
        Node top=priQ.top();
        priQ.pop();
        if(top.spotID==s2)
        {
            break ;
        }

        for (int i=1; i<graph[top.spotID].size(); ++i)
        {
            Node &adjNode=graph[top.spotID][i];
            if(top.distance+adjNode.distance<=dist[adjNode.spotID])
            {
                graph[top.spotID][i-1].distance= adjNode.distance*100;
            }
        }

        for (int j=1; j<graph[top.spotID].size(); ++j)
        {
            Node &adjNode=graph[top.spotID][j];
            if(top.distance+adjNode.distance<=dist[adjNode.spotID])
            {
                graph[top.spotID][j-1].distance= adjNode.distance*100;
            }
        }

        for (int z=1; z<graph[top.spotID].size(); ++z)
        {
            Node &adjNode=graph[top.spotID][z];
            if(top.distance+adjNode.distance<=dist[adjNode.spotID])
            {
                path[adjNode.spotID]=top.spotID;
                dist[adjNode.spotID]=top.distance+adjNode.distance;
                priQ.push(Node(adjNode.spotID, dist[adjNode.spotID]));
            }
        }
    }

    if(path[s2]==-1)
    {
        return false;
    }
    int p=s2;
    while(path[p]!=-1)
    {
        spotsList.push_front(p);
        edgesList.push_front(Edge(path[p],p));
        p=path[p];
    }
    spotsList.push_front(s1);
    return true;
}
