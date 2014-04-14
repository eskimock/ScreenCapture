#ifndef  RECTX_HPP
#define  RECTX_HPP

#include "stdafx.h"

typedef struct CRectEx
{
	int leftX;
	int topX;
	int rightX;
	int bottomX;

public:
	CRectEx()
	{
		leftX = 0;
		topX = 0;
		rightX = 0;
		bottomX = 0;
	}
	CRectEx(int left,int top,int right,int bottom)
	{
		leftX = left;
		topX = top;
		rightX = right;
		bottomX = bottom;
	}

	CRectEx(POINT ptStart, POINT ptEnd)
	{
		leftX = ptStart.x;
		topX = ptStart.y;
		rightX = ptEnd.x;
		bottomX = ptEnd.y;
	}

	CRectEx(RECT rc)
	{
		leftX = rc.left;
		topX = rc.top;
		rightX = rc.right;
		bottomX = rc.bottom;
	}

	CRectEx(const CRectEx& rc)
	{
		leftX = rc.leftX;
		topX = rc.topX;
		rightX = rc.rightX;
		bottomX = rc.bottomX;
	}

	CRectEx& operator=(const CRectEx& rc)
	{
		leftX = rc.leftX;
		topX = rc.topX;
		rightX = rc.rightX;
		bottomX = rc.bottomX;

		return *this;
	}

	//转换为标准矩形（左上右下）
	void ResetStartEnd(void)
	{
		if(leftX > rightX)  std::swap(leftX,rightX);
		if(topX > bottomX)  std::swap(topX,bottomX);
	}

	POINT GetStartPoint(void)
	{
		POINT pt = {leftX,topX};
		return pt;
	}

	POINT GetEndPoint(void)
	{
		POINT pt = {rightX,bottomX};
		return pt;
	}

	int GetH(void) const
	{
		return bottomX - topX;
	}

	int GetW(void) const
	{
		return rightX - leftX;
	}

	RECT ToRECT(void)
	{
		RECT rcConvert = {leftX,topX,rightX,bottomX};
		return rcConvert;
	}

	BOOL IsInvalid(void)
	{
		return (rightX == leftX) || (bottomX == topX);
	}

	BOOL PtInRectX(POINT pt) const
	{
		return (((pt.x > leftX) && (pt.x < rightX)) 
			     || ((pt.x < leftX) && (pt.x) > rightX))
			    && (((pt.y < topX) && (pt.y > bottomX))
				 || ((pt.y > topX) && (pt.y < bottomX)));
	}

    BOOL OutOfBoundary(const CRectEx& rcBoundary)
	{
		CRectEx rcTemp(*this);
		rcTemp.ResetStartEnd();
		return (rcTemp.leftX < rcBoundary.leftX) || (rcTemp.topX < rcBoundary.topX)\
			   || (rcTemp.rightX > rcBoundary.rightX) || (rcTemp.bottomX > rcBoundary.bottomX);
	}

	//标准矩形,亦可用AdjustPos
	void ResetBoundary(const CRectEx& rcBoundary)
	{
		if(leftX < rcBoundary.leftX)   leftX = rcBoundary.leftX;
		if(topX < rcBoundary.topX)   topX = rcBoundary.topX;
		if(rightX > rcBoundary.rightX)   rightX = rcBoundary.rightX;
		if(bottomX > rcBoundary.bottomX)   bottomX = rcBoundary.bottomX;
	}
	
	//rcCmp务必为标准矩形
	BOOL operator ==(const CRectEx& rcCmp)
	{
		CRectEx rcTemp = *this;
		rcTemp.ResetStartEnd();
		return (rcTemp.leftX == rcCmp.leftX)
			   && (rcTemp.topX == rcCmp.topX)
			   && (rcTemp.rightX == rcCmp.rightX)
			   && (rcTemp.bottomX == rcCmp.bottomX);
	}

	BOOL operator != (const CRectEx& rcCmp)
	{
		CRectEx rcTemp = *this;
		rcTemp.ResetStartEnd();
		return (rcTemp.leftX != rcCmp.leftX)
			   || (rcTemp.topX != rcCmp.topX)
			   || (rcTemp.rightX != rcCmp.rightX)
			   || (rcTemp.bottomX != rcCmp.bottomX);
	}

	void AdjustPos(const CRectEx& rcBoundary)
	{
		CRectEx rcTemp = *this;
		rcTemp.ResetStartEnd();

		if(rcTemp.leftX <= rcBoundary.leftX)
		{
			rcTemp.rightX -= rcTemp.leftX;
			if(rcTemp.rightX >= rcBoundary.rightX)
				rcTemp.rightX = rcBoundary.rightX;
			rcTemp.leftX = rcBoundary.leftX;
		}
		if(rcTemp.topX <= rcBoundary.topX)
		{
			rcTemp.bottomX -= rcTemp.topX;
			if(rcTemp.bottomX >= rcBoundary.bottomX)
				rcTemp.bottomX = rcBoundary.bottomX;
			rcTemp.topX = rcBoundary.topX;
		}
		if(rcTemp.rightX >= rcBoundary.rightX)
		{
			rcTemp.leftX -= rcTemp.rightX - rcBoundary.rightX; 
			if(rcTemp.leftX <= rcBoundary.leftX)
				rcTemp.leftX = rcBoundary.leftX;
			rcTemp.rightX = rcBoundary.rightX;	
		}
		if(rcTemp.bottomX >= rcBoundary.bottomX)
		{
			rcTemp.topX -= rcTemp.bottomX - rcBoundary.bottomX;
			if(rcTemp.topX <= rcBoundary.topX)
				rcTemp.topX = rcBoundary.topX;
			rcTemp.bottomX = rcBoundary.bottomX;
		}

		if(leftX > rightX)
			std::swap(rcTemp.leftX,rcTemp.rightX);
		if(topX > bottomX)
			std::swap(rcTemp.topX,rcTemp.bottomX);

		*this = rcTemp;
		
	}

} RectX, *PRectX;

#endif