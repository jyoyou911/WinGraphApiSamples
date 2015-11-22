// MyWindowsSim.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include <math.h>
#include "MyWindowsSim.h"
#include "XWindow.h"
#include "MyDrawingUtils.h"


static const TCHAR szMessage[] = _T("Hello World");
static const TCHAR szFace[] = _T("Times New Roman");
static const TCHAR szHint[] = _T("Press ESC to quit");
static const TCHAR szProgram[] = _T("KWindowTest");

static const long KW_OK = 0;
static const long KW_ERR = 1;

static const int XWINDOW_NUM_MAX = 10;

static int XWINDOW_ID = 0;

template<typename T> struct JListNode
{
	T* Element;
	JListNode* Next;
	JListNode* Prev;
};


class XWindowTest : public MyDrawingUtils::KWindow
{
public:
	XWindowTest();
	~XWindowTest();

protected:
	void OnCreate(HWND hWnd);

	void OnKeyDown(WPARAM wParam, LPARAM lParam);

	void OnMouseMove(WPARAM wParam, LPARAM lParam);

	void OnLButtonDown(WPARAM wParam, LPARAM lParam);

	void OnLButtonUp(WPARAM wParam, LPARAM lParam);

	//void OnTimer(UINT nIDEvent);

	void OnDraw(HDC hDC);

	long LoadBmp(LPCTSTR lpszBmpFileName, HBITMAP& hBmpHandle, BITMAP& scBmpInfo);

	bool IsRectInter(const RECT& rect1, const RECT& rect2, RECT* interRect = NULL);

	void GetOrRect(const RECT& rect1, const RECT& rect2, RECT* orRect);

	void Destroy(void);

protected:
	JListNode<XWindow>* m_listHead;
	//XWindow* m_windows[XWINDOW_NUM_MAX];
	XWindow* m_actWnd;
	HBITMAP  m_hCanvas;
	HBITMAP  m_hBackground;
	BITMAP   m_scCanvas;
	HDC      m_hDestDC;
	HDC      m_hSrcDC;
	HICON    m_hMousePointer;
	POINT	 m_scPreMouse;
	int	     m_nAction;
	int		 m_nIndex;
	int      m_nResizeMode;
	bool     m_bVisibleFlag;
};

XWindowTest::XWindowTest()
	: m_listHead(NULL), m_actWnd(NULL), m_hCanvas(NULL), m_hBackground(NULL), m_hDestDC(NULL), m_hSrcDC(NULL),
	m_hMousePointer(NULL), m_bVisibleFlag(false), m_nAction(XWINDOW_ACT_NOTHING), m_nIndex(-1)
{
	//memset(m_windows, 0x00, sizeof(XWindow*) * XWINDOW_NUM_MAX);
	memset(&m_scCanvas, 0x00, sizeof(m_scCanvas));
	memset(&m_scPreMouse, 0x00, sizeof(m_scPreMouse));

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}

XWindowTest::~XWindowTest()
{
}

void XWindowTest::OnCreate(HWND hWnd)
{
	this->m_hWnd = hWnd;

	HDC hdc = ::GetDC(hWnd);
	// Create memory device context
	m_hDestDC = ::CreateCompatibleDC(hdc);
	m_hSrcDC = ::CreateCompatibleDC(hdc);
	// Create DDB with the window size
	//m_hBackground = ::CreateCompatibleBitmap(hdc, m_scCanvas.bmWidth, m_scCanvas.bmHeight);
	m_hCanvas = ::CreateCompatibleBitmap(hdc, m_nWidth, m_nHeight);
	m_hBackground = ::CreateCompatibleBitmap(hdc, m_nWidth, m_nHeight);


	HBITMAP hBmpOriginHandle;
	this->LoadBmp(_T("..\\MyWindows\\bg.bmp"), hBmpOriginHandle, m_scCanvas);
	// Zoom it to the window size
	HBITMAP hOldSrc = (HBITMAP)::SelectObject(m_hSrcDC, hBmpOriginHandle);
	HBITMAP hOldDest = (HBITMAP)::SelectObject(m_hDestDC, m_hCanvas);
	::SetStretchBltMode(m_hDestDC, HALFTONE);
	::StretchBlt(m_hDestDC, 0, 0, m_nWidth, m_nHeight,
		m_hSrcDC, 0, 0, m_scCanvas.bmWidth, m_scCanvas.bmHeight, SRCCOPY);

	::SelectObject(m_hDestDC, hOldDest);
	::SelectObject(m_hSrcDC, hOldSrc);

	::ReleaseDC(hWnd, hdc);

	return;
}

