#ifndef  TOOLBARWND_H
#define  TOOLBARWND_H

#include "WndImpl.hpp"

class CToolBarWnd : public CWndImpl
{
public:
	CToolBarWnd(TBBUTTON* pTbBtn, int nBtnCnt,int nSepCnt,int cxImage,int cyImage);
	~CToolBarWnd();
public:
	LPCTSTR GetWindowClassName() const;
	void Initialize(void);
	BOOL SetTbButtonImage(HBITMAP* phImage,UINT uMsgSetImage = TB_SETIMAGELIST,COLORREF crMask = RGB(255,255,255));

private:
	TBBUTTON* m_pTbButton;           //指向toolbar上的按钮(包括分割线)数组

	int m_nTbButtonCnt;              //工具栏上的按钮数
	int m_nSepCnt;                   //分隔线数

	int m_cxImage;
	int m_cyImage;
};

#endif