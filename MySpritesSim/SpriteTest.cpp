#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <math.h>
#include <tchar.h>

#include "MyDrawingUtils.h"
#include "XSprite.h"

using namespace MyDrawingUtils;

static const TCHAR szMessage[] = _T("Hello World");
static const TCHAR szFace[] = _T("Times New Roman");
static const TCHAR szHint[] = _T("Press ESC to quit");
static const TCHAR szProgram[] = _T("SpriteTest");
//static const DWORD MASKCOLOR = 0x00FC04A8; //sakura
//static const DWORD MASKCOLOR = 0x00E9ECE9; // butterfly
static const DWORD MASKCOLOR = 0x00FF00FF; // cat

static const long KW_OK = 0;
static const long KW_ERR = -1;

static const int  XSPRITE_NUM_MAX = 10;

static const int  KEY_MIN_INTERVAL = 50;
static const int  KEY_ACC_LEVEL = 3;
static const int  KEY_ACC_INTERVAL = 500;
static const int  KEY_STEP_COUNT = 5;
static const int  KEY_MOVE_PIX = 10;

static const int  FRAME_NUM = 6;
static const int  FRAME_INTERVAL = 100;

static const int  LAYER_MAX = 1023;
static const int  LAYER_STEP = 100;
static const int  ALPHA_MAIN = 0xFF;
static const int  ALPHA_LEVEL = 3;
static const int  ALPHA_STEP = ALPHA_MAIN / ALPHA_LEVEL;
static const int  FADE_INTERVAL = 200;

/*
 Timer's event ID is 4 bytes:
   0: Unused
   1: Unused
   2: Timer type (defined below)
   3: Sprite ID
 */
static const int  TIMER_ANIME = 0x00;
static const int  TIMER_FADE = 0x01;


class XSpriteWindow : public KWindow
{
public:
	XSpriteWindow();
	~XSpriteWindow();

protected:
	void OnCreate(HWND hWnd);

	void OnKeyDown(WPARAM wParam,LPARAM lParam);

	void OnTimer(UINT nIDEvent);

	void OnDraw(HDC hDC);	

	long LoadBmp(LPCTSTR lpszBmpFileName, HBITMAP& hBmpHandle, BITMAP& scBmpInfo);

	int CreateMask(HDC hDest, HDC hSrc, COLORREF crBackGround, 
			HBITMAP hSrcBmp, HBITMAP& hMask, int width, int height, int range = 0);

	bool IsRectInter(const RECT& rect1, const RECT& rect2, RECT* interRect = NULL);

	void GetOrRect(const RECT& rect1, const RECT& rect2, RECT* orRect);

	void GenFade(void);

	void Destroy(void);

protected:
	XSprite* m_sprites[XSPRITE_NUM_MAX];
	HBITMAP  m_hCanvas;
	HBITMAP  m_hBackground;
	BITMAP   m_scCanvas;
	HDC      m_hDestDC;
	HDC      m_hSrcDC;
	int	     m_nAction;
	bool     m_bVisibleFlag;
};

XSpriteWindow::XSpriteWindow()
		: m_hCanvas(NULL), m_hBackground(NULL), m_hDestDC(NULL), m_hSrcDC(NULL), 
		  m_bVisibleFlag(false), m_nAction(0)
{
	memset(m_sprites, 0x00, sizeof(XSprite*) * XSPRITE_NUM_MAX);
	memset(&m_scCanvas, 0x00, sizeof(m_scCanvas));
}

XSpriteWindow::~XSpriteWindow()
{
}