void XWindowTest::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	int xPos = lParam & 0x0000FFFF;
	int yPos = (lParam & 0xFFFF0000) >> 16;
	int x_offset = xPos - m_scPreMouse.x;
	int y_offset = yPos - m_scPreMouse.y;
	m_scPreMouse.x = xPos;
	m_scPreMouse.y = yPos;
	POINT p = { xPos, yPos };
	RECT update = { 0 };
	RECT previous = { 0 };
	RECT current = { 0 };
	HICON icon = NULL;
	bool bUFlag = false;

	if (m_listHead == NULL)
		return;

	if (XWINDOW_ACT_NOTHING == m_nAction ||
		XWINDOW_ACT_RESIZE_READY == m_nAction)
	{
		JListNode<XWindow>* w = m_listHead;
		for (; w != NULL; w = w->Next)
		{
			if (NULL == w->Element)
				continue;
			XWindowRegion r = w->Element->HitTest(p);
			switch (r)
			{
			case XWINDOW_REGION_TOP:
				m_hMousePointer = LoadCursor(NULL, IDC_SIZENS);
				m_nResizeMode = XWINDOW_RESIZE_MODE_N;
				break;
			case XWINDOW_REGION_BOTTUM:
				m_hMousePointer = LoadCursor(NULL, IDC_SIZENS);
				m_nResizeMode = XWINDOW_RESIZE_MODE_S;
				break;
			case XWINDOW_REGION_LEFT:
				m_hMousePointer = ::LoadCursor(NULL, IDC_SIZEWE);
				m_nResizeMode = XWINDOW_RESIZE_MODE_W;
				break;
			case XWINDOW_REGION_RIGHT:
				m_hMousePointer = ::LoadCursor(NULL, IDC_SIZEWE);
				m_nResizeMode = XWINDOW_RESIZE_MODE_E;
				break;
			case XWINDOW_REGION_TL_CORNER:
				m_hMousePointer = ::LoadCursor(NULL, IDC_SIZENWSE);
				m_nResizeMode = XWINDOW_RESIZE_MODE_NW;
				break;
			case XWINDOW_REGION_BR_CORNER:
				m_hMousePointer = ::LoadCursor(NULL, IDC_SIZENWSE);
				m_nResizeMode = XWINDOW_RESIZE_MODE_SE;
				break;
			case XWINDOW_REGION_TR_CORNER:
				m_hMousePointer = ::LoadCursor(NULL, IDC_SIZENESW);
				m_nResizeMode = XWINDOW_RESIZE_MODE_NE;
				break;
			case XWINDOW_REGION_BL_CORNER:
				m_hMousePointer = ::LoadCursor(NULL, IDC_SIZENESW);
				m_nResizeMode = XWINDOW_RESIZE_MODE_SW;
				break;
			case XWINDOW_REGION_BODY:
			case XWINDOW_REGION_TITLE:
				goto NO_ACTION;
			default:
				continue;
			}
			::SetCursor(m_hMousePointer);
			m_nAction = XWINDOW_ACT_RESIZE_READY;
			return;
		}
	NO_ACTION:
		m_nAction = XWINDOW_ACT_NOTHING;
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		return;
	}

	if (m_actWnd == NULL)
		return;

	::SetRect(&previous,
		m_actWnd->Current.x,
		m_actWnd->Current.y,
		m_actWnd->Current.x + m_actWnd->Width,
		m_actWnd->Current.y + m_actWnd->Height);
	switch (m_nAction)
	{
	case XWINDOW_ACT_MOVE:
		if (m_actWnd->Current.x + x_offset < 0
			|| m_actWnd->Current.x + m_actWnd->Width + x_offset >= m_nWidth
			|| m_actWnd->Current.y + y_offset < 0
			|| m_actWnd->Current.y + m_actWnd->Height + y_offset >= m_nHeight)
		{
			break;
		}
		m_actWnd->Erase();
		m_actWnd->Move(x_offset, y_offset);
		m_actWnd->Draw();
		bUFlag = true;
		break;
	case XWINDOW_ACT_RESIZE:
		if (m_actWnd->Current.x + x_offset < 0
			|| m_actWnd->Current.x + m_actWnd->Width + x_offset >= m_nWidth
			|| m_actWnd->Current.y + y_offset < 0
			|| m_actWnd->Current.y + m_actWnd->Height + y_offset >= m_nHeight)
		{
			break;
		}
		m_actWnd->Erase();
		m_actWnd->Resize(m_nResizeMode, x_offset, y_offset);
		m_actWnd->Draw();
		bUFlag = true;
		break;
	default:
		;
	}

	if (bUFlag)
	{
		::SetRect(&current,
			m_actWnd->Current.x,
			m_actWnd->Current.y,
			m_actWnd->Current.x + m_actWnd->Width,
			m_actWnd->Current.y + m_actWnd->Height);
		::UnionRect(&update, &previous, &current);
		::InvalidateRect(m_hWnd, &update, FALSE);
	}

	return;
}

