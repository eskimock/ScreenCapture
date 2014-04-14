#include "stdafx.h"
#include "ScrnCapWnd.h"


std::vector<RectX> CScrnCapWnd::m_vecAllWndRect;
HCURSOR CScrnCapWnd::m_hCursor = NULL;

CScrnCapWnd::CScrnCapWnd()
{
	m_ptStart = ZERO_PT;
    m_ptMoving = ZERO_PT;

	m_rcSel = ZERO_RC;
	m_rcChoosing = ZERO_RC;
	m_rcSelRedo = ZERO_RC;

	m_bLBtnDown   = FALSE;
	m_bStretching = FALSE;

	m_emAction  = ACTION_CHOOSING;
	m_emStrech = STRETCH_NO;

	m_pGraph = NULL;
	m_pToolWnd = NULL;

	m_pColorWnd = NULL;

	m_nPenWidth = 1;
	m_dwPenColor = RGB(255,0,0);
}

CScrnCapWnd::~CScrnCapWnd()
{
	if(NULL != m_pGraph)
	{
		delete m_pGraph;
		m_pGraph = NULL;
	}

	//在窗口销毁时已删除自身，无需重复delete，否则会有异常
	if(NULL != m_pToolWnd)
	{
		m_pToolWnd = NULL;
	}
	if(NULL != m_pColorWnd)
	{
		m_pColorWnd = NULL;
	}
}

void CScrnCapWnd::Initialize()
{
	HICON APPICON_BIG = ::LoadIcon(GetInstance(),MAKEINTRESOURCE(IDI_APPICON_BIG));
	HICON APPICON_SMALL = ::LoadIcon(GetInstance(),MAKEINTRESOURCE(IDI_APPICON_SMALL));
	SetIcon(APPICON_BIG,APPICON_SMALL);
}

LPCTSTR CScrnCapWnd::GetWindowClassName() const
{
	return _T("CScrnCapWnd");
}

void CScrnCapWnd::CreateToolWnd()
{
	if(NULL != m_pToolWnd)
	{
		if(!::IsWindow(m_pToolWnd->GetSafeHwnd()))
		{
			delete m_pToolWnd;
			m_pToolWnd = NULL;
		}
	}
	else
	{
		m_pToolWnd = new CToolWnd();
		m_pToolWnd->Create(GetSafeHwnd(),_T("ToolMenu"),WS_POPUP,WS_EX_TOPMOST|WS_EX_TOOLWINDOW,TOOLWND_RC);
	}

	ShowWindow(m_pToolWnd->GetSafeHwnd(),SW_HIDE);
}

void CScrnCapWnd::CreateColorWnd(BOOL bShow)
{
	if(NULL != m_pColorWnd)
	{
		if(!::IsWindow(m_pColorWnd->GetSafeHwnd()))
		{
			delete m_pColorWnd;
			m_pColorWnd = NULL;
		}
	}
	else
	{
		m_pColorWnd = new CColorWnd();
		m_pColorWnd->Create(GetSafeHwnd(),_T("colorSel"),WS_POPUP,WS_EX_TOPMOST|WS_EX_TOOLWINDOW,COLORWND_RC);
	}

	RECT rcToolWnd;
	::GetWindowRect(m_pToolWnd->GetSafeHwnd(),&rcToolWnd);
	RectX rcToolWndX(rcToolWnd);
	RectX rcColorWndX;
	rcColorWndX.leftX = rcToolWndX.leftX;
	rcColorWndX.rightX = rcColorWndX.leftX + m_pColorWnd->GetWindowSize().cx;
	if(((rcToolWndX.bottomX < m_rcSel.topX) && (rcToolWndX.topX - 2 > m_pColorWnd->GetWindowSize().cy))
		|| (SCREEN_Y - rcToolWndX.bottomX - 2 < m_pColorWnd->GetWindowSize().cy))
	{
		rcColorWndX.bottomX = rcToolWndX.topX - 2;
		rcColorWndX.topX = rcColorWndX.bottomX - m_pColorWnd->GetWindowSize().cy;
	}
	else
	{
		rcColorWndX.topX = rcToolWndX.bottomX + 2;
		rcColorWndX.bottomX = rcColorWndX.topX + m_pColorWnd->GetWindowSize().cy;
	}
	MoveWindow(m_pColorWnd->GetSafeHwnd(),rcColorWndX.leftX,rcColorWndX.topX,rcColorWndX.GetW(),rcColorWndX.GetH(),FALSE);
	
	if(!bShow)
	{
		m_emAction = m_stackUndoGraph.size() == 0 ? ACTION_ADJUST:ACTION_REDO;
	}

	int iCmdShow = bShow ? SW_SHOW:SW_HIDE;
	ShowWindow(m_pColorWnd->GetSafeHwnd(),iCmdShow);

	::SetFocus(GetSafeHwnd());
}

void CScrnCapWnd::InitializeDC()
{
	HDC hDispDC = CreateDC(_T("DISPLAY"),NULL,NULL,NULL);
	m_hMemDC = CreateCompatibleDC(hDispDC);

	//原始BMP
	m_hScrnBMP = CreateCompatibleBitmap(hDispDC,SCREEN_X,SCREEN_Y);
	m_hOldScrnBMP = (HBITMAP)SelectObject(m_hMemDC,m_hScrnBMP);
	BitBltEx(m_hMemDC,SCREEN_RC,hDispDC,ZERO_PT,SRCCOPY|CAPTUREBLT);
	SelectObject(m_hMemDC,m_hOldScrnBMP);

	//灰色BMP
	m_hGrayBMP = CreateCompatibleBitmap(hDispDC,SCREEN_X,SCREEN_Y);
	m_hOldGrayBMP = (HBITMAP)SelectObject(m_hMemDC,m_hGrayBMP);
	BitBltEx(m_hMemDC,SCREEN_RC,hDispDC,ZERO_PT,SRCCOPY|CAPTUREBLT);
	ConvertToGrayBitmap(m_hGrayBMP,m_hMemDC); //转成灰色BMP
	SelectObject(m_hMemDC,m_hOldGrayBMP);

	//m_hGrayBMP全程被m_hMemPaintDC使用，故新建相同灰色BMP供操作用
	m_hGrayBMP2 = CreateCompatibleBitmap(hDispDC,SCREEN_X,SCREEN_Y);
	m_hOldGrayBMP2 = (HBITMAP)SelectObject(m_hMemDC,m_hGrayBMP2);
	BitBltEx(m_hMemDC,SCREEN_RC,hDispDC,ZERO_PT,SRCCOPY|CAPTUREBLT);
	ConvertToGrayBitmap(m_hGrayBMP2,m_hMemDC); //转成灰色BMP
	SelectObject(m_hMemDC,m_hOldGrayBMP2);

	//绘制DC
	m_hMemPaintDC = CreateCompatibleDC(hDispDC);
	m_hOldScrnBMP = (HBITMAP)SelectObject(m_hMemPaintDC,m_hGrayBMP);

	//保存m_hMemPaintDC
	m_hMemCurScrnDC = CreateCompatibleDC(hDispDC);
	m_hCurScrnBMP = CreateCompatibleBitmap(hDispDC,SCREEN_X,SCREEN_Y);
	m_hOldCurScrnBMP = (HBITMAP)SelectObject(m_hMemCurScrnDC,m_hCurScrnBMP);

	DeleteDC(hDispDC);
}

