#include "stdafx.h"
#include "ColorWnd.h"

const int TB_PENSIZE_BTN_CNT = 4;
const int TB_PENSIZE_SEP_CNT = 1;

const int TB_COLOR_BTN_CNT = 12;
const int TB_COLOR_SEP_CNT = 0;

CColorWnd::CColorWnd()
{
	m_pPenSizeTool = NULL;
	m_pPenSizeToolBtn = NULL;
	m_pPenSizeTBNormalImage = NULL;

	m_pColorTool = NULL;
	m_pColorToolBtn = NULL;
	m_pColorTBNormalImage = NULL;

	m_hStaticBrush = NULL;
	m_dwSSBrushColor = RGB(255,0,0);
}

CColorWnd::~CColorWnd()
{
	if(NULL != m_pPenSizeTool)
	{
		delete m_pPenSizeTool;
		m_pPenSizeTool = NULL;
	}
	if(NULL != m_pColorTool)
	{
		delete m_pColorTool;
		m_pColorTool = NULL;
	}

	if(NULL != m_pPenSizeToolBtn)
	{
		delete[] m_pPenSizeToolBtn;
		m_pPenSizeToolBtn = NULL;
	}
	if(NULL != m_pColorToolBtn)
	{
		delete[] m_pColorToolBtn;
		m_pColorToolBtn = NULL;
	}

	if(NULL != m_pPenSizeTBNormalImage)
	{
		for(int iDelBMP = 0;iDelBMP < TB_PENSIZE_BTN_CNT - TB_PENSIZE_SEP_CNT; iDelBMP++)
		{
			DeleteObject(m_pPenSizeTBNormalImage[iDelBMP]);
		}

		delete[] m_pPenSizeTBNormalImage;
		m_pPenSizeTBNormalImage = NULL;
	}

	if(NULL != m_pColorTBNormalImage)
	{
		for(int iDelBMP = 0;iDelBMP < TB_PENSIZE_BTN_CNT - TB_PENSIZE_SEP_CNT; iDelBMP++)
		{
			DeleteObject(m_pColorTBNormalImage[iDelBMP]);
		}

		delete[] m_pColorTBNormalImage;
		m_pColorTBNormalImage = NULL;
	}

}

LPCTSTR CColorWnd::GetWindowClassName() const
{
	return _T("CColorWnd");
}

void CColorWnd::Initialize()
{
	InitCommonControls();

	CreatePenSizeTool();
	CreateCurColorSS();
	CreateColorTool();

	SetWndBkColor(RGB(222,238,254));
}

void CColorWnd::InitPenSizeToolBtn()
{
	m_pPenSizeToolBtn = new TBBUTTON[TB_PENSIZE_BTN_CNT];
	for(int iCnt = 0, iCmd = 0 ; iCnt < TB_PENSIZE_BTN_CNT; iCnt++)
	{
		if(iCnt == 3)
		{
			TBBUTTON tbBtnTemp = {0,0,TBSTATE_ENABLED,TBSTYLE_SEP};
			m_pPenSizeToolBtn[iCnt] = tbBtnTemp;
		}
		else
		{
			TBBUTTON tbBtnTemp = {iCmd,iCmd,TBSTATE_ENABLED,TBSTYLE_GROUP|BTNS_CHECKGROUP};
			m_pPenSizeToolBtn[iCnt] = tbBtnTemp;
			iCmd++;
		}

	}

	::SendMessage(m_pPenSizeTool->GetSafeHwnd(),TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON),0L);
	::SendMessage(m_pPenSizeTool->GetSafeHwnd(),TB_ADDBUTTONS,(WPARAM)TB_PENSIZE_BTN_CNT,(LPARAM)m_pPenSizeToolBtn);

}

void CColorWnd::InitPenSizeToolImage()
{
	m_pPenSizeTBNormalImage = new HBITMAP[TB_PENSIZE_BTN_CNT - TB_PENSIZE_SEP_CNT];

	for(int iImage = 0; iImage < TB_PENSIZE_BTN_CNT - TB_PENSIZE_SEP_CNT; iImage++)
	{
		m_pPenSizeTBNormalImage[iImage] = LoadBitmap(GetInstance(),MAKEINTRESOURCE(IDB_PENSIZE1 + iImage));
	}

}

void CColorWnd::CreatePenSizeTool()
{
	InitPenSizeToolBtn();
	InitPenSizeToolImage();

	RectX rcPenSizeTool(0,0,100,32);
	m_pPenSizeTool = new CToolBarWnd(m_pPenSizeToolBtn,TB_PENSIZE_BTN_CNT,TB_PENSIZE_SEP_CNT,24,24);
	m_pPenSizeTool->Create(GetSafeHwnd(),_T("pen_size_tool"),WS_CHILD|WS_VISIBLE|CCS_NORESIZE|TBSTYLE_TOOLTIPS|TBSTYLE_FLAT,
		WS_EX_TOPMOST|WS_EX_TOOLWINDOW,rcPenSizeTool);

	m_pPenSizeTool->SetTbButtonImage(m_pPenSizeTBNormalImage);

}

