#ifndef BOOLEANPLUGIN_HH_INCLUDED
#define BOOLEANPLUGIN_HH_INCLUDED
#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/ToolboxInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>
#include <OpenFlipper/BasePlugin/LoadSaveInterface.hh>
#include <OpenFlipper/common/Types.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/PolyLine/PolyLine.hh>
#include <ObjectTypes/PolyLineCollection/PolyLineCollection.hh>

#include <memory>
#include <tuple>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QSpinBox>

const double eps = 1e-10;

static int dcmp(double x)
{
	if (fabs(x) < eps) return 0;
	else return x < 0 ? -1 : 1;
}

class Point
{
public:
	double x, y;

	Point(double x = 0, double y = 0) : x(x), y(y)
	{
	}

	Point& operator=(const Point& b)
	{
		x = b.x;
		y = b.y;
		return *this;
	}

	bool operator<(const Point& b)
	{
		if (x < b.x)
		{
			return true;
		}
		else if (x == b.x)
		{
			return y < b.y;
		}
		return false;
	}

	bool operator!=(const Point b)
	{
		return !(dcmp(x - b.x) == 0 && dcmp(y - b.y) == 0);
	}

	bool operator==(const Point b)
	{
		return dcmp(x - b.x) == 0 && dcmp(y - b.y) == 0;
	}
};

const Point NOPOINT(1e10, 1e10);

static Point operator-(const Point a, const Point b)
{
	return Point(a.x - b.x, a.y - b.y);
}

class Edge
{
public:
	Point LPoint, RPoint;
	int odd; //Ææ±ß £º 1 Å¼±ß £º 0
	int inout; //ÄÚ±ß £º 1 Íâ±ß£º 0 ÖØ±ß £º2
	void init()
	{
		if (RPoint < LPoint)
		{
			std::swap(LPoint, RPoint);
		}
	}

	Edge()
	{
	}

	Edge(Point l, Point r) : LPoint(l), RPoint(r)
	{
	}

	bool operator==(const Edge B)
	{
		return LPoint == B.LPoint && RPoint == B.RPoint;
	}

	bool operator!=(const Edge B)
	{
		return !(LPoint == B.LPoint && RPoint == B.RPoint);
	}
};

class TEdge : public Edge
{
public:
	int pgn; /* ±ê¼Ç±ßÊÇÊôÓÚ¶à±ßÐÎA¡¢»¹ÊÇ¶à±ßÐÎB */
	bool visited; /* ±ê¼Ç±ßÊÇ·ñÒÑ·ÃÎÊ¹ý, ³õÊ¼ÖµÎªfalse */
	int repet;
	int lpAdjEg, rpAdjEg; /* Ö¸ÏòÁ¬½Ó×ó¡¢ÓÒ¶ËµãµÄ±¾ÊäÈë¶à±ßÐÎµÄÁÚ±ßµÄÖ¸Õë*/
	int lpotherPgnAdjEg[2], rpotherPgnAdjEg[2]; /* Ö¸ÏòÁ¬½Ó×ó¡¢ÓÒ¶ËµãµÄÁíÒ»ÊäÈë¶à±ßÐÎµÄÁ½ÁÚ±ßµÄÖ¸Õë£¬ÖÁ¶à2Ìõ*/

	TEdge()
	{
		memset(lpotherPgnAdjEg, -1, sizeof(lpotherPgnAdjEg));
		memset(rpotherPgnAdjEg, -1, sizeof(rpotherPgnAdjEg));
		visited = false;
	}

	TEdge& operator=(const Edge& e)
	{
		//TEdge ans;
		LPoint = e.LPoint;
		RPoint = e.RPoint;
		odd = e.odd;
		inout = e.inout;
		return *this;
	}

	bool operator==(const TEdge B)
	{
		return LPoint == B.LPoint && RPoint == B.RPoint;
	}

	bool operator!=(const TEdge B)
	{
		return !(LPoint == B.LPoint && RPoint == B.RPoint);
	}

	bool operator==(const Edge B)
	{
		return LPoint == B.LPoint && RPoint == B.RPoint;
	}

	bool operator!=(const Edge B)
	{
		return !(LPoint == B.LPoint && RPoint == B.RPoint);
	}
};

//*************************************************Á½Ïß¶Î½»µã
static double tCross(const Point& p1, const Point& p2)
{
	return p1.x * p2.y - p1.y * p2.x;
}

static double Cross(const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	return (p2.x - p1.x) * (p4.y - p3.y) - (p2.y - p1.y) * (p4.x - p3.x);
}

static double Area(const Point& p1, const Point& p2, const Point& p3)
{
	return Cross(p1, p2, p1, p3);
}