BOOL CScrnCapWnd::AutoRecognizeWnd(const POINT& ptMousePos)
{
	if(m_vecAllWndRect.size() == 0)
		return FALSE;
	
	auto itWndRc = m_vecAllWndRect.begin();
	while(itWndRc != m_vecAllWndRect.end())
	{
		if(itWndRc->PtInRectX(ptMousePos))
		{
			m_rcChoosing = *itWndRc;
			return TRUE;
		}
		else
		{
			++itWndRc;
		}
	}

	return FALSE;
}


void CScrnCapWnd::Stretch_Adjust(const STRETCH& emStretch,RectX& rcStretch,int cxOffset,int cyOffset)
{
	switch(emStretch)
	{
	default:
		break;
	case STRETCH_L:
		rcStretch.leftX += cxOffset;
		break;
	case STRETCH_R:
		rcStretch.rightX += cxOffset;
		break;
	case STRETCH_U:
		rcStretch.topX += cyOffset;
		break;
	case STRETCH_D:
		rcStretch.bottomX += cyOffset;
		break;
	case STRETCH_LU:
		rcStretch.leftX += cxOffset;
		rcStretch.topX += cyOffset;
		break;
	case STRETCH_LD:
		rcStretch.leftX += cxOffset;
		rcStretch.bottomX += cyOffset;
		break;
	case STRETCH_RU:
		rcStretch.rightX += cxOffset;
		rcStretch.topX += cyOffset;
		break;
	case STRETCH_RD:
		rcStretch.rightX += cxOffset;
		rcStretch.bottomX += cyOffset;
		break;

	}
}

void CScrnCapWnd::Drag_Adjust(RectX& rcDrag,int cxOffset,int cyOffset)
{
	rcDrag.leftX = rcDrag.leftX + cxOffset;
	rcDrag.topX = rcDrag.topX + cyOffset;
	rcDrag.rightX = rcDrag.rightX + cxOffset;
	rcDrag.bottomX = rcDrag.bottomX + cyOffset;
}

BOOL CScrnCapWnd::Adjust(int cxOffset,int cyOffset)
{
	//拉伸
	if(m_bStretching)
	{
		Stretch_Adjust(m_emStrech,m_rcChoosing,cxOffset,cyOffset);

		m_rcSel = m_rcChoosing;
		m_rcSel.ResetStartEnd();

		if(m_rcSel.leftX == m_rcSel.rightX)
		{
			m_rcSel.rightX++;
		}
		if(m_rcSel.topX == m_rcSel.bottomX)
		{
			m_rcSel.bottomX++;
		}
		

		return TRUE;
	}
	else
	{
		//拖拽
		if(m_rcSel.PtInRectX(m_ptMoving))
		{
			m_rcChoosing = m_rcSel;
			Drag_Adjust(m_rcChoosing,cxOffset,cyOffset);
			m_rcSel = m_rcChoosing;
			m_rcSel.ResetStartEnd();

			return TRUE;
		}
	}

	return FALSE;
}

void CScrnCapWnd::AdjustToolPos()
{
	SIZE  sizeTool = m_pToolWnd->GetWindowSize();
	
	RectX rcAdjust;
	if(SCREEN_Y - m_rcSel.bottomX >= sizeTool.cy + 5)
	{
		rcAdjust.leftX = m_rcSel.rightX - sizeTool.cx;
		rcAdjust.topX = m_rcSel.bottomX + 5;		
		rcAdjust.rightX = m_rcSel.rightX;
		rcAdjust.bottomX = m_rcSel.bottomX + sizeTool.cy + 5;
	}
	else
	{
		//80为"W×H"的宽度
		int iOffset = m_rcSel.GetW() - 80 > sizeTool.cx? 5 : 18;

		rcAdjust.leftX = m_rcSel.rightX - sizeTool.cx;
		rcAdjust.topX = m_rcSel.topX - sizeTool.cy - iOffset;
		rcAdjust.rightX = m_rcSel.rightX;
		rcAdjust.bottomX = m_rcSel.topX - iOffset;
	}

	if(rcAdjust.OutOfBoundary(SCREEN_RC))
	{
		rcAdjust.AdjustPos(SCREEN_RC);
	}

	MoveWindow(m_pToolWnd->GetSafeHwnd(),rcAdjust.leftX ,rcAdjust.topX,rcAdjust.GetW(),rcAdjust.GetH(),TRUE);
	if(!IsWindowVisible(m_pToolWnd->GetSafeHwnd()))
	{
		ShowWindow(m_pToolWnd->GetSafeHwnd(),SW_SHOW);
	}
	::SetFocus(GetSafeHwnd());
}

void CScrnCapWnd::Enlarge(RectX& rcEnlarge, int iSpeed)
{
	rcEnlarge.leftX -= iSpeed;
	rcEnlarge.topX -= iSpeed;
	rcEnlarge.rightX += iSpeed;
	rcEnlarge.bottomX += iSpeed;
}

void CScrnCapWnd::PaintSelRgn()
{
	m_hOldGrayBMP2 = (HBITMAP)::SelectObject(m_hMemDC,m_hGrayBMP2);
	BitBltEx(m_hMemPaintDC,SCREEN_RC,m_hMemDC,ZERO_PT,SRCCOPY|CAPTUREBLT);
	SelectObject(m_hMemDC,m_hOldGrayBMP2);

	m_hOldScrnBMP = (HBITMAP)::SelectObject(m_hMemDC,m_hScrnBMP);
	BitBltEx(m_hMemPaintDC,m_rcSel,m_hMemDC,m_rcSel.GetStartPoint(),SRCCOPY|CAPTUREBLT);
	SelectObject(m_hMemDC,m_hOldScrnBMP);
}

void CScrnCapWnd::Undo()
{
	if(m_stackUndoGraph.empty())
	{
		m_emAction = ACTION_CHOOSING;
		m_rcSel = m_rcChoosing = RectX();

		ClearRedoStack();

		if(m_pToolWnd != NULL)
		{
			delete m_pToolWnd;
			m_pToolWnd = NULL;
		}

		if(m_pColorWnd != NULL)
		{
			delete m_pColorWnd;
			m_pColorWnd = NULL;
		}
	}
	else
	{
		if(m_stackRedoGraph.empty())
		{
			m_hCurBMP = CreateCompatibleBitmap(m_hMemCurScrnDC,SCREEN_X,SCREEN_Y);
			m_hOldCurBMP = (HBITMAP)SelectObject(m_hMemDC,m_hCurBMP);
			BitBltEx(m_hMemDC,SCREEN_RC,m_hMemCurScrnDC,ZERO_PT,SRCCOPY|CAPTUREBLT);
			SelectObject(m_hMemDC,m_hOldCurBMP);

		}

		m_stackRedoGraph.push(m_hCurBMP);

		m_hCurBMP = m_stackUndoGraph.top();
		m_hOldCurBMP = (HBITMAP)SelectObject(m_hMemDC,m_hCurBMP);
		BitBltEx(m_hMemCurScrnDC,SCREEN_RC,m_hMemDC,ZERO_PT,SRCCOPY|CAPTUREBLT);
		SelectObject(m_hMemDC,m_hOldCurBMP);

		m_stackUndoGraph.pop();

		m_rcSelRedo = m_rcSel;

		if(m_stackUndoGraph.size() == 0)
		{
			
			m_emAction = IsWindowVisible(m_pColorWnd->GetSafeHwnd())?m_emAction:ACTION_ADJUST;
		}
	}
}

