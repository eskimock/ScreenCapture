#ifndef SCRNCAP_H
#define SCRNCAP_H

#include "WndImpl.hpp"
#include "DrawGraph.hpp"
#include "ToolWnd.h"

typedef enum
{
	ACTION_CHOOSING,
	ACTION_ADJUST,
	ACTION_RECT,
	ACTION_ELLIPSE,
	ACTION_ARROW,
	ACTION_SCRAWL,
	ACTION_HIGHLIGHT,
	ACTION_SAVE,
	ACTION_UNDO,
	ACTION_REDO
}ACTION;

//拉伸方向L:left,R:right,U:up,D:down
typedef enum
{
	STRETCH_NO,
	STRETCH_L,
	STRETCH_U,
	STRETCH_R,
	STRETCH_D,
	STRETCH_LU,
	STRETCH_RU,
	STRETCH_LD,
	STRETCH_RD
}STRETCH;

class CScrnCapWnd: public CWndImpl
{
public:
	CScrnCapWnd();
	~CScrnCapWnd();

public:
	LPCTSTR   GetWindowClassName() const;
	void Initialize(void);

	LRESULT ProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

	//sys msg processing
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDbClk(WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);

	//user msg processing
	LRESULT OnRecap(WPARAM wParam, LPARAM lParam);
	LRESULT OnSaveImage(WPARAM wParam, LPARAM lParam);
	LRESULT OnRectangle(WPARAM wParam, LPARAM lParam);
	LRESULT OnEllipse(WPARAM wParam, LPARAM lParam);
	LRESULT OnArrowLine(WPARAM wParam, LPARAM lParam);
	LRESULT OnScrawl(WPARAM wParam, LPARAM lParam);
	LRESULT OnHighlight(WPARAM wParam, LPARAM lParam);
	LRESULT OnUndo(WPARAM wParam, LPARAM lParam);
	LRESULT OnRedo(WPARAM wParam, LPARAM lParam);
	LRESULT OnCancel(WPARAM wParam, LPARAM lParam);
	LRESULT OnFinish(WPARAM wParam, LPARAM lParam);

	LRESULT OnPenSizeChange(WPARAM wParam, LPARAM lParam);
	LRESULT OnPenColorChange(WPARAM wParam, LPARAM lParam);

public:
	void InitializeDC(void);

	// @brief  创建截屏工具栏
	void CreateToolWnd(void);
	// @brief  创建颜色选择框
	void CreateColorWnd(BOOL bShow);
	// @brief  自动识别窗口
	BOOL AutoRecognizeWnd(const POINT& ptMousePos);

	// @brief  调整已选区域
	void Stretch_Adjust(const STRETCH& emStretch,RectX& rcStretch,int cxOffset,int cyOffset);
	void Drag_Adjust(RectX& rcDrag,int cxOffset,int cyOffset);
	BOOL Adjust(int cxOffset,int cyOffset);

	// @brief  调整截屏工具栏位置
	void AdjustToolPos(void);

	void Enlarge(RectX& rcEnlarge, int iSpeed);

	//@brief  在m_hMemPaintDC上绘制已选(调整后)区域
	void PaintSelRgn(void);   

	void Undo(void);
	void Redo(void);

	BOOL Save(void);

	void Finish(void);

	void ClearUndoStack(void);
	void ClearRedoStack(void);

	//@brief  API rewriting
	static BOOL BitBltEx(HDC hDestDC,RectX rc,HDC hSrcDC, POINT ptStart, DWORD dwRop);
	static int  DrawTextEx(HDC hDC, const CString& strTx, RectX rc, UINT uFormat);

	static BOOL RectangleX(HDC hDC, RectX rc); 


	//Painting

	//@brief  在矩形的每条边线中点和顶点画实心小方块
	static void DrawAdjustSquare(HDC hDC,const RectX& rc, const int iSide); 
	static void DrawRect(HDC hDC, const RectX& rcDraw, const int& iPenWidth, const int& iPenStyle);
	static void DrawSizeTx(HDC hDC, const RectX& rcSel);