static double fArea(const Point& p1, const Point& p2, const Point& p3)
{
	return fabs(Area(p1, p2, p3));
}

// bool Meet(const Point& p1, const Point& p2, const Point& p3, const Point& p4) //Ã»ÓÐÓÃµ½
// {
// 	return max(min(p1.x, p2.x), min(p3.x, p4.x)) <= min(max(p1.x, p2.x), max(p3.x, p4.x)) //overlap_x
// 		&& max(min(p1.y, p2.y), min(p3.y, p4.y)) <= min(max(p1.y, p2.y), max(p3.y, p4.y)) //overlap_y
// 		&& dcmp(Cross(p3, p2, p3, p4) * Cross(p3, p4, p3, p1)) >= 0
// 		&& dcmp(Cross(p1, p4, p1, p2) * Cross(p1, p2, p1, p3)) >= 0;
// }

static Point Inter(const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	double s1 = fArea(p1, p2, p3), s2 = fArea(p1, p2, p4);
	return Point((p4.x * s1 + p3.x * s2) / (s1 + s2), (p4.y * s1 + p3.y * s2) / (s1 + s2));
}

static double Dot(Point A, Point B)
{
	return A.x * B.x + A.y * B.y;
}

static bool OnSegment(Point p, Point a1, Point a2)
{
	if (p == a1 || p == a2)
	{
		return true;
	}
	double x1 = std::min(a1.x, a2.x);
	double x2 = std::max(a1.x, a2.x);
	double y1 = std::min(a1.y, a2.y);
	double y2 = std::max(a1.y, a2.y);
	return dcmp(tCross(a1 - p, a2 - p)) == 0 && p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2;
}

static Point GetCrossPoint(Edge a, Edge b)
{
	Point a1 = a.LPoint;
	Point a2 = a.RPoint;
	Point b1 = b.LPoint;
	Point b2 = b.RPoint;
	double c1 = tCross(a2 - a1, b1 - a1);
	double c2 = tCross(a2 - a1, b2 - a1);
	double c3 = tCross(b2 - b1, a1 - b1);
	double c4 = tCross(b2 - b1, a2 - b1);
	if (OnSegment(b.LPoint, a.LPoint, a.RPoint))
	{
		return b.LPoint;
	}
	if (OnSegment(b.RPoint, a.LPoint, a.RPoint))
	{
		return b.RPoint;
	}
	//	if (dcmp(Cross(a.LPoint, a.RPoint, b.LPoint, b.RPoint)) == 0){
	//		return NOPOINT;
	//	}
	if (dcmp(c1) * dcmp(c2) < 0 && dcmp(c3) * dcmp(c4) < 0)
	{
		return Inter(a.LPoint, a.RPoint, b.LPoint, b.RPoint);
	}
	else
	{
		return NOPOINT;
	}
}

//************************************************************

class Circle
{
public:
	std::vector<Edge> circle_edge;

	Circle()
	{
		circle_edge.clear();
	}

	void init()
	{
		for (int i = 0; i < (int)circle_edge.size(); i++)
		{
			circle_edge[i].init();
		}
	}

	void reset()
	{
		circle_edge.clear();
	}
};

static bool PointAboveSegment(Point p, Edge e)
{
	if (dcmp(e.LPoint.x - p.x) == 0 && dcmp(e.LPoint.x - e.RPoint.x) == 0 && dcmp(p.y - e.LPoint.y) >= 0 &&
		dcmp(p.y - e.RPoint.y) < 0)
	{
		return true;
	}
	if (dcmp(p.x - e.RPoint.x) >= 0 || dcmp(p.x - e.LPoint.x) < 0)
	{
		return false;
	}
	Point c = p;
	Point a = e.LPoint;
	Point b = e.RPoint;
	return dcmp(c.y - a.y - (c.x - a.x) / (b.x - a.x) * (b.y - a.y)) >= 0;
}

class MPolygon
{
public:
	Circle external_circle; //Íâ»·
	std::vector<Circle> internal_circle; //ÄÚ»·

	MPolygon()
	{
		internal_circle.clear();
	}

	void reset()
	{
		external_circle.reset();
		internal_circle.clear();
	}

	void init()
	{
		external_circle.init();
		for (int i = 0; i < (int)internal_circle.size(); i++)
		{
			internal_circle[i].init();
		}
	}