void XWindowTest::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	int xPos = lParam & 0x0000FFFF;
	int yPos = (lParam & 0xFFFF0000) >> 16;
	m_scPreMouse.x = xPos;
	m_scPreMouse.y = yPos;
	POINT p = { xPos, yPos };
	RECT window = { 0 };
	m_actWnd = NULL;

	// Active a window if possible
	JListNode<XWindow>* w = m_listHead;
	for (; w != NULL; w = w->Next)
	{
		if (NULL == w->Element)
			continue;
		::SetRect(&window,
			w->Element->Current.x,
			w->Element->Current.y,
			w->Element->Current.x + w->Element->Width,
			w->Element->Current.y + w->Element->Height);
		if (::PtInRect(&window, p))
		{
			m_actWnd = w->Element;
			// Move the active window node to to head of List
			// And redraw all regions intersected
			if (w != m_listHead)
			{
				// 1. Remove from current position
				JListNode<XWindow>* prev = w->Prev;
				JListNode<XWindow>* next = w->Next;
				prev->Next = next;
				if (next != NULL)
					next->Prev = prev;
				// 2. Add to the head
				w->Next = m_listHead;
				m_listHead->Prev = w;
				w->Prev = NULL;
				m_listHead = w;
				// 3. Erase the active window first
				w->Element->Erase();
				// 4. Redraw all intersected regions from bottum to top(Z order)
				JListNode<XWindow>* other = prev;
				RECT actWindow = { w->Element->Current.x,
					w->Element->Current.y,
					w->Element->Current.x + w->Element->Width,
					w->Element->Current.y + w->Element->Height };
				for (; other != NULL; other = other->Prev)
				{
					RECT window = { other->Element->Current.x,
						other->Element->Current.y,
						other->Element->Current.x + other->Element->Width,
						other->Element->Current.y + other->Element->Height };
					RECT interRect;
					BOOL isInter;
					// Calculate intersect rectangle
					isInter = ::IntersectRect(&interRect, &window, &actWindow);
					if (isInter)
					{
						other->Element->Draw(interRect.left,
							interRect.top,
							interRect.right - interRect.left,
							interRect.bottom - interRect.top);
					}
				}
				::InvalidateRect(m_hWnd, &actWindow, FALSE);
			}
			break;
		}
	}

	if (NULL == m_actWnd)
		return;

	if (XWINDOW_ACT_RESIZE_READY == m_nAction)
	{
		m_nAction = XWINDOW_ACT_RESIZE;
		return;
	}

	if (XWINDOW_ACT_NOTHING != m_nAction)
	{
		m_nAction = XWINDOW_ACT_NOTHING;
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		return;
	}

	XWindowRegion r = m_actWnd->HitTest(m_scPreMouse);
	switch (r)
	{
	case XWINDOW_REGION_TITLE:
		m_nAction = XWINDOW_ACT_MOVE;
		break;
	case XWINDOW_REGION_TITLE_CLOSE:
		m_nAction = XWINDOW_ACT_CLOSE;
		break;
	default:
		;
	}

	return;
}

