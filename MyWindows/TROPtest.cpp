#define STRICT
#define WIN32_LEAN_AND_MEAN

#include "KWindow.h"
#include "KDIB.h"

#define ABS(x) ((x) > 0 ? (x) : (-x))

static const TCHAR szMessage[] = _T("Hello World");
static const TCHAR szFace[] = _T("Times New Roman");
static const TCHAR szHint[] = _T("Press ESC to quit");
static const TCHAR szProgram[] = _T("TROPtest");
//static const DWORD MASKCOLOR = 0x00FC04A8; //sakura
static const DWORD MASKCOLOR = 0x00FFFFFF;

static const long KW_OK = 0;
static const long KW_ERR = -1;

class KTROPWindow : public KWindow
{
public:
	KTROPWindow();

	enum 
	{
		DRAW_INIT = 0, 
		DRAW_NOTHING, 
		DRAW_FLY,
	};

protected:
	void OnCreate(HWND hWnd);

	void OnKeyDown(WPARAM wParam,LPARAM lParam);

	void OnDraw(HDC hDC);	

	long LoadBmp(LPCTSTR lpszBmpFileName, HBITMAP& hBmpHandle, BITMAP& scBmpInfo);

	void CreateMask(COLORREF crBackGround, HDC hMaskDC, HDC hBmp, int width, int height);

	void Destroy(void);

protected:
	HBITMAP m_hBmp;
	BITMAP m_scBmp;
	HBITMAP m_hFlyBmp;
	BITMAP m_scFlyBmp;
	HBITMAP m_hBackBmp;
	HBITMAP m_hTmpBmp;
	HDC m_hFlyMask;
	HDC m_hMemDC;
	HDC m_hFlyMemDC;
	KDIB m_flyDIB;
	POINT m_current;
	POINT m_previous;
	int m_nAction;
	bool m_bVisibleFlag;
};

KTROPWindow::KTROPWindow()
		: m_hBmp(NULL), m_hMemDC(NULL), m_hFlyBmp(NULL), 
		m_hBackBmp(NULL), m_hTmpBmp(NULL), 
		m_hFlyMask(NULL), m_hFlyMemDC(NULL), 
		m_bVisibleFlag(false), m_nAction(DRAW_INIT)
{
	memset(&this->m_scBmp, 0x00, sizeof(this->m_scBmp));
	memset(&this->m_scFlyBmp, 0x00, sizeof(this->m_scFlyBmp));
	m_current.x = 0;
	m_current.y = 0;
	m_previous.x = -1;
	m_previous.y = -1;
}

void KTROPWindow::OnCreate(HWND hWnd)
{
	this->LoadBmp(_T("f:\\downloads\\grass.bmp"), m_hBmp, m_scBmp);
	//this->LoadBmp(_T("c:\\temp\\butterfly.bmp"), m_hFlyBmp, m_scFlyBmp);
	m_flyDIB.LoadFile(_T("f:\\downloads\\butterfly.bmp"));

	// Change the part will be transparent to pure white
	// The original image maybe has color offset so modify it
	BYTE *p = NULL;
	BYTE *line = NULL;
	BYTE byteDepth = m_flyDIB.GetDepth() >> 3;
	int r_off = 0;
	int g_off = 0;
	int b_off = 0;
	for (int i = 0; i < m_flyDIB.GetHeight(); ++i)
	{
		line = &(m_flyDIB.GetBits()[i * m_flyDIB.GetWidth() * byteDepth]);
		for (int j = 0; j < m_flyDIB.GetWidth(); ++j)
		{
			p = line + j * byteDepth;
			r_off = *(p + 2) - ((MASKCOLOR & 0x00FF0000) >> 16);
			g_off = *(p + 1) - ((MASKCOLOR & 0x0000FF00) >> 8);
			b_off = *p - (MASKCOLOR & 0x000000FF);			
			if (ABS(r_off) + ABS(g_off) + ABS(b_off) < 50)
			{
				::memset(p , 0xFF, 3);
			}
		}
	}

	HDC hdc = ::GetDC(hWnd);
	m_hMemDC = ::CreateCompatibleDC(hdc);	
	m_hFlyMask = ::CreateCompatibleDC(hdc);

	// Memory DC is monochrome by default, so can not create bmp by it.
	m_hFlyMemDC = ::CreateCompatibleDC(hdc);
	m_hFlyBmp = ::CreateCompatibleBitmap(hdc, m_flyDIB.GetWidth(), m_flyDIB.GetHeight());
	m_hBackBmp = ::CreateCompatibleBitmap(hdc, m_flyDIB.GetWidth(), m_flyDIB.GetHeight());
	m_hTmpBmp = ::CreateCompatibleBitmap(hdc, m_flyDIB.GetWidth(), m_flyDIB.GetHeight());
	::SetDIBits(hdc, m_hFlyBmp, 0, m_flyDIB.GetHeight(), m_flyDIB.GetBits(), m_flyDIB.GetBMI(), DIB_RGB_COLORS);
	::ReleaseDC(hWnd, hdc);	
	::SelectObject(m_hMemDC, m_hBmp);

	::SelectObject(m_hFlyMemDC, m_hFlyBmp);
	this->CreateMask(0x00FFFFFF, m_hFlyMask, m_hFlyMemDC, m_flyDIB.GetWidth(), m_flyDIB.GetHeight());	

	//::SetTextColor(m_hMemDC, 0x00FFFFFF);
	//::SetBkColor(m_hMemDC, 0x00000000);		
	
	return;
}