	int calc(Point p, const MPolygon B)
	{
		int cnt = 0;
		for (int i = 0; i < (int)B.external_circle.circle_edge.size(); i++)
		{
			if (PointAboveSegment(p, B.external_circle.circle_edge[i]))
			{
				cnt++;
			}
		}
		for (int j = 0; j < (int)B.internal_circle.size(); j++)
		{
			for (int i = 0; i < (int)B.internal_circle[j].circle_edge.size(); i++)
			{
				if (PointAboveSegment(p, B.internal_circle[j].circle_edge[i]))
				{
					cnt++;
				}
			}
		}
		return cnt;
	}

	bool finde(Edge e, const MPolygon B)
	{
		for (int i = 0; i < (int)B.external_circle.circle_edge.size(); i++)
		{
			if (e == B.external_circle.circle_edge[i])
			{
				return true;
			}
		}
		for (int j = 0; j < (int)B.internal_circle.size(); j++)
		{
			for (int i = 0; i < (int)B.internal_circle[j].circle_edge.size(); i++)
			{
				if (e == B.internal_circle[j].circle_edge[i])
				{
					return true;
				}
			}
		}
		return false;
	}

	void getinout(const MPolygon B)
	{
		//ÅÐ¶ÏÄÚ±ß»¹ÊÇÍâ±ß
		for (int i = 0; i < (int)external_circle.circle_edge.size(); i++)
		{
			Point p;
			p.x = (external_circle.circle_edge[i].LPoint.x + external_circle.circle_edge[i].RPoint.x) / 2.0;
			p.y = (external_circle.circle_edge[i].LPoint.y + external_circle.circle_edge[i].RPoint.y) / 2.0;
			if (finde(external_circle.circle_edge[i], B))
			{
				external_circle.circle_edge[i].inout = 2;
			}
			else if (calc(p, B) & 1)
			{
				external_circle.circle_edge[i].inout = 1;
			}
			else
			{
				external_circle.circle_edge[i].inout = 0;
			}
		}
		for (int j = 0; j < (int)internal_circle.size(); j++)
		{
			for (int i = 0; i < (int)internal_circle[j].circle_edge.size(); i++)
			{
				Point p;
				p.x = (internal_circle[j].circle_edge[i].LPoint.x + internal_circle[j].circle_edge[i].RPoint.x) / 2.0;
				p.y = (internal_circle[j].circle_edge[i].LPoint.y + internal_circle[j].circle_edge[i].RPoint.y) / 2.0;
				if (finde(internal_circle[j].circle_edge[i], B))
				{
					internal_circle[j].circle_edge[i].inout = 2;
				}
				else if (calc(p, B) & 1)
				{
					internal_circle[j].circle_edge[i].inout = 1;
				}
				else
				{
					internal_circle[j].circle_edge[i].inout = 0;
				}
			}
		}
	}

	void getodd()
	{
		//ÅÐ¶ÏÆæ±ß»¹ÊÇÅ¼±ß
		for (int i = 0; i < (int)external_circle.circle_edge.size(); i++)
		{
			Point p;
			p.x = (external_circle.circle_edge[i].LPoint.x + external_circle.circle_edge[i].RPoint.x) / 2.0;
			p.y = (external_circle.circle_edge[i].LPoint.y + external_circle.circle_edge[i].RPoint.y) / 2.0;
			if (calc(p, *this) & 1)
			{
				external_circle.circle_edge[i].odd = 1;
			}
			else
			{
				external_circle.circle_edge[i].odd = 0;
			}
		}
		for (int j = 0; j < (int)internal_circle.size(); j++)
		{
			for (int i = 0; i < (int)internal_circle[j].circle_edge.size(); i++)
			{
				Point p;
				p.x = (internal_circle[j].circle_edge[i].LPoint.x + internal_circle[j].circle_edge[i].RPoint.x) / 2.0;
				p.y = (internal_circle[j].circle_edge[i].LPoint.y + internal_circle[j].circle_edge[i].RPoint.y) / 2.0;
				if (calc(p, *this) & 1)
				{
					internal_circle[j].circle_edge[i].odd = 1;
				}
				else
				{
					internal_circle[j].circle_edge[i].odd = 0;
				}
			}
		}
	}