void XWindowTest::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	int xPos = lParam & 0x0000FFFF;
	int yPos = (lParam & 0xFFFF0000) >> 16;
	POINT p = { xPos, yPos };
	RECT UpdateRect = { 0 };
	PRECT pUpdateRect = &UpdateRect;

	if (m_actWnd == NULL)
		return;

	switch (m_nAction)
	{
	case XWINDOW_ACT_CLOSE:
		if (XWINDOW_REGION_TITLE_CLOSE == m_actWnd->HitTest(p))
		{
			m_actWnd->Erase();

			::SetRect(pUpdateRect,
				m_actWnd->Current.x,
				m_actWnd->Current.y,
				m_actWnd->Current.x + m_actWnd->Width,
				m_actWnd->Current.y + m_actWnd->Height);
			::InvalidateRect(m_hWnd, pUpdateRect, FALSE);

			delete m_actWnd;
			m_actWnd = NULL;

			// Active window is the head node
			JListNode<XWindow>* w = m_listHead->Next;
			if (w != NULL)
				w->Prev = NULL;
			delete m_listHead;
			m_listHead = w;
		}
		break;
	case XWINDOW_ACT_MOVE:
	case XWINDOW_ACT_RESIZE:
		break;
	default:
		;
	}

	m_nAction = XWINDOW_ACT_NOTHING;
	return;
}

void XWindowTest::OnKeyDown(WPARAM wParam, LPARAM lParam)
{

	RECT UpdateRect = { 0 };
	PRECT pUpdateRect = &UpdateRect;
	bool bUpdate = false;
	XWindow* newWindow = NULL;
	JListNode<XWindow>* w = NULL;

	switch (wParam)
	{
	case VK_ESCAPE:
		this->Destroy();
		PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		return;
	case VK_SPACE:
		// Create new window
		newWindow = new XWindow(m_hWnd, m_hDestDC, m_hSrcDC, m_hCanvas,
			400, 200, 200, 150, m_nWidth, m_nHeight);
		newWindow->Alpha = 0xFF;
		newWindow->Visible = true;
		newWindow->ID = ++XWINDOW_ID;
		newWindow->Draw();
		// New window is the active window
		// Add it to the List head
		m_actWnd = newWindow;
		w = new JListNode<XWindow>();
		w->Element = newWindow;
		w->Prev = NULL;
		w->Next = m_listHead;
		if (m_listHead != NULL)
			m_listHead->Prev = w;
		m_listHead = w;

		// Calculate the update rectangle
		UpdateRect.left = newWindow->Current.x;
		UpdateRect.top = newWindow->Current.y;
		UpdateRect.right = newWindow->Current.x + newWindow->Width;
		UpdateRect.bottom = newWindow->Current.y + newWindow->Height;
		bUpdate = true;
		break;
	case VK_RETURN:
		::SetCursor(::LoadCursor(NULL, IDC_CROSS));
		break;
	case VK_F5:
		::InvalidateRect(m_hWnd, NULL, FALSE);
		break;
	default:
		;
	}

	if (bUpdate)
	{
		::InvalidateRect(m_hWnd, pUpdateRect, FALSE);
	}

	return;
}

void XWindowTest::OnDraw(HDC hDC)
{
	HBITMAP hOld = (HBITMAP)::SelectObject(m_hSrcDC, m_hCanvas);
	::BitBlt(hDC, 0, 0, m_nWidth, m_nHeight, m_hSrcDC, 0, 0, SRCCOPY);
	::SelectObject(m_hSrcDC, hOld);
	return;
}

long XWindowTest::LoadBmp(LPCTSTR lpszBmpFileName, HBITMAP &hBmpHandle, BITMAP &scBmpInfo)
{
	if (NULL == lpszBmpFileName)
		return KW_ERR;

	// Load the original bmp file to memory, and get the original size
	hBmpHandle = (HBITMAP)
		::LoadImage(NULL, lpszBmpFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (NULL == hBmpHandle)
		return KW_ERR;
	::GetObject(hBmpHandle, sizeof(scBmpInfo), &scBmpInfo);

	return KW_OK;
}

bool XWindowTest::IsRectInter(const RECT& rect1, const RECT& rect2, RECT* interRect)
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

void XWindowTest::GetOrRect(const RECT& rect1, const RECT& rect2, RECT* orRect)
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

void XWindowTest::Destroy(void)
{
	m_actWnd = NULL;

	JListNode<XWindow>* p = m_listHead;
	while (p != NULL)
	{
		JListNode<XWindow>* q = p->Next;
		if (p->Element != NULL)
			delete p->Element;
		delete p;
		p = q;
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
	XWindowTest win;

	win.CreateEX(0, szProgram, szProgram, WS_POPUP, 480, 424,
		800,//GetSystemMetrics(SM_CXSCREEN) / 2, 
		600,//GetSystemMetrics(SM_CYSCREEN) / 2, 
		NULL, NULL, hInst);

	win.ShowWindow(nShow);
	win.UpdateWindow();

	return win.MessageLoop();
}