void CScrnCapWnd::Redo()
{
	if(m_rcSelRedo != m_rcSel)
	{
		ClearRedoStack();
		return ;
	}
	if(!m_stackRedoGraph.empty())
	{
		m_stackUndoGraph.push(m_hCurBMP);

		m_hCurBMP = m_stackRedoGraph.top();

		m_hOldCurBMP = (HBITMAP)SelectObject(m_hMemDC,m_hCurBMP);
		BitBltEx(m_hMemCurScrnDC,SCREEN_RC,m_hMemDC,ZERO_PT,SRCCOPY|CAPTUREBLT);
		SelectObject(m_hMemDC,m_hOldCurBMP);

		m_stackRedoGraph.pop();

		//此处要改变光标，还要刷新屏幕
		m_emAction = IsWindowVisible(m_pColorWnd->GetSafeHwnd())?m_emAction:ACTION_REDO;
	}
}

BOOL CScrnCapWnd::Save()
{
	BOOL bSuccess = FALSE;

	CString strDefName;
	SetDefFileName(strDefName);

	int nFilterIndex = 1;  //过滤器索引
	TCHAR* szDefPath = new TCHAR[MAX_PATH];
	TCHAR* szDefTitle = new TCHAR[MAX_PATH];

	ZeroMemory(szDefPath,MAX_PATH);
	ZeroMemory(szDefTitle,MAX_PATH);

	StrCpy(szDefPath,strDefName);

	//打开保存对话框
	if(GetSaveFilePath(m_hInstance,GetSafeHwnd(),szDefPath,szDefTitle,nFilterIndex))
	{
		HBITMAP hSaveBMP = CreateCompatibleBitmap(m_hMemCurScrnDC,abs(m_rcSel.GetW()),abs(m_rcSel.GetH()));
		HBITMAP hOldSaveBMP = (HBITMAP)SelectObject(m_hMemDC,hSaveBMP);
		RectX rcTempSel(0,0,abs(m_rcSel.GetW()),abs(m_rcSel.GetH())); //from zero point
		BitBltEx(m_hMemDC,rcTempSel,m_hMemCurScrnDC,m_rcSel.GetStartPoint(),SRCCOPY|CAPTUREBLT);
		::SelectObject(m_hMemDC,hOldSaveBMP);

		CString strFileName = szDefPath;
		strFileName += FILTERNAME_ARR[nFilterIndex - 1];

		bSuccess = SaveAsPic(hSaveBMP,strFileName,ENCODERFORMART_ARR[nFilterIndex - 1]);

		DeleteObject(hSaveBMP);

	}

	delete[] szDefTitle;
	delete[] szDefPath;
	szDefTitle = NULL;
	szDefPath = NULL;

	return bSuccess;
}

void CScrnCapWnd::Finish()
{
	HBITMAP hCopyBMP = CreateCompatibleBitmap(m_hMemCurScrnDC,abs(m_rcSel.GetW()),abs(m_rcSel.GetH()));
	HBITMAP hOldCopyBMP = (HBITMAP)SelectObject(m_hMemDC,hCopyBMP);
	RectX rcTempSel(0,0,abs(m_rcSel.GetW()),abs(m_rcSel.GetH())); //from zero point
	BitBltEx(m_hMemDC,rcTempSel,m_hMemCurScrnDC,m_rcSel.GetStartPoint(),SRCCOPY|CAPTUREBLT);
	::SelectObject(m_hMemDC,hOldCopyBMP);

	if(CopyBMP2Clipboard(hCopyBMP,GetSafeHwnd()))
	{
		SendMessage(WM_CLOSE);
	}
	DeleteObject(hCopyBMP);
}

void CScrnCapWnd::ClearUndoStack()
{
	while(!m_stackUndoGraph.empty())
	{
		HBITMAP hTempBMP = m_stackUndoGraph.top();
		DeleteObject(hTempBMP);
		m_stackUndoGraph.pop();
	}
}

void CScrnCapWnd::ClearRedoStack()
{
	while(!m_stackRedoGraph.empty())
	{
		HBITMAP hTempBMP = m_stackRedoGraph.top();
		DeleteObject(hTempBMP);
		m_stackRedoGraph.pop();
	}
}

LRESULT CScrnCapWnd::OnCreate(WPARAM wParam, LPARAM lParam)
{
	GetAllWndRect();

	InitializeDC();

	return 0;
}

LRESULT CScrnCapWnd::OnPaint(WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	m_hClientDC = BeginPaint (GetSafeHwnd(), &ps) ;

	if(ACTION_ADJUST >= m_emAction)
	{
		PaintSelRgn();
	}
	else
	{
		BitBltEx(m_hMemPaintDC,m_rcSel,m_hMemCurScrnDC,m_rcSel.GetStartPoint(),SRCCOPY|CAPTUREBLT);
	}
	//画尺寸前保存最纯净当前屏幕
	BitBltEx(m_hMemCurScrnDC,SCREEN_RC,m_hMemPaintDC,ZERO_PT,SRCCOPY|CAPTUREBLT);

	if(!m_rcSel.IsInvalid())
	{
		//将已选区域框起
		if(ACTION_CHOOSING == m_emAction && !m_bLBtnDown)
		{
			DrawRect(m_hMemPaintDC,m_rcSel,5,PS_SOLID);
		}
		else
		{
			DrawRect(m_hMemPaintDC, m_rcSel,1,PS_DASH);
			DrawAdjustSquare(m_hMemPaintDC,m_rcSel,2);
		}

		//显示已选区域尺寸W×H
		DrawSizeTx(m_hMemPaintDC, m_rcSel);
	}

	//显示
	BitBltEx(m_hClientDC,SCREEN_RC,m_hMemPaintDC,ZERO_PT,SRCCOPY);

	EndPaint(GetSafeHwnd(), &ps) ;
	
	//SetFocus(GetSafeHwnd());

	return 0;
}

LRESULT CScrnCapWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	m_ptStart.x = LOWORD(lParam);
	m_ptStart.y = HIWORD(lParam);
	m_ptMoving = m_ptStart;
		
	switch(m_emAction)
	{
	default:
		break;
	case ACTION_ADJUST:
		if(STRETCH_NO != m_emStrech)
		{
			//拉伸处理
			m_rcChoosing = m_rcSel;    //！！拉伸前务必转换成标准矩形
			m_bStretching =  TRUE;
		}
		break;
	case ACTION_RECT:
    case ACTION_ELLIPSE:
	case ACTION_ARROW:
	case ACTION_SCRAWL:
	case ACTION_HIGHLIGHT:
		{
			//绘图操作
			m_hGraphBMP =  CreateCompatibleBitmap(m_hMemCurScrnDC,SCREEN_X,SCREEN_Y);
			m_hOldGraphBMP = (HBITMAP)SelectObject(m_hMemDC,m_hGraphBMP);
			BitBltEx(m_hMemDC,SCREEN_RC,m_hMemCurScrnDC,ZERO_PT,SRCCOPY|CAPTUREBLT);	

			if(NULL != m_pGraph)
			{
				delete m_pGraph;
				m_pGraph = NULL;
			}
			m_pGraph = GraphFactory::CreateGraph(m_emAction);

			break;
		}
		
	}

	m_bLBtnDown = TRUE;

	return 0;
}