	void getSimpleEdge(const MPolygon B)
	{
		//µÃµ½¼òµ¥±ß
		std::vector<Edge> temp1;
		temp1.clear();

		//´¦ÀíÍâ»·
		for (int i = 0; i < (int)external_circle.circle_edge.size(); i++)
		{
			std::vector<Point> temp2;
			temp2.clear();
			temp2.push_back(external_circle.circle_edge[i].LPoint);
			temp2.push_back(external_circle.circle_edge[i].RPoint);
			for (int j = 0; j < (int)B.external_circle.circle_edge.size(); j++)
			{
				Edge a = external_circle.circle_edge[i];
				Edge b = B.external_circle.circle_edge[j];
				Point tempp = GetCrossPoint(external_circle.circle_edge[i], B.external_circle.circle_edge[j]);
				if (tempp != NOPOINT)
				{
					temp2.push_back(tempp);
				}
			}
			for (int j = 0; j < (int)B.internal_circle.size(); j++)
			{
				for (int k = 0; k < (int)B.internal_circle[j].circle_edge.size(); k++)
				{
					Point tempp = GetCrossPoint(external_circle.circle_edge[i], B.internal_circle[j].circle_edge[k]);
					if (tempp != NOPOINT)
					{
						temp2.push_back(tempp);
					}
				}
			}
			sort(temp2.begin(), temp2.end());
			for (int j = 0; j < (int)temp2.size() - 1; j++)
			{
				for (int k = j + 1; k < (int)temp2.size(); k++)
				{
					if (temp2[k] != temp2[j])
					{
						temp1.push_back(Edge(temp2[j], temp2[k]));
						j = k - 1;
						break;
					}
				}
			}
		}
		external_circle.circle_edge = temp1;


		//´¦ÀíÄÚ»·
		for (int l = 0; l < (int)internal_circle.size(); l++)
		{
			std::vector<Edge> temp1;
			temp1.clear();
			for (int i = 0; i < (int)internal_circle[l].circle_edge.size(); i++)
			{
				std::vector<Point> temp2;
				temp2.clear();
				temp2.push_back(internal_circle[l].circle_edge[i].LPoint);
				temp2.push_back(internal_circle[l].circle_edge[i].RPoint);
				for (int j = 0; j < (int)B.external_circle.circle_edge.size(); j++)
				{
					Point tempp = GetCrossPoint(internal_circle[l].circle_edge[i], B.external_circle.circle_edge[j]);
					if (tempp != NOPOINT)
					{
						temp2.push_back(tempp);
					}
				}
				for (int j = 0; j < (int)B.internal_circle.size(); j++)
				{
					for (int k = 0; k < (int)B.internal_circle[j].circle_edge.size(); k++)
					{
						Point tempp = GetCrossPoint(internal_circle[l].circle_edge[i],
						                            B.internal_circle[j].circle_edge[k]);
						if (tempp != NOPOINT)
						{
							temp2.push_back(tempp);
						}
					}
				}
				sort(temp2.begin(), temp2.end());
				for (int j = 0; j < (int)temp2.size() - 1; j++)
				{
					for (int k = j + 1; k < (int)temp2.size(); k++)
					{
						if (temp2[k] != temp2[j])
						{
							temp1.push_back(Edge(temp2[j], temp2[k]));
							j = k - 1;
							break;
						}
					}
				}
			}
			internal_circle[l].circle_edge = temp1;
		}
	}
};

