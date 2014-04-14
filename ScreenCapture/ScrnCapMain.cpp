#include "stdafx.h"
#include "ScrnCapWnd.h"


int APIENTRY _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	HANDLE hMutex = CreateMutex(NULL,TRUE,_T("ScreenCapture"));
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hMutex);
		return -1;
	}

	CWndImpl::SetInstance(hInstance);
	CScrnCapWnd* pScrnCap = new CScrnCapWnd;
	pScrnCap->Create(NULL,_T("ScrnCap"),WS_POPUP|WS_VISIBLE,WS_EX_TOPMOST|WS_EX_TOOLWINDOW,SCREEN_RC);
	UpdateWindow(pScrnCap->GetSafeHwnd());

	CWndImpl::MsgLoop();

	CloseHandle(hMutex);

	return 0;
}