void CColorWnd::CreateCurColorSS()
{
	m_hStatic = CreateWindowEx(0,_T("static"),_T(""),WS_CHILD|WS_VISIBLE|SS_NOTIFY,
		105,5,25,25,GetSafeHwnd(),NULL,GetInstance(),NULL);

}

void CColorWnd::InitColorToolBtn()
{
	m_pColorToolBtn = new TBBUTTON[TB_COLOR_BTN_CNT];
	for(int iCnt = 0; iCnt < TB_COLOR_BTN_CNT; iCnt++)
	{
		TBBUTTON tbBtnTemp = {iCnt,iCnt,TBSTATE_ENABLED,TBSTYLE_GROUP|BTNS_CHECKGROUP};
		m_pColorToolBtn[iCnt] = tbBtnTemp;
	}

	::SendMessage(m_pColorTool->GetSafeHwnd(),TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON),0L);
	::SendMessage(m_pColorTool->GetSafeHwnd(),TB_ADDBUTTONS, (WPARAM)TB_COLOR_BTN_CNT, (LPARAM)m_pColorToolBtn);
}

void CColorWnd::InitColorToolImage()
{
	m_pColorTBNormalImage = new HBITMAP[TB_COLOR_BTN_CNT - TB_COLOR_SEP_CNT];

	for(int iImage = 0; iImage < TB_COLOR_BTN_CNT - TB_COLOR_SEP_CNT; iImage++)
	{
		m_pColorTBNormalImage[iImage] = LoadBitmap(GetInstance(),MAKEINTRESOURCE(IDB_COLOR1 + iImage));
	}
}

void CColorWnd::CreateColorTool()
{
	InitColorToolBtn();
	InitColorToolImage();

	RectX rcColorTool(135,0,COLORWND_RC.GetW() + 50,COLORWND_RC.GetH()); //+50?ÎªÃ«£¿
	m_pColorTool = new CToolBarWnd(m_pColorToolBtn,TB_COLOR_BTN_CNT,TB_COLOR_SEP_CNT,10,10);
	m_pColorTool->Create(GetSafeHwnd(),_T("colortool"),WS_CHILD|WS_VISIBLE|CCS_NORESIZE|TBSTYLE_WRAPABLE|TBSTYLE_TOOLTIPS|TBSTYLE_TRANSPARENT|TBSTYLE_FLAT,
		WS_EX_TOPMOST|WS_EX_TOOLWINDOW,rcColorTool);

	m_pColorTool->SetTbButtonImage(m_pColorTBNormalImage);

}

BOOL CColorWnd::SetWndBkColor(COLORREF dwColor)
{
	m_hBrushBk = CreateSolidBrush(dwColor);
	SetClassLong(GetSafeHwnd(),GCL_HBRBACKGROUND,(LONG)m_hBrushBk);

	return TRUE;
}

LRESULT CColorWnd::ProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	default:
		break;
	case WM_CTLCOLORSTATIC:
		OnCtlColorStatic(wParam,lParam);
		return (LRESULT)m_hStaticBrush;
	case WM_COMMAND:
		OnCommand(wParam,lParam);
		break;
	case WM_KEYDOWN:
		OnKeyDown(wParam,lParam);
		break;
	}

	return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

LRESULT CColorWnd::OnCtlColorStatic(WPARAM wParam, LPARAM lParam)
{
	if(NULL != m_hStaticBrush)
	{
		DeleteObject(m_hStaticBrush);
	}
	
	m_hStaticBrush = CreateSolidBrush(m_dwSSBrushColor);
	
	return 0;
}

LRESULT CColorWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	HWND hTempWnd = (HWND)lParam;

	if((HWND)lParam == m_pPenSizeTool->GetSafeHwnd())
	{
		switch(HIWORD(wParam))
		{
		default:
			break;
		case BN_CLICKED:
			switch(LOWORD(wParam))
			{
			case 0:
				::SendMessage(GetParent(m_hWnd),SCMSG_PENSIZE_CHANGE,1L,0L);
				break;
			case 1:
				::SendMessage(GetParent(m_hWnd),SCMSG_PENSIZE_CHANGE,2L,0L);
				break;
			case 2:
				::SendMessage(GetParent(m_hWnd),SCMSG_PENSIZE_CHANGE,5L,0L);
				break;
			}
			
		}
	}
	else
		if((HWND)lParam == m_pColorTool->GetSafeHwnd())
		{
			switch(HIWORD(wParam))
			{
			default:
				break;
			case BN_CLICKED:
				{
					m_dwSSBrushColor = PEN_COLOR[LOWORD(wParam)];
					InvalidateRect(m_hStatic,NULL,FALSE);
				}

				::SendMessage(GetParent(m_hWnd),SCMSG_PENCOLOR_CHANGE,PEN_COLOR[LOWORD(wParam)],0L);
				break;
			
			}
		}

	return 0;
}

LRESULT CColorWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	default:
		break;
	case VK_ESCAPE:
		::SendMessage(GetParent(GetSafeHwnd()),WM_CLOSE,0L,0L);
		break;
	case VK_RETURN:
		::SendMessage(GetParent(GetSafeHwnd()),SCMSG_FINISH,0L,0L);
		break;
	}

	return 0;
}