static void solve_or(MPolygon p1, MPolygon p2, std::vector<std::vector<Point>>& ans)
{
	p1.init();
	p2.init();
	p1.getSimpleEdge(p2);
	p2.getSimpleEdge(p1);
	p1.getinout(p2);
	p2.getinout(p1);
	p1.getodd();
	p2.getodd();

	std::vector<TEdge> edge;
	edge.clear();

	for (int i = 0; i < p1.external_circle.circle_edge.size(); i++)
	{
		TEdge temp;
		temp = p1.external_circle.circle_edge[i];
		temp.pgn = 1;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p1.internal_circle.size(); j++)
	{
		for (int i = 0; i < p1.internal_circle[j].circle_edge.size(); i++)
		{
			TEdge temp;
			temp = p1.internal_circle[j].circle_edge[i];
			temp.pgn = 1;
			temp.visited = false;
			edge.push_back(temp);
		}
	}
	for (int i = 0; i < p2.external_circle.circle_edge.size(); i++)
	{
		TEdge temp;
		temp = p2.external_circle.circle_edge[i];
		temp.pgn = 2;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p2.internal_circle.size(); j++)
	{
		for (int i = 0; i < p2.internal_circle[j].circle_edge.size(); i++)
		{
			TEdge temp;
			temp = p2.internal_circle[j].circle_edge[i];
			temp.pgn = 2;
			temp.visited = false;
			edge.push_back(temp);
		}
	}

	for (int i = 0; i < edge.size(); i++)
	{
		for (int j = 0; j < edge.size(); j++)
		{
			if (i == j) continue;
			if (edge[j].pgn == edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint))
			{
				edge[i].lpAdjEg = j;
			}
			if (edge[j].pgn == edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint))
			{
				edge[i].rpAdjEg = j;
			}
		}
		int t1 = 0, t2 = 0;
		for (int j = 0; j < edge.size(); j++)
		{
			if (i == j) continue;
			if (edge[j].pgn != edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint))
			{
				edge[i].lpotherPgnAdjEg[t1++] = j;
			}
			if (edge[j].pgn != edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint))
			{
				edge[i].rpotherPgnAdjEg[t2++] = j;
			}
		}
		if (edge[i].inout == 2)
		{
			for (int j = 0; j < edge.size(); j++)
			{
				if (i == j) continue;
				if (edge[j].pgn != edge[i].pgn && edge[j] == edge[i])
				{
					edge[i].repet = j;
				}
			}
		}
	}

	for (int i = 0; i < edge.size(); i++)
	{
		std::vector<Point> ansv;
		ansv.clear();
		if (!edge[i].visited && edge[i].inout == 0)
		{
			Point P;
			int e = i;
			do
			{
				edge[e].visited = true;
				if (edge[e].odd == 1)
				{
					ansv.push_back(edge[e].LPoint);
					P = edge[e].RPoint;
					if (edge[edge[e].rpAdjEg].inout == 0)
					{
						e = edge[e].rpAdjEg;
					}
					else if (edge[e].rpotherPgnAdjEg[0] != -1
						&& edge[edge[e].rpotherPgnAdjEg[0]].inout == 0)
					{
						e = edge[e].rpotherPgnAdjEg[0];
					}
					else if (edge[e].rpotherPgnAdjEg[1] != -1
						&& edge[edge[e].rpotherPgnAdjEg[1]].inout == 0)
					{
						e = edge[e].rpotherPgnAdjEg[1];
					}
					else
					{
						e = edge[e].rpAdjEg;
					}
				}
				else
				{
					ansv.push_back(edge[e].RPoint);
					P = edge[e].LPoint;
					if (edge[edge[e].lpAdjEg].inout == 0)
					{
						e = edge[e].lpAdjEg;
					}
					else if (edge[e].lpotherPgnAdjEg[0] != -1
						&& edge[edge[e].lpotherPgnAdjEg[0]].inout == 0)
					{
						e = edge[e].lpotherPgnAdjEg[0];
					}
					else if (edge[e].lpotherPgnAdjEg[1] != -1
						&& edge[edge[e].lpotherPgnAdjEg[1]].inout == 0)
					{
						e = edge[e].lpotherPgnAdjEg[1];
					}
					else
					{
						e = edge[e].lpAdjEg;
					}
				}
			}
			while (ansv[0] != P);
			std::vector<Point>tmp;
            tmp.reserve(ansv.size());
            for (auto p: ansv) tmp.emplace_back(p);
            tmp.push_back(ansv.front());
            ans.emplace_back(tmp);
		}
	}
	printf("Finish!\n");
}