void KTROPWindow::OnKeyDown(WPARAM wParam,LPARAM lParam)
{
	bool RestoreFlag = true;	
	RECT UpdateRect = {m_current.x, m_current.y, 
		m_current.x + m_flyDIB.GetWidth(), 
		m_current.y + m_flyDIB.GetHeight()};
	PRECT pUpdateRect = &UpdateRect;
	switch (wParam)
	{
	case VK_ESCAPE:
		this->Destroy();
		PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		return;
	case VK_SPACE:
		m_previous = m_current;
		m_current.x = (m_scBmp.bmWidth - m_flyDIB.GetWidth()) / 2;
		m_current.y = (m_scBmp.bmHeight - m_flyDIB.GetHeight()) / 2;
		pUpdateRect = NULL;
		break;
	case VK_RETURN:
		if (m_bVisibleFlag)
		{
			// Restore background
			::SelectObject(m_hFlyMemDC, m_hBackBmp);
			::BitBlt(m_hMemDC, 
				m_current.x,
				m_current.y,
				m_flyDIB.GetWidth(), 
				m_flyDIB.GetHeight(),
				m_hFlyMemDC, 
				0, 
				0, 								
				SRCCOPY);
			m_bVisibleFlag = false;
		
			::InvalidateRect(m_hWnd, &UpdateRect, FALSE);
			
			return;
		}
		else
		{
			m_bVisibleFlag = true;
			RestoreFlag = false;
		}
		break;
	case VK_LEFT:
		m_previous = m_current;
		if (0 == m_current.x)
			break;
		m_current.x -= 10;
		if (m_current.x < 0)
			m_current.x = 0;
		UpdateRect.left = m_current.x;
		break;
	case VK_UP:
		m_previous = m_current;
		if (0 == m_current.y)
			break;
		m_current.y -= 10;
		if (m_current.y < 0)
			m_current.y = 0;
		UpdateRect.top = m_current.y;
		break;
	case VK_RIGHT:
		m_previous = m_current;
		if (m_scBmp.bmWidth - m_flyDIB.GetWidth() == m_current.x)
			break;
		m_current.x += 10;
		if (m_current.x > m_scBmp.bmWidth - m_flyDIB.GetWidth())
			m_current.x = m_scBmp.bmWidth - m_flyDIB.GetWidth();
		UpdateRect.right = m_current.x + m_flyDIB.GetWidth();
		break;
	case VK_DOWN:
		m_previous = m_current;
		if (m_scBmp.bmHeight - m_flyDIB.GetHeight() == m_current.y)
			break;
		m_current.y += 10;
		if (m_current.y > m_scBmp.bmHeight - m_flyDIB.GetHeight())
			m_current.y = m_scBmp.bmHeight - m_flyDIB.GetHeight();
		UpdateRect.bottom = m_current.y + m_flyDIB.GetHeight();
		break;
	default:
		return;
	}

	if (!m_bVisibleFlag)
		return;

	if (m_previous.x == m_current.x 
		&& m_previous.y == m_current.y)
		return;

	// Restore background
	::SelectObject(m_hFlyMemDC, m_hBackBmp);
	if (RestoreFlag)
		::BitBlt(m_hMemDC, 
			m_previous.x,
			m_previous.y,
			m_flyDIB.GetWidth(), 
			m_flyDIB.GetHeight(),
			m_hFlyMemDC, 
			0, 
			0, 								
			SRCCOPY);

	// Backup background to m_hBackBmp
	::BitBlt(m_hFlyMemDC, 
		0, 
		0, 
		m_flyDIB.GetWidth(), 
		m_flyDIB.GetHeight(), 
		m_hMemDC, 
		m_current.x,
		m_current.y,
		SRCCOPY);
	
	// Copy background to a tmp buffer(m_hTmpBmp)
	::SelectObject(m_hFlyMemDC, m_hTmpBmp);
	::BitBlt(m_hFlyMemDC, 
		0, 
		0, 
		m_flyDIB.GetWidth(), 
		m_flyDIB.GetHeight(), 
		m_hMemDC, 
		m_current.x,
		m_current.y,
		SRCCOPY);
	
	//// Ready for the alpha blend (background and the sprite)
	//// Sprite's transparent part will be white
	//::BitBlt(m_hFlyMemDC, 
	//	0, 
	//	0, 
	//	m_flyDIB.GetWidth(), 
	//	m_flyDIB.GetHeight(), 
	//	m_hFlyMask, 
	//	0,
	//	0,
	//	SRCPAINT);
	
	// Alpha blend
	BLENDFUNCTION blend = { AC_SRC_OVER, 0, 204, 0 };
	::SelectObject(m_hMemDC, m_hFlyBmp);
	::AlphaBlend(m_hFlyMemDC, 0, 0, m_flyDIB.GetWidth(), m_flyDIB.GetHeight(), 
		m_hMemDC, 0, 0, m_flyDIB.GetWidth(), m_flyDIB.GetHeight(), blend);
	::SelectObject(m_hMemDC, m_hBmp);
	
	//// D | M
	//::BitBlt(m_hFlyMemDC, 
	//	0, 
	//	0, 
	//	m_flyDIB.GetWidth(), 
	//	m_flyDIB.GetHeight(), 
	//	m_hFlyMask, 
	//	0,
	//	0,
	//	SRCPAINT);	

	::BitBlt(m_hMemDC,
		m_current.x,
		m_current.y,
		m_flyDIB.GetWidth(),
		m_flyDIB.GetHeight(),
		m_hFlyMemDC,
		0,
		0,
		SRCINVERT);
	//::InvalidateRect(m_hWnd, NULL, FALSE);return;

	// reverse the colors
	//::SetTextColor(m_hMemDC, 0x00FFFFFF);
	//::SetBkColor(m_hMemDC, 0x00000000);
	// Mask
	::BitBlt(m_hMemDC, 
		m_current.x, 
		m_current.y, 
		m_flyDIB.GetWidth(), 
		m_flyDIB.GetHeight(), 
		m_hFlyMask, 
		0, 
		0, 
		//SRCCOPY);
		SRCAND);
	//::InvalidateRect(m_hWnd, pUpdateRect, FALSE);return;

	// Draw
	//::SelectObject(m_hFlyMemDC, m_hFlyBmp);
	::BitBlt(m_hMemDC, 
		m_current.x, 
		m_current.y, 
		m_flyDIB.GetWidth(), 
		m_flyDIB.GetHeight(), 
		m_hFlyMemDC, 
		0, 
		0, 
		//SRCCOPY);
		SRCINVERT);

	float wRatio = ((float)m_nWidth) / m_scBmp.bmWidth;
	float hRatio = ((float)m_nHeight) / m_scBmp.bmHeight;
	float ratio = wRatio > hRatio ? wRatio : hRatio;

	pUpdateRect->right = (int)(pUpdateRect->right * ratio) + 1;
	pUpdateRect->bottom = (int)(pUpdateRect->bottom * ratio) + 1;

	::InvalidateRect(m_hWnd, pUpdateRect, FALSE);

	return;
}