void XSpriteWindow::OnCreate(HWND hWnd)
{
	int i;
	this->m_hWnd = hWnd;

	HDC hdc = ::GetDC(hWnd);
	// Create memory device context
	m_hDestDC = ::CreateCompatibleDC(hdc);	
	m_hSrcDC = ::CreateCompatibleDC(hdc);
	// Create DDB
	m_hBackground = ::CreateCompatibleBitmap(hdc, m_scCanvas.bmWidth, m_scCanvas.bmHeight);
	::ReleaseDC(hWnd, hdc);
	
	this->LoadBmp(_T("c:\\temp\\grass.bmp"), m_hCanvas, m_scCanvas);

	HBITMAP hBmp;
	BITMAP  scBmp;
	TCHAR fileName[64] = _T("");
	memset(&scBmp, 0x00, sizeof(scBmp));	
	//this->LoadBmp(_T("c:\\temp\\butterfly.bmp"), hBmp, scBmp);
	PXHBITMAP xhBmp[FRAME_NUM];
	HBITMAP hMask;
	PXHBITMAP xhMask[FRAME_NUM];
	for (i = 0; i < FRAME_NUM; ++i)
	{
		::wsprintf(fileName, _T("c:\\temp\\cat%d.bmp"), i + 1);
		this->LoadBmp(fileName, hBmp, scBmp);
		this->CreateMask(m_hDestDC, m_hSrcDC, MASKCOLOR, hBmp, hMask, 
			scBmp.bmWidth, scBmp.bmHeight, 0);
		xhBmp[i] = new XHBITMAP(hBmp);
		xhMask[i] = new XHBITMAP(hMask);
	}

	// m_hDestDC <-> m_hBackground
	// m_hSrcDC  <-> m_hCanvas
	// ATTENTION! If you want to select the two DDBs to some DC, MUST release them first!
	::SelectObject(m_hDestDC, m_hBackground);
	::SelectObject(m_hSrcDC, m_hCanvas);
	// Duplicate original background bmp
	::BitBlt(m_hDestDC, 0, 0, m_scCanvas.bmWidth, m_scCanvas.bmHeight, 
		m_hSrcDC, 0, 0, 
		SRCCOPY);
			
	for (i = 0; i < XSPRITE_NUM_MAX; ++i)
	{
		m_sprites[i] = new XSprite(hWnd);
		for (int j = 0; j < FRAME_NUM; ++j)
			m_sprites[i]->AddFrame(xhBmp[j], xhMask[j], scBmp.bmWidth, scBmp.bmHeight);
		m_sprites[i]->Layer = LAYER_MAX - i * 100;
		m_sprites[i]->Alpha = ALPHA_MAIN;
		m_sprites[i]->Visible = false;
		m_sprites[i]->ID = i + 1;
	}	
	
	return;
}

