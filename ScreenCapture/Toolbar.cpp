#include "stdafx.h"
#include "Toolbar.h"


CToolBarWnd::CToolBarWnd(TBBUTTON* pTbBtn,int nBtnCnt,int nSepCnt,int cxImage,int cyImage)
{
	m_pTbButton = pTbBtn;

	m_nTbButtonCnt = nBtnCnt;
	m_nSepCnt = nSepCnt;

	m_cxImage = cxImage;
	m_cyImage = cyImage;
}

CToolBarWnd::~CToolBarWnd()
{
	m_pTbButton = NULL;
}

LPCTSTR CToolBarWnd::GetWindowClassName() const
{
	return TOOLBARCLASSNAME;
}

void CToolBarWnd::Initialize()
{
	InitCommonControls();
	SendMessage(TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON));
	SendMessage(TB_ADDBUTTONS, (WPARAM)m_nTbButtonCnt, (LPARAM)m_pTbButton);
}

BOOL CToolBarWnd::SetTbButtonImage(HBITMAP* phImage,UINT uMsgSetImage /* = TB_SETIMAGELIST */,COLORREF crMask /* = RGB */)
{
	if(m_nTbButtonCnt <= 0)
		return FALSE;

	HIMAGELIST hImgList = ImageList_Create(m_cxImage, m_cyImage, ILC_COLOR24|ILC_MASK, m_nTbButtonCnt - m_nSepCnt, 3);
	for(int iImage = 0; iImage < m_nTbButtonCnt - m_nSepCnt; iImage++)
	{
		ImageList_AddMasked(hImgList,phImage[iImage],crMask);
	}
	SendMessage(uMsgSetImage, 0L, (LPARAM)hImgList);

	DeleteObject(hImgList);

	return TRUE;
}