static void solve_and(MPolygon p1, MPolygon p2, std::vector<std::vector<Point>>& ans)
{
	p1.init();
	p2.init();
	p1.getSimpleEdge(p2);
	p2.getSimpleEdge(p1);
	p1.getinout(p2);
	p2.getinout(p1);
	p1.getodd();
	p2.getodd();

	std::vector<TEdge> edge;
	edge.clear();

	for (int i = 0; i < p1.external_circle.circle_edge.size(); i++)
	{
		TEdge temp;
		temp = p1.external_circle.circle_edge[i];
		temp.pgn = 1;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p1.internal_circle.size(); j++)
	{
		for (int i = 0; i < p1.internal_circle[j].circle_edge.size(); i++)
		{
			TEdge temp;
			temp = p1.internal_circle[j].circle_edge[i];
			temp.pgn = 1;
			temp.visited = false;
			edge.push_back(temp);
		}
	}
	for (int i = 0; i < p2.external_circle.circle_edge.size(); i++)
	{
		TEdge temp;
		temp = p2.external_circle.circle_edge[i];
		temp.pgn = 2;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p2.internal_circle.size(); j++)
	{
		for (int i = 0; i < p2.internal_circle[j].circle_edge.size(); i++)
		{
			TEdge temp;
			temp = p2.internal_circle[j].circle_edge[i];
			temp.pgn = 2;
			temp.visited = false;
			edge.push_back(temp);
		}
	}

	for (int i = 0; i < edge.size(); i++)
	{
		for (int j = 0; j < edge.size(); j++)
		{
			if (i == j) continue;
			if (edge[j].pgn == edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint))
			{
				edge[i].lpAdjEg = j;
			}
			if (edge[j].pgn == edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint))
			{
				edge[i].rpAdjEg = j;
			}
		}
		int t1 = 0, t2 = 0;
		for (int j = 0; j < edge.size(); j++)
		{
			if (i == j) continue;
			if (edge[j].pgn != edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint))
			{
				edge[i].lpotherPgnAdjEg[t1++] = j;
			}
			if (edge[j].pgn != edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint))
			{
				edge[i].rpotherPgnAdjEg[t2++] = j;
			}
		}
		if (edge[i].inout == 2)
		{
			for (int j = 0; j < edge.size(); j++)
			{
				if (i == j) continue;
				if (edge[j].pgn != edge[i].pgn && edge[j] == edge[i])
				{
					edge[i].repet = j;
				}
			}
		}
	}

	for (int i = 0; i < edge.size(); i++)
	{
		std::vector<Point> ansv;
		ansv.clear();
		if (!edge[i].visited && edge[i].inout == 1)
		{
			Point P;
			int e = i;
			do
			{
				edge[e].visited = true;
				if (edge[e].odd == 1)
				{
					ansv.push_back(edge[e].LPoint);
					P = edge[e].RPoint;
					if (edge[e].rpotherPgnAdjEg[0] != -1
						&& edge[edge[e].rpotherPgnAdjEg[0]].inout == 1
						&& ((edge[edge[e].rpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].rpotherPgnAdjEg[0]].LPoint == P)
							|| (edge[edge[e].rpotherPgnAdjEg[0]].odd == 0 &&
								edge[edge[e].rpotherPgnAdjEg[0]].RPoint == P)))
					{
						e = edge[e].rpotherPgnAdjEg[0];
					}
					else if (edge[e].rpotherPgnAdjEg[1] != -1
						&& edge[edge[e].rpotherPgnAdjEg[1]].inout == 1
						&& ((edge[edge[e].rpotherPgnAdjEg[1]].odd == 1 &&
								edge[edge[e].rpotherPgnAdjEg[1]].LPoint == P)
							|| (edge[edge[e].rpotherPgnAdjEg[1]].odd == 0 &&
								edge[edge[e].rpotherPgnAdjEg[1]].RPoint == P)))
					{
						e = edge[e].rpotherPgnAdjEg[1];
					}
					else
					{
						e = edge[e].rpAdjEg;
					}
				}
				else
				{
					ansv.push_back(edge[e].RPoint);
					P = edge[e].LPoint;
					if (edge[e].lpotherPgnAdjEg[0] != -1
						&& edge[edge[e].lpotherPgnAdjEg[0]].inout == 1
						&& ((edge[edge[e].lpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].lpotherPgnAdjEg[0]].LPoint == P)
							|| (edge[edge[e].lpotherPgnAdjEg[0]].odd == 0 &&
								edge[edge[e].lpotherPgnAdjEg[0]].RPoint == P)))
					{
						e = edge[e].lpotherPgnAdjEg[0];
					}
					else if (edge[e].lpotherPgnAdjEg[1] != -1
						&& edge[edge[e].lpotherPgnAdjEg[1]].inout == 1
						&& ((edge[edge[e].lpotherPgnAdjEg[1]].odd == 1 &&
								edge[edge[e].lpotherPgnAdjEg[1]].LPoint == P)
							|| (edge[edge[e].lpotherPgnAdjEg[1]].odd == 0 &&
								edge[edge[e].lpotherPgnAdjEg[1]].RPoint == P)))
					{
						e = edge[e].lpotherPgnAdjEg[1];
					}
					else
					{
						e = edge[e].lpAdjEg;
					}
				}
			}
			while (ansv[0] != P);
			std::vector<Point>tmp;
            tmp.reserve(ansv.size());
            for (auto p: ansv) tmp.emplace_back(p);
            tmp.push_back(ansv.front());
            ans.emplace_back(tmp);
		}
	}
}