void XSpriteWindow::OnKeyDown(WPARAM wParam,LPARAM lParam)
{
	static UINT nPrevKey = VK_ESCAPE;
	static DWORD dPrevTick = 0;
	static int nKeyCount = 0;
	DWORD dTick = 0;
	int nAccLv = 0;
	bool RestoreFlag = true;	
	RECT UpdateRect = {0};
	PRECT pUpdateRect = &UpdateRect;
	RECT Rect1, Rect2;

	Rect1.left = m_sprites[0]->Current.x;
	Rect1.top = m_sprites[0]->Current.y;
	Rect1.right = m_sprites[0]->Current.x + m_sprites[0]->Width();
	Rect1.bottom = m_sprites[0]->Current.y + m_sprites[0]->Height();
	m_sprites[0]->Previous = m_sprites[0]->Current;

	if (wParam == nPrevKey)
	{
		dTick = ::GetTickCount();
		if (dTick - dPrevTick < KEY_MIN_INTERVAL)
		{
			return;
		}
		if (dTick - dPrevTick > KEY_ACC_INTERVAL)
			nKeyCount = 0;
		if (nAccLv < KEY_ACC_LEVEL)
			nAccLv = (++nKeyCount / KEY_STEP_COUNT);	
	}
	else 
		nKeyCount = 0;
	dPrevTick = ::GetTickCount();
	

	switch (wParam)
	{
	case VK_ESCAPE:
		this->Destroy();
		PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		return;
	case VK_SPACE:		
		m_sprites[0]->Current.x = (m_scCanvas.bmWidth - m_sprites[0]->Width()) / 2;
		m_sprites[0]->Current.y = (m_scCanvas.bmHeight - m_sprites[0]->Height()) / 2;
		break;
	case VK_RETURN:
		pUpdateRect = &Rect1;
		if (m_sprites[0]->Visible)
		{
			// Erase sprite
			m_sprites[0]->Erase(m_hDestDC, m_hSrcDC, m_hCanvas);
			m_sprites[0]->Visible = false;
			::KillTimer(m_hWnd, m_sprites[0]->ID | (TIMER_ANIME << 8));			
		}
		else
		{
			m_sprites[0]->ResetIndex();
			// Draw sprite
			m_sprites[0]->Draw(m_hDestDC, m_hSrcDC, m_hCanvas, 
				m_sprites[0]->Current.x, m_sprites[0]->Current.y, 
				m_sprites[0]->Width(), m_sprites[0]->Height());
			m_sprites[0]->Visible = true;
			::SetTimer(m_hWnd, m_sprites[0]->ID | (TIMER_ANIME << 8), FRAME_INTERVAL, NULL);
		}
		::InvalidateRect(m_hWnd, pUpdateRect, FALSE);
		return;
	case VK_LEFT:		
		if (0 == m_sprites[0]->Current.x)
			break;
		m_sprites[0]->Current.x -= (nAccLv + 1) * KEY_MOVE_PIX;
		if (m_sprites[0]->Current.x < 0)
			m_sprites[0]->Current.x = 0;
		break;
	case VK_UP:
		if (0 == m_sprites[0]->Current.y)
			break;
		m_sprites[0]->Current.y -= (nAccLv + 1) * KEY_MOVE_PIX;
		if (m_sprites[0]->Current.y < 0)
			m_sprites[0]->Current.y = 0;
		break;
	case VK_RIGHT:
		if (m_scCanvas.bmWidth - m_sprites[0]->Width() == m_sprites[0]->Current.x)
			break;
		m_sprites[0]->Current.x += (nAccLv + 1) * KEY_MOVE_PIX;
		if (m_sprites[0]->Current.x > m_scCanvas.bmWidth - m_sprites[0]->Width())
			m_sprites[0]->Current.x = m_scCanvas.bmWidth - m_sprites[0]->Width();
		break;
	case VK_DOWN:
		if (m_scCanvas.bmHeight - m_sprites[0]->Height() == m_sprites[0]->Current.y)
			break;
		m_sprites[0]->Current.y += (nAccLv + 1) * KEY_MOVE_PIX;
		if (m_sprites[0]->Current.y > m_scCanvas.bmHeight - m_sprites[0]->Height())
			m_sprites[0]->Current.y = m_scCanvas.bmHeight - m_sprites[0]->Height();
		break;
	default:
		return;
	}
	nPrevKey = wParam;
	Rect2.left = m_sprites[0]->Current.x;
	Rect2.top = m_sprites[0]->Current.y;
	Rect2.right = m_sprites[0]->Current.x + m_sprites[0]->Width();
	Rect2.bottom = m_sprites[0]->Current.y + m_sprites[0]->Height();
	this->GetOrRect(Rect1, Rect2, pUpdateRect);

	if (!m_sprites[0]->Visible)
		return;

	if (m_sprites[0]->Previous.x == m_sprites[0]->Current.x 
		&& m_sprites[0]->Previous.y == m_sprites[0]->Current.y)
		return;

	this->GenFade();
	// Erase previous
	m_sprites[0]->Erase(m_hDestDC, m_hSrcDC, m_hCanvas, XSprite::XSPRITE_ERASE_PREVIOUS);

	// Draw fade
	m_sprites[1]->Draw(m_hDestDC, m_hSrcDC, m_hCanvas);

	// Draw current
	m_sprites[0]->Draw(m_hDestDC, m_hSrcDC, m_hCanvas);	

	::InvalidateRect(m_hWnd, pUpdateRect, FALSE);

	return;
}

