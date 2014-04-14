#ifndef  TOOLWND_H
#define  TOOLWND_H

#include "Toolbar.h"
#include "ColorWnd.h"
#include <bitset>

#define  N_GRAPH   5

class CToolWnd : public CWndImpl
{
public:
	CToolWnd();
	~CToolWnd();

public:
	LPCTSTR  GetWindowClassName() const;
	void Initialize(void);
	BOOL InitTbButtonArr(void);
	BOOL InitTbNormalImageArr(void);
	BOOL InitTbHotImageArr(void);
	void CreateToolbar(void);

	BOOL SetWndBkColor(COLORREF dwColor);

	LRESULT ProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);

private:
	CToolBarWnd* m_pToolBar;
	TBBUTTON* m_pTbBtn;
	HBITMAP* m_pTbNormalImage;
	HBITMAP* m_pTbHotImage;

	int m_nTbBtnCnt;

	HBRUSH m_hBrushBk;

private:
	std::bitset<N_GRAPH> m_bsChecked;    //判断哪个绘图操作被选中

};

#endif