static void solve_sub(MPolygon p1, MPolygon p2, std::vector<std::vector<Point>>& ans)
{
	p1.init();
	p2.init();
	p1.getSimpleEdge(p2);
	p2.getSimpleEdge(p1);
	p1.getinout(p2);
	p2.getinout(p1);
	p1.getodd();
	p2.getodd();

	std::vector<TEdge> edge;
	edge.clear();

	for (int i = 0; i < p1.external_circle.circle_edge.size(); i++)
	{
		TEdge temp;
		temp = p1.external_circle.circle_edge[i];
		temp.pgn = 1;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p1.internal_circle.size(); j++)
	{
		for (int i = 0; i < p1.internal_circle[j].circle_edge.size(); i++)
		{
			TEdge temp;
			temp = p1.internal_circle[j].circle_edge[i];
			temp.pgn = 1;
			temp.visited = false;
			edge.push_back(temp);
		}
	}
	for (int i = 0; i < p2.external_circle.circle_edge.size(); i++)
	{
		TEdge temp;
		temp = p2.external_circle.circle_edge[i];
		temp.pgn = 2;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p2.internal_circle.size(); j++)
	{
		for (int i = 0; i < p2.internal_circle[j].circle_edge.size(); i++)
		{
			TEdge temp;
			temp = p2.internal_circle[j].circle_edge[i];
			temp.pgn = 2;
			temp.visited = false;
			edge.push_back(temp);
		}
	}

	for (int i = 0; i < edge.size(); i++)
	{
		for (int j = 0; j < edge.size(); j++)
		{
			if (i == j) continue;
			if (edge[j].pgn == edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint))
			{
				edge[i].lpAdjEg = j;
			}
			if (edge[j].pgn == edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint))
			{
				edge[i].rpAdjEg = j;
			}
		}
		int t1 = 0, t2 = 0;
		for (int j = 0; j < edge.size(); j++)
		{
			if (i == j) continue;
			if (edge[j].pgn != edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint))
			{
				edge[i].lpotherPgnAdjEg[t1++] = j;
			}
			if (edge[j].pgn != edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint))
			{
				edge[i].rpotherPgnAdjEg[t2++] = j;
			}
		}
		if (edge[i].inout == 2)
		{
			for (int j = 0; j < edge.size(); j++)
			{
				if (i == j) continue;
				if (edge[j].pgn != edge[i].pgn && edge[j] == edge[i])
				{
					edge[i].repet = j;
				}
			}
		}
	}

	for (int i = 0; i < edge.size(); i++)
	{
		std::vector<Point> ansv;
		ansv.clear();
		if (!edge[i].visited &&
			((edge[i].inout == 0 && edge[i].pgn == 1) || (edge[i].inout == 1 && edge[i].pgn == 2)))
		{
			Point P;
			int e = i;
			do
			{
				edge[e].visited = true;
				if ((edge[e].odd == 1 && edge[e].pgn == 1) || (edge[e].odd == 0 && edge[e].pgn == 2))
				{
					ansv.push_back(edge[e].LPoint);
					P = edge[e].RPoint;
					if (edge[e].pgn == 1
						&& edge[e].rpotherPgnAdjEg[0] != -1
						&& edge[edge[e].rpotherPgnAdjEg[0]].inout == 1
						&& ((edge[edge[e].rpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].rpotherPgnAdjEg[0]].RPoint == P)
							|| (edge[edge[e].rpotherPgnAdjEg[0]].odd == 0 &&
								edge[edge[e].rpotherPgnAdjEg[0]].LPoint == P)))
					{
						e = edge[e].rpotherPgnAdjEg[0];
					}
					else if (edge[e].pgn == 1
						&& edge[e].rpotherPgnAdjEg[1] != -1
						&& edge[edge[e].rpotherPgnAdjEg[1]].inout == 1
						&& ((edge[edge[e].rpotherPgnAdjEg[1]].odd == 1 &&
								edge[edge[e].rpotherPgnAdjEg[1]].RPoint == P)
							|| (edge[edge[e].rpotherPgnAdjEg[1]].odd == 0 &&
								edge[edge[e].rpotherPgnAdjEg[1]].LPoint == P)))
					{
						e = edge[e].rpotherPgnAdjEg[1];
					}
					else if (edge[e].pgn == 2
						&& edge[e].rpotherPgnAdjEg[0] != -1
						&& edge[edge[e].rpotherPgnAdjEg[0]].inout == 0
						&& ((edge[edge[e].rpotherPgnAdjEg[0]].odd == 1 &&
								edge[edge[e].rpotherPgnAdjEg[0]].LPoint == P)
							|| (edge[edge[e].rpotherPgnAdjEg[0]].odd == 0 &&
								edge[edge[e].rpotherPgnAdjEg[0]].RPoint == P)))
					{
						e = edge[e].rpotherPgnAdjEg[0];
					}
					else if (edge[e].pgn == 2
						&& edge[e].rpotherPgnAdjEg[1] != -1
						&& edge[edge[e].rpotherPgnAdjEg[1]].inout == 0
						&& ((edge[edge[e].rpotherPgnAdjEg[1]].odd == 1 &&
								edge[edge[e].rpotherPgnAdjEg[1]].LPoint == P)
							|| (edge[edge[e].rpotherPgnAdjEg[1]].odd == 0 &&
								edge[edge[e].rpotherPgnAdjEg[1]].RPoint == P)))
					{
						e = edge[e].rpotherPgnAdjEg[1];
					}
					else
					{
						e = edge[e].rpAdjEg;
					}
				}
				else
				{
					ansv.push_back(edge[e].RPoint);
					P = edge[e].LPoint;
					if (edge[e].pgn == 1
						&& edge[e].lpotherPgnAdjEg[0] != -1
						&& edge[edge[e].lpotherPgnAdjEg[0]].inout == 1
						&& ((edge[edge[e].lpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].lpotherPgnAdjEg[0]].RPoint == P)
							|| (edge[edge[e].lpotherPgnAdjEg[0]].odd == 0 &&
								edge[edge[e].lpotherPgnAdjEg[0]].LPoint == P)))
					{
						e = edge[e].lpotherPgnAdjEg[0];
					}
					else if (edge[e].pgn == 1
						&& edge[e].lpotherPgnAdjEg[1] != -1
						&& edge[edge[e].lpotherPgnAdjEg[1]].inout == 1
						&& ((edge[edge[e].lpotherPgnAdjEg[1]].odd == 1 &&
								edge[edge[e].lpotherPgnAdjEg[1]].RPoint == P)
							|| (edge[edge[e].lpotherPgnAdjEg[1]].odd == 0 &&
								edge[edge[e].lpotherPgnAdjEg[1]].LPoint == P)))
					{
						e = edge[e].lpotherPgnAdjEg[1];
					}
					else if (edge[e].pgn == 2
						&& edge[e].lpotherPgnAdjEg[0] != -1
						&& edge[edge[e].lpotherPgnAdjEg[0]].inout == 0
						&& ((edge[edge[e].lpotherPgnAdjEg[0]].odd == 1 &&
								edge[edge[e].lpotherPgnAdjEg[0]].LPoint == P)
							|| (edge[edge[e].lpotherPgnAdjEg[0]].odd == 0 &&
								edge[edge[e].lpotherPgnAdjEg[0]].RPoint == P)))
					{
						e = edge[e].lpotherPgnAdjEg[0];
					}
					else if (edge[e].pgn == 2
						&& edge[e].lpotherPgnAdjEg[1] != -1
						&& edge[edge[e].lpotherPgnAdjEg[1]].inout == 0
						&& ((edge[edge[e].lpotherPgnAdjEg[1]].odd == 1 &&
								edge[edge[e].lpotherPgnAdjEg[1]].LPoint == P)
							|| (edge[edge[e].lpotherPgnAdjEg[1]].odd == 0 &&
								edge[edge[e].lpotherPgnAdjEg[1]].RPoint == P)))
					{
						e = edge[e].lpotherPgnAdjEg[1];
					}
					else
					{
						e = edge[e].lpAdjEg;
					}
				}
			}
			while (ansv[0] != P);
			std::vector<Point>tmp;
            tmp.reserve(ansv.size());
            for (auto p: ansv) tmp.emplace_back(p);
            tmp.push_back(ansv.front());
            ans.emplace_back(tmp);
		}
	}
}