void XSpriteWindow::OnTimer(UINT nIDEvent)
{
	int j;
	bool flag;
	int index = 0;
	for (int i = 0; i <XSPRITE_NUM_MAX; ++i)
		if (m_sprites[i]->ID == (nIDEvent & 0x000000FF))
		{
			index = i;
			break;
		}

	RECT Rect1;
	Rect1.left = m_sprites[index]->Current.x;
	Rect1.top = m_sprites[index]->Current.y;
	Rect1.right = m_sprites[index]->Current.x + m_sprites[index]->Width();
	Rect1.bottom = m_sprites[index]->Current.y + m_sprites[index]->Height();

	switch ((nIDEvent & 0x0000FF00) >> 8)
	{
	case TIMER_ANIME:
		if (index != 0)
			return;
		// Erase 
		m_sprites[index]->Erase(m_hDestDC, m_hSrcDC, m_hCanvas);
		m_sprites[index]->Next();
		m_sprites[index]->Draw(m_hDestDC, m_hSrcDC, m_hCanvas);
		break;
	case TIMER_FADE:
		if (0 == index)
			return;
		m_sprites[index]->Erase(m_hDestDC, m_hSrcDC, m_hCanvas);
		m_sprites[index]->Alpha -= ALPHA_STEP;
		if (m_sprites[index]->Alpha < ALPHA_STEP)
		{
			::KillTimer(m_hWnd, nIDEvent);
			m_sprites[index]->Visible = false;
			m_sprites[index]->ResetIndex();
		}
		else 
		{
			// Redraw with diff alpha
			m_sprites[index]->Draw(m_hDestDC, m_hSrcDC, m_hCanvas);
		}

		// Update all sprites on it
		RECT Rect2, Rect3;
		flag = false;
		for (j = index - 1; j > -1; --j)
		{
			Rect2.left = m_sprites[j]->Current.x;
			Rect2.top = m_sprites[j]->Current.y;
			Rect2.right = m_sprites[j]->Current.x + m_sprites[j]->Width();
			Rect2.bottom = m_sprites[j]->Current.y + m_sprites[j]->Height();
			flag = this->IsRectInter(Rect1, Rect2, &Rect3);
			if (flag)
			{
				m_sprites[j]->Draw(m_hDestDC, m_hSrcDC, m_hCanvas, 
					Rect3.left, Rect3.top, Rect3.right - Rect3.left, Rect3.bottom - Rect3.top);
			}
		}
		break;
	default:
		;
	}

	::InvalidateRect(m_hWnd, &Rect1, FALSE);
	return;
}

void XSpriteWindow::OnDraw(HDC hDC)
{
	::SetStretchBltMode(hDC, HALFTONE);
	::StretchBlt(hDC, 0, 0, m_nWidth, m_nHeight, 
		m_hSrcDC, 0, 0, m_scCanvas.bmWidth, m_scCanvas.bmHeight, SRCCOPY);
	return;
}

long XSpriteWindow::LoadBmp(LPCTSTR lpszBmpFileName, HBITMAP &hBmpHandle, BITMAP &scBmpInfo)
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

// Transparent: white (0x1)
// Opaque     : black (0x0)
int XSpriteWindow::CreateMask(HDC hDest, HDC hSrc, COLORREF crBackGround, 
			HBITMAP hSrcBmp, HBITMAP& hMask, int width, int height, int range)
{
	HBITMAP hDestOld = NULL;
	HBITMAP hSrcOld = NULL;
	COLORREF crOld = ::SetBkColor(hSrc, crBackGround);

	// Create monochrome bitmap
	hMask = CreateBitmap(width, height, 1, 1, NULL);
	// Select mask to DC
	hDestOld = (HBITMAP) ::SelectObject(hDest, hMask);
	// Select frame to DC
	hSrcOld = (HBITMAP)::SelectObject(hSrc, hSrcBmp);	
	
	// Color range
	if (range != 0)
	{
		int r_off = 0;
		int g_off = 0;
		int b_off = 0;
		COLORREF pix = 0x00000000;
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				pix = ::GetPixel(hSrc, x, y);
				r_off = ((pix & 0x00FF0000) >> 16) 
					- ((crBackGround & 0x00FF0000) >> 16);
				g_off = ((pix & 0x0000FF00) >> 8) 
					- ((crBackGround & 0x0000FF00) >> 8);
				b_off = (pix & 0x000000FF) 
					- (crBackGround & 0x000000FF);
				if (abs(r_off) + abs(g_off) + abs(b_off) < range)
					::SetPixel(hSrc, x, y, crBackGround);
			}
		}
	}
	
	// Color convert to make mask
	::BitBlt(hDest, 0, 0, width, height, hSrc, 0, 0, SRCCOPY);
	// Set background to white
	::SetBkColor(hSrc, 0x00FFFFFF);
	// Bmp | Mask
	// Make source bmp's transparent part to white
	::BitBlt(hSrc, 0, 0, width, height, hDest, 0, 0, SRCPAINT);

	if (hDestOld)
		::SelectObject(hDest, hDestOld);
	if (hSrcOld)
		::SelectObject(hSrc, hSrcOld);
	::SetBkColor(hSrc, crOld);

	return 0;
}