LRESULT CScrnCapWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	//设置光标，拉伸过程中，光标保持不变
	if(ACTION_ADJUST != m_emAction || !m_bStretching)
	{
		SetScrnCursor(GetSafeHwnd(),m_rcSel,m_bLBtnDown,m_emAction,m_emStrech);
	}

	POINT ptParam = {LOWORD(lParam), HIWORD(lParam)};

	//窗口识别
	if(ACTION_CHOOSING == m_emAction && !m_bLBtnDown)
	{
		if(AutoRecognizeWnd(ptParam))
		{
			m_rcSel = m_rcChoosing;
			m_rcSel.ResetStartEnd();
		}
		else
		{
			m_rcSel = m_rcChoosing = ZERO_RC;
		}

		InvalidateRect(GetSafeHwnd(),NULL,false);
	}

	if(m_bLBtnDown)
	{
		switch(m_emAction)
		{
		default:
			break;
		case ACTION_CHOOSING:
			m_rcChoosing = RectX(m_ptStart,ptParam);
			m_rcSel = m_rcChoosing;
			m_rcSel.ResetStartEnd();
			InvalidateRect(GetSafeHwnd(),NULL,false);

			break;
		case ACTION_ADJUST:
			{
				//计算鼠标偏移位置
				int xOffset = ptParam.x - m_ptMoving.x;
				int yOffset = ptParam.y - m_ptMoving.y;

				//调整
				if(Adjust(xOffset,yOffset))
				{
					m_ptMoving = ptParam;

					//AdjustToolPos();会有闪屏
					
					ShowWindow(m_pToolWnd->GetSafeHwnd(),SW_HIDE);
					InvalidateRgn(GetSafeHwnd(),NULL,false);
				}
				
				break;
			}
		case ACTION_RECT:
		case ACTION_ELLIPSE:
		case ACTION_ARROW:
			{
				BitBltEx(m_hMemCurScrnDC,SCREEN_RC,m_hMemDC,ZERO_PT,SRCCOPY|CAPTUREBLT);
				m_pGraph->DrawGraph(m_hMemCurScrnDC,m_ptStart,ptParam,m_nPenWidth,m_dwPenColor,m_rcSel);
				InvalidateRgn(GetSafeHwnd(),NULL,false);
				break;
			}
		case ACTION_SCRAWL:
		case ACTION_HIGHLIGHT:
			{
				int iPenWidth = ACTION_HIGHLIGHT != m_emAction ? m_nPenWidth:m_nPenWidth*4 + 5;
				
				m_pGraph->DrawGraph(m_hMemCurScrnDC,m_ptMoving,ptParam,iPenWidth,m_dwPenColor,m_rcSel);
				m_ptMoving = ptParam;
				InvalidateRgn(GetSafeHwnd(),NULL,false);
				break;
			}
			
		}//end of switch(m_emAction)

	}

	return 0;
}



LRESULT CScrnCapWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	POINT ptParam = {LOWORD(lParam), HIWORD(lParam)};

	if(!m_rcSel.IsInvalid())
	{
		switch(m_emAction)
		{
		default:
			break;
		case ACTION_CHOOSING:
			{
				m_emAction = ACTION_ADJUST;
				CreateToolWnd();
				AdjustToolPos();
			}
			InvalidateRgn(GetSafeHwnd(),NULL,false);
			break;
		case ACTION_ADJUST:
			if(m_rcSel.OutOfBoundary(SCREEN_RC))
			{
				m_rcSel.AdjustPos(SCREEN_RC);
			}
			AdjustToolPos();
			InvalidateRgn(GetSafeHwnd(),NULL,false);
			
			break;
		case ACTION_RECT:
		case ACTION_ELLIPSE:
		case ACTION_ARROW:
			{
				m_stackUndoGraph.push(m_hGraphBMP);     //每完成一个绘图操作，将绘图前屏幕压入“撤销”栈
				SelectObject(m_hMemDC,m_hOldGraphBMP);
				InvalidateRgn(GetSafeHwnd(),NULL,false);
				break;
			}
			
		case ACTION_HIGHLIGHT:
		case ACTION_SCRAWL:
			{
				int iPenWidth = ACTION_HIGHLIGHT != m_emAction ? m_nPenWidth:m_nPenWidth*4 + 5;
				m_pGraph->DrawGraph(m_hMemCurScrnDC,m_ptMoving,ptParam,iPenWidth,m_dwPenColor,m_rcSel);

				m_stackUndoGraph.push(m_hGraphBMP);
				SelectObject(m_hMemDC,m_hOldGraphBMP);
				InvalidateRgn(GetSafeHwnd(),NULL,false);
				break;
			}
		
		}//end of switch

	}

	m_bLBtnDown = FALSE;
	m_bStretching = FALSE;

	m_emStrech = STRETCH_NO;

	return 0;
}

LRESULT CScrnCapWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	if(VK_ESCAPE == wParam)
	{
		SendMessage(SCMSG_CANCEL,lParam,wParam);
		return 0;
	}
	else
		if(m_emAction > ACTION_CHOOSING)
		{
			switch(wParam)
			{
			default:
				break;
			case VK_RETURN:
				{
					SendMessage(SCMSG_FINISH);
					break;
				}
			case 'Z':
				{
					if((GetAsyncKeyState(VK_CONTROL)&0x8000) != 0)
					{
						SendMessage(SCMSG_UNDO);
					}

					break;
				}
			case 'Y':
				{
					if((GetAsyncKeyState(VK_CONTROL)&0x8000) != 0)
					{
						SendMessage(SCMSG_REDO);
					}
					break;
				}
			case 'S':
				{
					if((GetAsyncKeyState(VK_CONTROL)&0x8000) != 0)
					{
						SendMessage(SCMSG_SAVEIMAGE);
					}
					break;
				}
			case VK_F1:				
				SendMessage(SCMSG_RECAP);
				break;				
			case VK_F2:
				SendMessage(SCMSG_SAVEIMAGE);
				break;
			case VK_F3:
				SendMessage(SCMSG_RECTANGLE,wParam);
				::SendMessage(m_pToolWnd->GetSafeHwnd(),WM_COMMAND,MAKEWPARAM(IDM_RECT,BN_CLICKED),0L);
				break;
			case VK_F4:
				SendMessage(SCMSG_ELLIPSE,wParam);
				::SendMessage(m_pToolWnd->GetSafeHwnd(),WM_COMMAND,MAKEWPARAM(IDM_ELLIPSE,BN_CLICKED),0L);
				break;
			case VK_F5:
				SendMessage(SCMSG_ARROWLINE,wParam);
				::SendMessage(m_pToolWnd->GetSafeHwnd(),WM_COMMAND,MAKEWPARAM(IDM_ARROW,BN_CLICKED),0L);
				break;
			case VK_F6:
				SendMessage(SCMSG_SCRAWL,wParam);
				::SendMessage(m_pToolWnd->GetSafeHwnd(),WM_COMMAND,MAKEWPARAM(IDM_SCRAWL,BN_CLICKED),0L);
				break;
			case VK_F7:
				SendMessage(SCMSG_HIGHLIGHT,wParam);
				::SendMessage(m_pToolWnd->GetSafeHwnd(),WM_COMMAND,MAKEWPARAM(IDM_HIGHLIGHT,BN_CLICKED),0L);
				break;

			}

		}

	return 0;

}