	//@brief  枚举所有窗口
	static BOOL CALLBACK EnumGrandChildWindowsProc(HWND hwnd,LPARAM lParam);
	static BOOL CALLBACK EnumChildWindowsProc(HWND hwnd,LPARAM lParam);
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
	static BOOL InsertWndRect(HWND hwnd);
	static std::vector<RectX> GetAllWndRect();

	static void ConvertToGrayBitmap(HBITMAP hSourceBmp,HDC sourceDC);


	//@brief  光标设置
	void SetScrnCursor(HWND hWnd, const RectX& rcCursorLie, const BOOL& bLButtonDown, const ACTION& m_emAction, STRETCH& emStretch);
	static void SetStretchCursor(const RectX& rc,const POINT& ptPos, STRETCH& emStretch);
	//@brief  根据鼠标位置获得伸缩方向
	static STRETCH GetStrechDrct(const RectX& rc,const POINT& ptPos);


	//保存相关
	static BOOL GetSaveFilePath(HINSTANCE hInstance,HWND hWnd, TCHAR* szFilePath, TCHAR* szFileTitle,int& nFilterIndex);
	static void GetCurTimeString(CString& strCurDateTime);
	static void SetDefFileName(CString& strDefName);

	static BOOL GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	/**
	 * @brief  将HBITMAP保存为指定格式图片
	 * @param  hImage[IN]，位图句柄
	 * @param  pFileName[IN]，指定的完整路径及图片名
	 * @param  strFormat[IN]，图片格式，形如"image/png"
	 * @return 保存是否成功
	**/
	static BOOL SaveAsPic(HBITMAP hImage, const CString& pFileName, const CString& strFormat);

	//@brief  拷至剪贴板
	static BOOL CopyBMP2Clipboard(HBITMAP hBMP,HWND hWnd);



private:

	HDC  m_hClientDC;       //屏幕DC，展示截屏过程
	HDC  m_hMemPaintDC;     //绘制DC，主要向m_hClientDC提供数据
	HDC  m_hMemDC;          //向m_hMemPaintDC提供灰色区域
	HDC  m_hMemCurScrnDC;   //保存m_hMemPaintDC的纯净区域

	HBITMAP m_hScrnBMP, m_hOldScrnBMP;       //正常BMP
	HBITMAP m_hGrayBMP, m_hOldGrayBMP;
	HBITMAP m_hGrayBMP2, m_hOldGrayBMP2;
	HBITMAP m_hCurScrnBMP, m_hOldCurScrnBMP; //对应m_hMemCurScrnDC

	HBITMAP m_hGraphBMP, m_hOldGraphBMP;     //利用双缓冲画橡皮筋矩形/椭圆/箭头


	HBITMAP m_hCurBMP, m_hOldCurBMP;         //压入栈的最新屏幕
	std::stack<HBITMAP> m_stackUndoGraph;    //"撤销"栈
	std::stack<HBITMAP> m_stackRedoGraph;    //"恢复"栈


	POINT m_ptStart;
	POINT m_ptMoving;

	BOOL m_bLBtnDown;    //左键按下
	BOOL m_bStretching;  //是否正在拉伸

	ACTION m_emAction;    //用户操作
	STRETCH m_emStrech;   //拉伸方向，当且仅当m_emAction为ACTION_ADJUST

	RectX m_rcSel;       //已选截屏区域（截屏过程中始终为标准矩形）
	RectX m_rcChoosing;  //动态区域
	RectX m_rcSelRedo;   //用来保存恢复操作前的截图区域

	IGraph *m_pGraph;
	CToolWnd* m_pToolWnd;
	CColorWnd* m_pColorWnd;

	int m_nPenWidth;
	COLORREF m_dwPenColor;

private:
	static HCURSOR m_hCursor;
	static std::vector<RectX> m_vecAllWndRect;
};

#endif