bool XSpriteWindow::IsRectInter(const RECT& rect1, const RECT& rect2, RECT* interRect)
{
	RECT rect;

	if (rect1.left > rect2.left)
		rect.left = rect1.left;
	else
		rect.left = rect2.left;

	if (rect1.top > rect2.top)
		rect.top = rect1.top;
	else
		rect.top = rect2.top;

	if (rect1.right < rect2.right)
		rect.right = rect1.right;
	else
		rect.right = rect2.right;

	if (rect1.bottom < rect2.bottom)
		rect.bottom = rect1.bottom;
	else
		rect.bottom = rect2.bottom;

	if (rect.right - rect.left > 0
		&& rect.bottom - rect.top >0)
	{
		if (interRect)
			*interRect = rect;
		return true;
	}

	return false;
}

void XSpriteWindow::GetOrRect(const RECT& rect1, const RECT& rect2, RECT* orRect)
{
	if (NULL == orRect)
		return;

	if (rect1.left > rect2.left)
		orRect->left = rect2.left;
	else
		orRect->left = rect1.left;

	if (rect1.top > rect2.top)
		orRect->top = rect2.top;
	else
		orRect->top = rect1.top;

	if (rect1.right < rect2.right)
		orRect->right = rect2.right;
	else
		orRect->right = rect1.right;

	if (rect1.bottom < rect2.bottom)
		orRect->bottom = rect2.bottom;
	else
		orRect->bottom = rect1.bottom;

	return;
}

void XSpriteWindow::GenFade(void)
{
	/*
	// The last one is showing
	if (m_sprites[XSPRITE_NUM_MAX - 1]->Visible)
	{
		// Erase it
		m_sprites[XSPRITE_NUM_MAX - 1]->Erase(m_hDestDC, m_hSrcDC, m_hCanvas);
		m_sprites[XSPRITE_NUM_MAX - 1]->Visible = false;
		m_sprites[XSPRITE_NUM_MAX - 1]->UpdateFlag = true;
	}
	*/
	XSprite* p = NULL;
	for (int i = XSPRITE_NUM_MAX - 1; i > 1; --i)
	{
		if (!m_sprites[i - 1]->Visible)
			continue;
		if (NULL == p)
			p = m_sprites[i];
		m_sprites[i] = m_sprites[i - 1];
		m_sprites[i]->Layer -= LAYER_STEP;
		//::SetTimer(m_hWnd, m_sprites[i]->ID, FADE_INTERVAL / 2, NULL);
	}
	if (p)
		m_sprites[1] = p;
	m_sprites[1]->Visible = true;
	m_sprites[1]->Alpha = ALPHA_MAIN - ALPHA_STEP;
	m_sprites[1]->Layer = LAYER_MAX - LAYER_STEP;
	m_sprites[1]->Current = m_sprites[0]->Previous;
	m_sprites[1]->Previous = m_sprites[1]->Current;
	m_sprites[1]->SetIndex(m_sprites[0]->GetIndex());
	::SetTimer(m_hWnd, m_sprites[1]->ID | (TIMER_FADE << 8), FADE_INTERVAL, NULL);

	return;
}

void XSpriteWindow::Destroy(void)
{
	for (int i = 0; i < XSPRITE_NUM_MAX; ++i)
	{
		if (m_sprites[i])
		{
			delete m_sprites[i];
			m_sprites[i] = NULL;
		}
	}

	if (NULL != m_hBackground)
		::DeleteObject(m_hBackground);

	if (NULL != m_hCanvas)
		::DeleteObject(m_hCanvas);

	if (NULL != m_hDestDC)
		::DeleteDC(m_hDestDC);

	if (NULL != m_hSrcDC)
		::DeleteDC(m_hSrcDC);

	return;
}

//----------------------------------------------------------------------------

/*
 * WinMain
 */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmd, int nShow)
{
	XSpriteWindow win;

	win.CreateEX(0, szProgram, szProgram, WS_POPUP, 480, 424, 
		800,//GetSystemMetrics(SM_CXSCREEN) / 2, 
		600,//GetSystemMetrics(SM_CYSCREEN) / 2, 
		NULL, NULL, hInst);

	win.ShowWindow(nShow);
	win.UpdateWindow();

	return win.MessageLoop();
}
