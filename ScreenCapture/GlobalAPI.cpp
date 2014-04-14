#include "stdafx.h"
#include "GlobalAPI.h"

namespace GlobalAPI
{
	BOOL BitBltEx(HDC hDestDC,RectX rc,HDC hSrcDC, POINT ptStart, DWORD dwRop)
	{
		return BitBlt(hDestDC,rc.leftX,rc.topX,rc.GetW(),rc.GetH(),hSrcDC,ptStart.x,ptStart.y,dwRop);
	}

	int DrawTextEx(HDC hDC, const CString& strTx, RectX rc, UINT uFormat)
	{
		RECT rcTemp = rc.ToRECT();
		return DrawText(hDC,strTx,lstrlen(strTx),&rcTemp,uFormat);
	}

	BOOL RectangleX(HDC hDC, RectX rc)
	{
		return Rectangle(hDC,rc.leftX,rc.topX,rc.rightX,rc.bottomX);
	}

	BOOL EllipseX(HDC hDC, RectX rc)
	{
		return Ellipse(hDC,rc.leftX,rc.topX,rc.rightX,rc.bottomX);
	}


	void DrawScrawl(HDC hDC, POINT ptStart, POINT ptEnd)
	{
		HPEN hPen = CreatePen(PS_SOLID, 15, RGB(50, 150, 255));
		HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);

		MoveToEx(hDC, ptStart.x, ptStart.y, NULL);
		LineTo(hDC, ptEnd.x, ptEnd.y);

		::SelectObject(hDC, hOldPen);
		::DeleteObject(hPen);
	}
}