class BooleanPlugin : public QObject, BaseInterface, ToolboxInterface, LoggingInterface, LoadSaveInterface
{
	Q_OBJECT
	Q_INTERFACES(BaseInterface)
	Q_INTERFACES(ToolboxInterface)
	Q_INTERFACES(LoggingInterface)
	Q_INTERFACES(LoadSaveInterface)
	Q_PLUGIN_METADATA(IID "cn.edu.zju.fhp.booleanPlugin")

signals:
	void updateView();
	void updatedObject(int _identifier, const UpdateType& _type);

	void log(Logtype _type, QString _message);
	void log(QString _message);

	void addToolbox(QString _name, QWidget* _widget);

	void addEmptyObject(DataType _type, int& _id);

public:
	BooleanPlugin();

	QString name() { return QString("Boolean"); }

	QString description() { return QString("Does actually nothing but works!"); }

public:
	using polygon = typename std::vector<OpenMesh::Vec3d>;
	using polygon_ptr = typename std::shared_ptr<polygon>;

private:
	polygon_ptr APolygon;
	polygon_ptr BPolygon;
	MPolygon MpA;
	MPolygon MpB;

	int obj_id_1, obj_id_2, result_obj_id;

	bool load(const std::string& filePath, MPolygon& p, PolyLineCollection* polygons);

	void clearSourceObjects();
	void clearResultObject();

private slots:
	void loadA();
	void loadB();

	void executeUnion();
	void executeInter();
	void executeSub();

	void initializePlugin();
	void pluginsInitialized();

	void reset();

public slots:
	QString version() { return QString("Mesh-based"); };
};
#endif