LRESULT CScrnCapWnd::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	short zDelta = (short)HIWORD(wParam);
	if(ACTION_ADJUST == m_emAction)
	{
		//按下Ctrl，滑动滚轮可放大缩小，速度为2
		if((GetAsyncKeyState(VK_CONTROL)&0x8000) != 0)
		{
			if(zDelta > 0)
			{
				Enlarge(m_rcSel,2);
			}
			if(zDelta < 0)
			{
				if( (abs(m_rcSel.GetH()) <= 5) || (abs(m_rcSel.GetW()) <= 5))
				{
					return 0;
				}
				else
				{
					Enlarge(m_rcSel,-2);
				}

			}

			m_rcSel.ResetStartEnd();
			if(m_rcSel.OutOfBoundary(SCREEN_RC))
			{
				m_rcSel.AdjustPos(SCREEN_RC);
			}
			m_rcChoosing = m_rcSel;
			InvalidateRgn(GetSafeHwnd(), NULL, FALSE);

		}
		else
		{
			//按下导航键，滑动滚动可单方向伸缩（可组合），速度为1
			if((GetAsyncKeyState(VK_LEFT)&0x8000) != 0)
			{
				m_rcChoosing.leftX = zDelta > 0 ? m_rcChoosing.leftX - 1:m_rcChoosing.leftX + 1;
			}
			if((GetAsyncKeyState(VK_UP)&0x8000) != 0)
			{
				m_rcChoosing.topX = zDelta > 0 ? m_rcChoosing.topX - 1:m_rcChoosing.topX + 1;
			}
			if((GetAsyncKeyState(VK_RIGHT)&0x8000) != 0)
			{
				m_rcChoosing.rightX = zDelta > 0 ? m_rcChoosing.rightX + 1:m_rcChoosing.rightX - 1;
			}
			if((GetAsyncKeyState(VK_DOWN)&0x8000) != 0)
			{
				m_rcChoosing.bottomX = zDelta > 0 ? m_rcChoosing.bottomX + 1:m_rcChoosing.bottomX - 1;
			}
		}
		m_rcSel = m_rcChoosing;
		m_rcSel.ResetStartEnd();
		if(m_rcSel.OutOfBoundary(SCREEN_RC))
		{
			m_rcSel.AdjustPos(SCREEN_RC);
		}
		//m_rcChoosing = m_rcSel;
		InvalidateRgn(GetSafeHwnd(), NULL, FALSE);

	}

	return 0;
}

LRESULT CScrnCapWnd::OnLButtonDbClk(WPARAM wParam, LPARAM lParam)
{
	if(m_emAction > ACTION_CHOOSING)
	{
		SendMessage(SCMSG_FINISH);
	}
	return 0;
	
}

LRESULT CScrnCapWnd::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
	if(m_emAction == ACTION_CHOOSING)
	{
		SendMessage(WM_CLOSE);
	}
	else
	{
		POINT ptCurPos = {0};
		::GetCursorPos(&ptCurPos);
		ScreenToClient(GetSafeHwnd(),&ptCurPos);

		if(!m_rcSel.PtInRectX(ptCurPos))
		{
			SendMessage(SCMSG_RECAP);
		}
	}
	return 0;

}

LRESULT CScrnCapWnd::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	ClearUndoStack();
	ClearRedoStack();
	
	SelectObject(m_hMemPaintDC,m_hOldGrayBMP);
	SelectObject(m_hMemCurScrnDC,m_hOldCurScrnBMP);

	DeleteObject(m_hScrnBMP);
	DeleteObject(m_hGrayBMP);
	DeleteObject(m_hOldCurScrnBMP);

	DeleteDC(m_hMemDC);
	DeleteDC(m_hMemPaintDC);
	DeleteDC(m_hMemCurScrnDC);

	PostQuitMessage(0);

	return 0;

}

LRESULT CScrnCapWnd::OnRecap(WPARAM wParam, LPARAM lParam)
{
	ClearUndoStack();
	ClearRedoStack();

	m_emAction = ACTION_CHOOSING;
	m_rcSel = m_rcChoosing = RectX();

	if(m_pToolWnd != NULL)
	{
		delete m_pToolWnd;
		m_pToolWnd = NULL;
	}

	if(m_pColorWnd != NULL)
	{
		delete m_pColorWnd;
		m_pColorWnd = NULL;
	}

	InvalidateRgn(GetSafeHwnd(),NULL,FALSE);
	return 0;
}

LRESULT CScrnCapWnd::OnSaveImage(WPARAM wParam, LPARAM lParam)
{
	ACTION emTempAction = m_emAction;
	m_emAction = ACTION_SAVE;
	if(Save())
	{
		SendMessage(WM_CLOSE);
	}
	else
	{
		m_emAction = emTempAction;
	}

	return 0;
}

LRESULT CScrnCapWnd::OnRectangle(WPARAM wParam, LPARAM lParam)
{
	BOOL bShow = BOOL(wParam);
	m_emAction = bShow ? ACTION_RECT : m_emAction;

	CreateColorWnd(bShow);
	
	return 0;
}

LRESULT CScrnCapWnd::OnEllipse(WPARAM wParam, LPARAM lParam)
{
	BOOL bShow = BOOL(wParam);
	m_emAction = bShow ? ACTION_ELLIPSE : m_emAction;

	CreateColorWnd(bShow);
	return 0;
}

LRESULT CScrnCapWnd::OnArrowLine(WPARAM wParam, LPARAM lParam)
{
	BOOL bShow = BOOL(wParam);
	m_emAction = bShow ? ACTION_ARROW : m_emAction;

	CreateColorWnd(bShow);

	return 0;
}

LRESULT CScrnCapWnd::OnScrawl(WPARAM wParam, LPARAM lParam)
{
	BOOL bShow = BOOL(wParam);
	m_emAction = bShow ? ACTION_SCRAWL : m_emAction;

	CreateColorWnd(bShow);

	return 0;
}

LRESULT CScrnCapWnd::OnHighlight(WPARAM wParam, LPARAM lParam)
{
	BOOL bShow = BOOL(wParam);
	m_emAction = bShow ? ACTION_HIGHLIGHT : m_emAction;

	CreateColorWnd(bShow);

	return 0;
}

LRESULT CScrnCapWnd::OnUndo(WPARAM wParam, LPARAM lParam)
{
	Undo();
	InvalidateRgn(GetSafeHwnd(),NULL,FALSE);
	
	return 0;
}

LRESULT CScrnCapWnd::OnRedo(WPARAM wParam, LPARAM lParam)
{
	Redo();
	InvalidateRgn(GetSafeHwnd(),NULL,FALSE);

	return 0;
}

LRESULT CScrnCapWnd::OnCancel(WPARAM wParam, LPARAM lParam)
{
	SendMessage(WM_CLOSE,lParam,wParam);
	return 0;
}

