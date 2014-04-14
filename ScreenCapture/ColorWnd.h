#ifndef  COLORWND_H
#define  COLORWND_H

#include "Toolbar.h"

class CColorWnd : public CWndImpl
{
public:
	CColorWnd();
	~CColorWnd();

public:
	LPCTSTR  GetWindowClassName() const;
	void Initialize(void);
	BOOL SetWndBkColor(COLORREF dwColor);

	void CreatePenSizeTool(void);
	void InitPenSizeToolBtn(void);
	void InitPenSizeToolImage(void);

	void CreateCurColorSS(void);

	void CreateColorTool(void);
	void InitColorToolBtn(void);
	void InitColorToolImage(void);
	

	LRESULT ProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnCtlColorStatic(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	//LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);

	////HWND CreateButton(LPCTSTR szName,int ID,HBITMAP hBtnBMP);

	//LRESULT ProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

	//LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	//LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	//LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	//LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	/*LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);*/
	//LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	
	/*LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);*/

private:
	HBRUSH m_hBrushBk;

	CToolBarWnd* m_pPenSizeTool;
	TBBUTTON* m_pPenSizeToolBtn;
	HBITMAP* m_pPenSizeTBNormalImage;

	HWND m_hStatic;
	HBRUSH m_hStaticBrush;
	COLORREF m_dwSSBrushColor;

	CToolBarWnd* m_pColorTool;
	TBBUTTON* m_pColorToolBtn;
	HBITMAP* m_pColorTBNormalImage;
	//HBITMAP* m_pColorTBHotImage;
};

#endif