void KTROPWindow::OnDraw(HDC hDC)
{
	::SetStretchBltMode(hDC, HALFTONE);
	::StretchBlt(hDC, 0, 0, m_nWidth, m_nHeight, 
		m_hMemDC, 0, 0, m_scBmp.bmWidth, m_scBmp.bmHeight, SRCCOPY);
	return;
}


long KTROPWindow::LoadBmp(LPCTSTR lpszBmpFileName, HBITMAP &hBmpHandle, BITMAP &scBmpInfo)
{
	if (NULL == lpszBmpFileName)
		return KW_ERR;

	hBmpHandle = (HBITMAP) 
		::LoadImage(NULL, lpszBmpFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (NULL == hBmpHandle)
		return KW_ERR;

	::GetObject(hBmpHandle, sizeof(scBmpInfo), &scBmpInfo);

	return KW_OK;
}

void KTROPWindow::CreateMask(COLORREF crBackGround, HDC hMaskDC, HDC hBmp, int width, int height)
{
	HBITMAP hMask = CreateBitmap(width, height, 1, 1, NULL);
	::SelectObject(hMaskDC, hMask);

	// Every pixel in the src image will be compared to the dest's background color.
	// Mathed one will be converted to 1, and no mathed one will be converted to 0.
	::SetBkColor(hMaskDC, crBackGround);
	::BitBlt(hMaskDC, 0, 0, width, height, hBmp, 0, 0, SRCCOPY);

	return;
}

void KTROPWindow::Destroy(void)
{
	if (NULL != m_hFlyMemDC)
		::DeleteDC(m_hFlyMemDC);

	if (NULL != m_hMemDC)
		::DeleteDC(m_hMemDC);

	if (NULL != m_hFlyMask)
		::DeleteDC(m_hFlyMask);

	if (NULL != m_hTmpBmp)
		::DeleteObject(m_hTmpBmp);

	if (NULL != m_hBackBmp)
		::DeleteObject(m_hBackBmp);

	if (NULL != m_hFlyBmp)
		::DeleteObject(m_hFlyBmp);

	if (NULL != m_hBmp)
		::DeleteObject(m_hBmp);

	return;
}

//----------------------------------------------------------------------------

/*
 * WinMain
 */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmd, int nShow)
{
	KTROPWindow win;

	win.CreateEX(0, szProgram, szProgram, WS_POPUP, 100, 100, 
		800,//GetSystemMetrics(SM_CXSCREEN) / 2, 
		600,//GetSystemMetrics(SM_CYSCREEN) / 2, 
		NULL, NULL, hInst);

	win.ShowWindow(nShow);
	win.UpdateWindow();

	return win.MessageLoop();
}