LRESULT CScrnCapWnd::OnFinish(WPARAM wParam, LPARAM lParam)
{
	Finish();
	return 0;
}

LRESULT CScrnCapWnd::OnPenSizeChange(WPARAM wParam, LPARAM lParam)
{
	m_nPenWidth = wParam;
	return 0;
}

LRESULT CScrnCapWnd::OnPenColorChange(WPARAM wParam, LPARAM lParam)
{
	m_dwPenColor = (COLORREF)wParam;
	return 0;
}

LRESULT CScrnCapWnd::ProcessMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	default:
		break;
	case WM_CREATE:		
		OnCreate(wParam,lParam);
		break;
	case WM_PAINT:
		OnPaint(wParam,lParam);
		break;
	case WM_LBUTTONDOWN:		
		OnLButtonDown(wParam,lParam);
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam,lParam);
		break;		
	case WM_LBUTTONUP:
		OnLButtonUp(wParam,lParam);
		break;		
	case WM_KEYDOWN:
		OnKeyDown(wParam,lParam);
		break;
	case WM_MOUSEWHEEL:
		OnMouseWheel(wParam,lParam);
		break;
	case WM_LBUTTONDBLCLK:
		OnLButtonDbClk(wParam,lParam);
		break;
	case WM_RBUTTONUP:
		OnRButtonUp(wParam,lParam);
		break;
	
		//user msg process
	case SCMSG_RECAP:
		OnRecap(wParam,lParam);
		break;
	case SCMSG_SAVEIMAGE:
		OnSaveImage(wParam,lParam);
		break;
	case SCMSG_RECTANGLE:
		OnRectangle(wParam,lParam);
		break;
	case SCMSG_ELLIPSE:
		OnEllipse(wParam,lParam);
		break;
	case SCMSG_ARROWLINE:
		OnArrowLine(wParam,lParam);
		break;
	case SCMSG_SCRAWL:
		OnScrawl(wParam,lParam);
		break;
	case SCMSG_HIGHLIGHT:
		OnHighlight(wParam,lParam);
		break;
	case SCMSG_UNDO:
		OnUndo(wParam,lParam);
		break;
	case SCMSG_REDO:
		OnRedo(wParam,lParam);
		break;
	case SCMSG_CANCEL:
		OnCancel(wParam,lParam);
		break;
	case SCMSG_FINISH:
		OnFinish(wParam,lParam);
		break;
	case SCMSG_PENSIZE_CHANGE:
		OnPenSizeChange(wParam,lParam);
		break;
	case SCMSG_PENCOLOR_CHANGE:
		OnPenColorChange(wParam,lParam);
		break;

	case WM_DESTROY:
		OnDestroy(wParam,lParam);
		break;
	}

	return DefWindowProc(GetSafeHwnd(), msg, wParam, lParam);
}


void CScrnCapWnd::ConvertToGrayBitmap(HBITMAP hSourceBmp,HDC sourceDc)
{
	HBITMAP retBmp=hSourceBmp;
	BITMAPINFO bmpInfo;
	ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	GetDIBits(sourceDc,retBmp,0,0,NULL,&bmpInfo,DIB_RGB_COLORS);
	BYTE* bits=new BYTE[bmpInfo.bmiHeader.biSizeImage];
	GetBitmapBits(retBmp,bmpInfo.bmiHeader.biSizeImage,bits);
	int bytePerPixel=4;//默认32位
	if( bmpInfo.bmiHeader.biBitCount==24)
	{
		bytePerPixel=3;
	}
	for(DWORD i=0;i<bmpInfo.bmiHeader.biSizeImage;i+=bytePerPixel)
	{
		*(bits+i) = max(0,min(255,*(bits + i)) - 50);
		*(bits+i + 1) = max(0,min(255,*(bits + i + 1) - 50));
		*(bits+i +2) = max(0,min(255,*(bits + i + 2) - 50));
	}

	SetBitmapBits(retBmp,bmpInfo.bmiHeader.biSizeImage,bits);

	delete[] bits;
}

BOOL CScrnCapWnd::BitBltEx(HDC hDestDC,RectX rc,HDC hSrcDC, POINT m_ptStart, DWORD dwRop)
{
	return BitBlt(hDestDC,rc.leftX,rc.topX,rc.GetW(),rc.GetH(),hSrcDC,m_ptStart.x,m_ptStart.y,dwRop);
}

int CScrnCapWnd::DrawTextEx(HDC hDC, const CString& strTx, RectX rc, UINT uFormat)
{
	RECT rcTemp = rc.ToRECT();
	return DrawText(hDC,strTx,lstrlen(strTx),&rcTemp,uFormat);
}

BOOL CScrnCapWnd::RectangleX(HDC hDC, RectX rc)
{
	return Rectangle(hDC,rc.leftX,rc.topX,rc.rightX,rc.bottomX);
}

void CScrnCapWnd::DrawRect(HDC hDC, const RectX& rcDraw, const int& iPenWidth, const int& iPenStyle)
{
	LOGBRUSH  LogBrush;
	LogBrush.lbStyle = BS_NULL;
	HBRUSH hBrush = CreateBrushIndirect(&LogBrush);
	HBRUSH hOldbrush = (HBRUSH)::SelectObject(hDC, hBrush);

	HPEN hPen = CreatePen(iPenStyle, iPenWidth, COLOR_RCSEL);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);

	RectangleX(hDC,rcDraw);

	::SelectObject(hDC,hOldbrush);
	::SelectObject(hDC,hOldPen);
	DeleteObject(hBrush);
	DeleteObject(hPen);
}

void CScrnCapWnd::DrawAdjustSquare(HDC hDC, const RectX& rc, const int iSize)
{
	HBRUSH hBrush = CreateSolidBrush(COLOR_RCSEL);
	HBRUSH hOldbrush = (HBRUSH)::SelectObject(hDC, hBrush);

	HPEN hPen = CreatePen(PS_SOLID, 2, COLOR_RCSEL);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);

	//左上
	RectX rcSquare(rc.leftX - iSize,rc.topX - iSize,rc.leftX + iSize,rc.topX + iSize);
	RectangleX(hDC,rcSquare);
	//右上
	rcSquare = RectX(rc.rightX - iSize, rc.topX - iSize, rc.rightX + iSize ,rc.topX + iSize);
	RectangleX(hDC,rcSquare);
	//左下
	rcSquare = RectX(rc.leftX - iSize, rc.bottomX - iSize, rc.leftX + iSize,rc.bottomX + iSize);
	RectangleX(hDC,rcSquare);
	//右下
	rcSquare = RectX(rc.rightX - iSize, rc.bottomX - iSize, rc.rightX + iSize ,rc.bottomX + iSize);
	RectangleX(hDC,rcSquare);
	//左边线
	rcSquare = RectX(rc.leftX - iSize, rc.topX + rc.GetH()/2 - iSize, rc.leftX + iSize ,rc.topX + rc.GetH()/2 + iSize);
	RectangleX(hDC,rcSquare);
	//上边线
	rcSquare = RectX(rc.leftX + rc.GetW()/2 - iSize, rc.topX - iSize, rc.leftX + rc.GetW()/2 + iSize ,rc.topX + iSize);
	RectangleX(hDC,rcSquare);
	//右边线
	rcSquare = RectX(rc.rightX - iSize, rc.topX + rc.GetH()/2 - iSize, rc.rightX + iSize ,rc.topX + rc.GetH()/2 + iSize);
	RectangleX(hDC,rcSquare);
	//下边线
	rcSquare = RectX(rc.leftX + rc.GetW()/2 - iSize, rc.bottomX - iSize, rc.leftX + rc.GetW()/2 + iSize ,rc.bottomX + iSize);
	RectangleX(hDC,rcSquare);

	::SelectObject(hDC, hOldbrush);
	::SelectObject(hDC, hOldPen);

	DeleteObject(hBrush);
	DeleteObject(hPen);

}

