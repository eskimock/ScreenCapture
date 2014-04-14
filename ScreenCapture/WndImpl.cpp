#include "stdafx.h"
#include "WndImpl.hpp"

HINSTANCE CWndImpl::m_hInstance = NULL;

CWndImpl::CWndImpl()
{
	m_hWnd = NULL;

	m_hIcon_Big = NULL;
	m_hIcon_Small = NULL;

	m_bFocused = FALSE;
	m_bDelInDestructor = FALSE;
}
CWndImpl::~CWndImpl() 
{
	if(NULL != m_hIcon_Big)
	{
		DeleteObject(m_hIcon_Big);
	}
	if(NULL != m_hIcon_Small)
	{
		DeleteObject(m_hIcon_Small);
	}

	if(NULL != m_hWnd)
	{
		m_bDelInDestructor = TRUE;
		::DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
}

HWND CWndImpl::GetSafeHwnd() const
{ 
	return this == NULL ? NULL : m_hWnd; 
}

void CWndImpl::SetInstance(HINSTANCE instance)
{
	m_hInstance = instance;
}

HINSTANCE CWndImpl::GetInstance()
{
	return m_hInstance;
}

LPCTSTR  CWndImpl::GetWindowClassName() const
{
	return _T("CWndImpl");
}

BOOL  CWndImpl::MyRegisterClass()
{
	WNDCLASS     wndclass ;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS ;
	wndclass.lpfnWndProc   = CWndImpl::WndMsgProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = CWndImpl::GetInstance() ;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = NULL ;
	wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
	wndclass.lpszMenuName  = NULL ;
	wndclass.lpszClassName = GetWindowClassName() ;

	//是否已经注册
	BOOL bRes = FALSE;
	if(::GetClassInfo(GetInstance(), GetWindowClassName(), &wndclass))
	{
		bRes = TRUE;
	}
	else
	{
		bRes =RegisterClass (&wndclass);
	}

	return bRes;
}

HWND CWndImpl::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle,RectX rc)
{
	if(!MyRegisterClass())
	{
		m_hWnd = NULL;
	}
	else
	{
		m_hWnd = CreateWindowEx(dwExStyle, GetWindowClassName(),pstrName,dwStyle,rc.leftX,rc.topX,rc.GetW(),rc.GetH(),hwndParent,NULL,CWndImpl::GetInstance(),this);
	}

	Initialize();

	return m_hWnd;
}

void CWndImpl::Initialize()
{

}

LRESULT CWndImpl::WndMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWndImpl* pThis = reinterpret_cast<CWndImpl*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if(pThis != NULL)
	{
		if(uMsg==WM_NCDESTROY )
		{
			LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
			::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
			pThis->m_hWnd = NULL;

			//当窗口结束时，删除掉对象本身
			if(!pThis->m_bDelInDestructor)
			{
				delete pThis;
				pThis = NULL;
			}
			return lRes;
		}
		else
		{
			return pThis->ProcessMsg(uMsg, wParam, lParam);
		}
	} 
	else
	{
		if(uMsg == WM_NCCREATE)
		{
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			pThis = static_cast<CWndImpl*>(lpcs->lpCreateParams);
			assert(pThis != NULL);
			pThis->m_hWnd = hWnd;
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
			return pThis->ProcessMsg(uMsg, wParam, lParam);
		}

		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

LRESULT CWndImpl::ProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(GetSafeHwnd(), uMsg, wParam, lParam);
}

void CWndImpl::MsgLoop()
{
	MSG msg = { 0 };

	while(::GetMessage(&msg, NULL, 0, 0)) 
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

LRESULT CWndImpl::SendMessage(UINT uMsg, WPARAM wParam /*= 0L*/, LPARAM lParam /*= 0L*/)
{
	if(m_hWnd != NULL)
	{
		return ::SendMessage(m_hWnd, uMsg, wParam,lParam);
	}
	return -1;
}

LRESULT CWndImpl::PostMessage(UINT uMsg, WPARAM wParam /*= 0L*/, LPARAM lParam /*= 0L*/)
{
	if(m_hWnd != NULL)
	{
		return ::PostMessage(m_hWnd, uMsg, wParam,lParam);
	}
	return -1;
}

void CWndImpl::SetIcon(HICON hIcon_Big,HICON hIcon_Small)
{
	m_hIcon_Big = hIcon_Big;
	m_hIcon_Small = hIcon_Small;

	SendMessage(WM_SETICON, WPARAM(ICON_BIG),LPARAM(&m_hIcon_Big));
	SendMessage(WM_SETICON, WPARAM(ICON_SMALL),LPARAM(&m_hIcon_Small));
}

SIZE  CWndImpl::GetWindowSize()
{
	RECT rcWnd;
	::GetClientRect(m_hWnd,&rcWnd);
	SIZE sizeWnd = {rcWnd.right - rcWnd.left,rcWnd.bottom - rcWnd.top};
	return sizeWnd;
}

BOOL CWndImpl::CurPosInWindow()
{
	RECT rcWindow;
	GetWindowRect(GetSafeHwnd(),&rcWindow);
	RectX rcWindowX(rcWindow);
	POINT ptCurPos;
	::GetCursorPos(&ptCurPos);

	return rcWindowX.PtInRectX(ptCurPos);
}