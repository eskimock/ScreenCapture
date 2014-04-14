#ifndef  DRAWGRAPH_HPP
#define  DRAWGRAPH_HPP

#include "stdafx.h"

class IGraph
{
public:
	IGraph()
	{

	}
	virtual ~IGraph()
	{

	}
public:
	virtual void DrawGraph(HDC hDC, POINT ptStart, POINT ptEnd, int nPenWidth, COLORREF dwPenColor, const RectX& rcRgn = SCREEN_RC) = 0;
};

class GraphRectangle : public IGraph
{
public:
	GraphRectangle()
	{

	}
	~GraphRectangle()
	{

	}
public:
	void DrawGraph(HDC hDC, POINT ptStart, POINT ptEnd, int nPenWidth, COLORREF dwPenColor,const RectX& rcRgn = SCREEN_RC);
};

class GraphEllipse : public IGraph
{
public:
	GraphEllipse()
	{

	}
	~GraphEllipse()
	{

	}
public:
	void DrawGraph(HDC hDC, POINT ptStart, POINT ptEnd, int nPenWidth, COLORREF dwPenColor, const RectX& rcRgn = SCREEN_RC);
};

class GraphArrowLine : public IGraph
{
public:
	GraphArrowLine()
	{

	}
	~GraphArrowLine()
	{

	}
public:
	void DrawGraph(HDC hDC, POINT ptStart, POINT ptEnd, int nPenWidth, COLORREF dwPenColor, const RectX& rcRgn = SCREEN_RC);
};

class GraphScrawl : public IGraph
{
public:
	GraphScrawl()
	{

	}
	~GraphScrawl()
	{

	}
public:
	void DrawGraph(HDC hDC, POINT ptStart, POINT ptEnd, int nPenWidth, COLORREF dwPenColor, const RectX& rcRgn = SCREEN_RC);
};

class GraphHighlight : public IGraph
{
public:
	GraphHighlight()
	{

	}
	~GraphHighlight()
	{

	}
public:
	void DrawGraph(HDC hDC, POINT ptStart, POINT ptEnd, int nPenWidth, COLORREF dwPenColor, const RectX& rcRgn = SCREEN_RC);
};

class GraphFactory
{
public:
	GraphFactory()
	{

	}
	virtual ~GraphFactory()
	{

	}
public:
	static IGraph* CreateGraph(int graphID);
};


#endif