void CScrnCapWnd::DrawSizeTx(HDC hDC, const RectX& m_rcSel)
{
	CString strRcSize;
	strRcSize.Format(_T("%d×%d"),abs(m_rcSel.GetW()),abs(m_rcSel.GetH()));
	
	HFONT hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("隶书"));
	
	SIZE sizeString;
	GetTextExtentPoint32(hDC,strRcSize,strRcSize.GetLength(),&sizeString);

	RectX rcSizeTx = RectX(m_rcSel.leftX,m_rcSel.topX - sizeString.cy,m_rcSel.leftX + sizeString.cx - 11,m_rcSel.topX);
	rcSizeTx.AdjustPos(SCREEN_RC);

	LOGBRUSH logBrush;
	logBrush.lbStyle = BS_SOLID ;
	logBrush.lbColor = RGB(121,141,157);
	HBRUSH hBrush = ::CreateBrushIndirect(&logBrush);

	HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(121,141,157));

	HFONT hOldFont = (HFONT)SelectObject(hDC,hFont);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC,hBrush);
	HPEN holdPen = (HPEN)SelectObject(hDC,hPen);

	int hOldROP = SetROP2(hDC,R2_MASKPEN);
	SetTextColor(hDC,RGB(249,255,250));
	SetBkMode(hDC,TRANSPARENT);

	RectangleX(hDC,rcSizeTx);
	DrawTextEx(hDC,strRcSize,rcSizeTx,DT_CENTER);

	SetROP2(hDC,hOldROP);

	SelectObject(hDC,hOldFont);
	SelectObject(hDC,hOldBrush);
	SelectObject(hDC,holdPen);

	DeleteObject(hFont);
	DeleteObject(hBrush);
	DeleteObject(hPen);
}

BOOL CScrnCapWnd::InsertWndRect(HWND hwnd)
{
	if(!IsWindowVisible(hwnd) || IsIconic(hwnd))
		return FALSE;

	RECT rcWnd;
	::GetWindowRect(hwnd,&rcWnd);

	RectX rcWndX(rcWnd);
	if(!rcWndX.IsInvalid())
	{
		if(rcWndX.OutOfBoundary(SCREEN_RC))
		{
			rcWndX.ResetBoundary(SCREEN_RC);
		}
		if(rcWndX != SCREEN_RC)
		{
			m_vecAllWndRect.push_back(rcWndX);
			return TRUE;
		}

	}

	return FALSE;
}

BOOL CScrnCapWnd::EnumGrandChildWindowsProc(HWND hwnd,LPARAM lParam)
{
	InsertWndRect(hwnd);
	return TRUE;
}

BOOL CScrnCapWnd::EnumChildWindowsProc(HWND hwnd,LPARAM lParam)
{
	::EnumChildWindows(hwnd,&EnumGrandChildWindowsProc,NULL);
	return TRUE;
}

BOOL CScrnCapWnd::EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
	::EnumChildWindows(hwnd,&EnumChildWindowsProc,NULL);
	InsertWndRect(hwnd);
	return TRUE;
}

std::vector<RectX> CScrnCapWnd::GetAllWndRect()
{
	m_vecAllWndRect.clear();
	::EnumWindows(&EnumWindowsProc,NULL);

	//将桌面尺寸插入到最后
	m_vecAllWndRect.push_back(SCREEN_RC);
	return m_vecAllWndRect;
}

BOOL CScrnCapWnd::GetSaveFilePath(HINSTANCE hInstance,HWND hWnd, TCHAR* szFilePath, TCHAR* szFileTitle,int& nFilterIndex)
{
	OPENFILENAME ofn;                           
	ZeroMemory(&ofn,sizeof(OPENFILENAME));    

	ofn.lStructSize = sizeof(OPENFILENAME);    
	ofn.hwndOwner = hWnd;  
	ofn.hInstance = hInstance;  
	ofn.lpstrFile = szFilePath;    
	ofn.nMaxFile  = MAX_PATH;    
	ofn.lpstrFilter =  TEXT("BMP (*.bmp)\0*.bmp\0JPEG (*.jpg)\0*.jpg\0PNG (*.png)\0*.png\0\0") ;    
	ofn.nFilterIndex = 1;    
	ofn.lpstrFileTitle = szFileTitle;    
	ofn.nMaxFileTitle = MAX_PATH;    
	ofn.lpstrInitialDir = NULL;  
	ofn.lpstrTitle = _T("截图保存");
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	if(GetSaveFileName(&ofn))
	{
		nFilterIndex = ofn.nFilterIndex;
		return TRUE;
	}

	return FALSE;                
	
}  

void CScrnCapWnd::GetCurTimeString(CString& strCurDateTime)
{
	SYSTEMTIME systime;     
	GetLocalTime(&systime);     
	strCurDateTime.Format(_T("%4d%02d%02d%02d%02d%02d"),     
		                  systime.wYear,systime.wMonth, systime.wDay,     
		                  systime.wHour, systime.wMinute, systime.wSecond);
}

void CScrnCapWnd::SetDefFileName(CString& strDefName)
{
	strDefName = _T("截图");
	CString strTime;
	GetCurTimeString(strTime);
	strDefName.Append(strTime);
}

void CScrnCapWnd::SetScrnCursor(HWND hWnd, const RectX& rcCursorLie, const BOOL& bLButtonDown, const ACTION& m_emAction, STRETCH& emStretch)
{
	if(NULL != m_pToolWnd)
	{
		if(m_pToolWnd->CurPosInWindow())
		{
			return;
		}
	}

	POINT ptCurPos = {0};
	::GetCursorPos(&ptCurPos);
	ScreenToClient(hWnd,&ptCurPos);

	if(rcCursorLie.PtInRectX(ptCurPos))
	{
		switch(m_emAction)
		{
		default:
			break;
		case ACTION_ADJUST:
			m_hCursor = ::LoadCursor(NULL,IDC_SIZEALL);
			break;
		case ACTION_RECT:
		case ACTION_ELLIPSE:
		case ACTION_ARROW:
		case ACTION_SCRAWL:
		case ACTION_HIGHLIGHT:
			m_hCursor = ::LoadCursor(NULL,IDC_CROSS);
			break;
		}

	}
	else
	{
		m_hCursor = bLButtonDown ? m_hCursor:LoadCursor(GetInstance(),MAKEINTRESOURCE(IDC_CURSOR_COLORFUL));  //若是左键一直处于按下状态，光标不变
	}

	if((ACTION_SAVE == m_emAction)||(ACTION_REDO == m_emAction))
	{
		m_hCursor = ::LoadCursor(NULL,IDC_ARROW);
	}

	if(m_hCursor == NULL)
	{
		m_hCursor = LoadCursor(GetInstance(),MAKEINTRESOURCE(IDC_CURSOR_COLORFUL));
	}
	if(ACTION_ADJUST == m_emAction)
	{
		SetStretchCursor(rcCursorLie,ptCurPos,emStretch);
	}
	
	::SetCursor(m_hCursor);

}

