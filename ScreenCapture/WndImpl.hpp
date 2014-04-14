#ifndef  WNDIMPL_HPP
#define  WNDIMPL_HPP

class CWndImpl
{
public:
	CWndImpl();
	virtual ~CWndImpl();

public:
	BOOL MyRegisterClass(void);
	virtual HWND  Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle,RectX rc);

	virtual void Initialize(void);

	HWND GetSafeHwnd() const;
	virtual LPCTSTR  GetWindowClassName() const;

	static HINSTANCE GetInstance(void);
	static void      SetInstance(HINSTANCE);

	static  LRESULT CALLBACK WndMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT ProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual LRESULT FinalProc();

	static void MsgLoop(void);

	//API rewriting
	LRESULT SendMessage(UINT uMsg, WPARAM wParam = 0L, LPARAM lParam = 0L);
	LRESULT PostMessage(UINT uMsg, WPARAM wParam = 0L, LPARAM lParam = 0L);

	void SetIcon(HICON hIcon_Big,HICON hIcon_Small);

	SIZE GetWindowSize(void);

	BOOL CurPosInWindow(void);

public:
	HICON  m_hIcon_Big;
	HICON  m_hIcon_Small;

protected:
	BOOL  m_bFocused;
	BOOL  m_bDelInDestructor;

protected:
	HWND    m_hWnd;
	static  HINSTANCE m_hInstance;
		
};



#endif