void CScrnCapWnd::SetStretchCursor(const RectX& rc,const POINT& ptPos, STRETCH& emStretch)
{
	emStretch = GetStrechDrct(rc,ptPos);

	switch(emStretch)
	{
	default:
		break;
	case STRETCH_L:
	case STRETCH_R:
		m_hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
		break;
	case STRETCH_U:
	case STRETCH_D:
		m_hCursor = ::LoadCursor(NULL, IDC_SIZENS);;
		break;
	case STRETCH_LU:
	case STRETCH_RD:
		m_hCursor = ::LoadCursor(NULL, IDC_SIZENWSE);
		break;
	case STRETCH_RU:
	case STRETCH_LD:
		m_hCursor = ::LoadCursor(NULL, IDC_SIZENESW);;
		break;

	}

	return ;
		
}

STRETCH CScrnCapWnd::GetStrechDrct(const RectX& rc,const POINT& ptPos)
{
	STRETCH emTempStretch = STRETCH_NO;

	RectX rcTemp = rc;
	rcTemp.ResetStartEnd();

	if((ptPos.x == rcTemp.leftX) && (ptPos.y == rcTemp.topX))
	{
		emTempStretch = STRETCH_LU;
	}
	if((ptPos.x == rcTemp.leftX) && (ptPos.y == rcTemp.bottomX))
	{
		emTempStretch = STRETCH_LD;
	}
	if((ptPos.x == rcTemp.rightX) && (ptPos.y == rcTemp.topX))
	{
		emTempStretch = STRETCH_RU;
	}
	if((ptPos.x == rcTemp.rightX) && (ptPos.y == rcTemp.bottomX))
	{
		emTempStretch = STRETCH_RD;
	}

	if((ptPos.y > rcTemp.topX) && (ptPos.y < rcTemp.bottomX))
	{
		if(ptPos.x == rcTemp.leftX)
		{
			emTempStretch = STRETCH_L;
		}
		if(ptPos.x == rcTemp.rightX)
		{
			emTempStretch = STRETCH_R;
		}
	}

	if((ptPos.x > rcTemp.leftX) && (ptPos.x < rcTemp.rightX))
	{
		if(ptPos.y == rcTemp.topX)
		{
			emTempStretch = STRETCH_U;
		}
		if(ptPos.y == rcTemp.bottomX)
		{
			emTempStretch = STRETCH_D;
		}
	}

	//判断当前位置是否在8个实心小方块中
	RectX rcSquare = RectX(rcTemp.leftX - 2,rcTemp.topX - 2,rcTemp.leftX + 2,rcTemp.topX + 2);
	if(rcSquare.PtInRectX(ptPos))   emTempStretch = STRETCH_LU;
	rcSquare = RectX(rcTemp.rightX - 2, rcTemp.topX - 2, rcTemp.rightX + 2 ,rcTemp.topX + 2);
	if(rcSquare.PtInRectX(ptPos))   emTempStretch = STRETCH_RU;
	rcSquare = RectX(rcTemp.leftX - 2, rcTemp.bottomX - 2, rcTemp.leftX + 2,rcTemp.bottomX + 2);
	if(rcSquare.PtInRectX(ptPos))   emTempStretch = STRETCH_LD;
	rcSquare = RectX(rcTemp.rightX - 2, rcTemp.bottomX - 2, rcTemp.rightX + 2 ,rcTemp.bottomX + 2);
	if(rcSquare.PtInRectX(ptPos))   emTempStretch = STRETCH_RD;
	rcSquare = RectX(rcTemp.leftX - 2, rcTemp.topX + rcTemp.GetH()/2 - 2, rcTemp.leftX + 2 ,rcTemp.topX + rcTemp.GetH()/2 + 2);
	if(rcSquare.PtInRectX(ptPos))   emTempStretch = STRETCH_L;
	rcSquare = RectX(rcTemp.leftX + rcTemp.GetW()/2 - 2, rcTemp.topX - 2, rcTemp.leftX + rcTemp.GetW()/2 + 2 ,rcTemp.topX + 2);
	if(rcSquare.PtInRectX(ptPos))   emTempStretch = STRETCH_U;
	rcSquare = RectX(rcTemp.rightX - 2, rcTemp.topX + rcTemp.GetH()/2 - 2, rcTemp.rightX + 2 ,rcTemp.topX + rcTemp.GetH()/2 + 2);
	if(rcSquare.PtInRectX(ptPos))   emTempStretch = STRETCH_R;
	rcSquare = RectX(rcTemp.leftX + rcTemp.GetW()/2 - 2, rcTemp.bottomX - 2, rcTemp.leftX + rcTemp.GetW()/2 + 2 ,rcTemp.bottomX + 2);
	if(rcSquare.PtInRectX(ptPos))   emTempStretch = STRETCH_D;
	
	return emTempStretch;

}

BOOL CScrnCapWnd::CopyBMP2Clipboard(HBITMAP hBMP,HWND hWnd)
{
	if((NULL == hWnd) || (NULL == hBMP))
		return FALSE;

	if(OpenClipboard(hWnd))
	{
		EmptyClipboard();
		SetClipboardData(CF_BITMAP,hBMP);
		CloseClipboard();

		return TRUE;
	}
	
	return FALSE;
}

BOOL CScrnCapWnd::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)     
{     
	UINT num = 0;                     // number of image encoders     
	UINT size = 0;                   // size of the image encoder array in bytes     
	ImageCodecInfo* pImageCodecInfo = NULL;     
	GetImageEncodersSize(&num, &size);     
	if(size == 0)     
		return FALSE;     //   Failure     

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));     
	if(pImageCodecInfo == NULL)     
		return FALSE;     //   Failure     

	GetImageEncoders(num, size, pImageCodecInfo);     
	for(UINT j = 0; j < num; ++j)     
	{     
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )     
		{     
			*pClsid = pImageCodecInfo[j].Clsid;     
			free(pImageCodecInfo);     
			return TRUE;     //   Success     
		}             
	}     
	free(pImageCodecInfo);     
	return FALSE;     //   Failure     
}  

BOOL CScrnCapWnd::SaveAsPic(HBITMAP hImage, const CString& pFileName, const CString& strFormat) 
{  
	BOOL bRes = FALSE;

	ULONG_PTR tokenGDIPlus;
	Gdiplus::GdiplusStartupInput StartupInput;  
	GdiplusStartup(&tokenGDIPlus,&StartupInput,NULL);
	{
		CLSID picClsid;   
		CScrnCapWnd::GetEncoderClsid(strFormat, &picClsid);  
		Bitmap *pBitmap = Bitmap::FromHBITMAP(hImage,NULL);
		bRes = Gdiplus::Ok == pBitmap->Save(pFileName,&picClsid);
	}
	Gdiplus::GdiplusShutdown(tokenGDIPlus);

	return bRes;
}  
