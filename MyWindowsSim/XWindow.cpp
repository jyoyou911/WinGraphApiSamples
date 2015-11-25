// XWindow.cpp: XWindow
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <math.h>
#include <exception>
#include "XWindow.h"

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

XWindow::XWindow(HWND hParent, HDC hDestDC, HDC hSrcDC, HBITMAP hCanvas, 
				 int width, int height, int x, int y, int maxX, int maxY)
		: m_hParentWnd(hParent), m_hMain(NULL), m_hMask(NULL), m_hBackup(NULL), m_hBlend(NULL), 
		  m_hCanvas(hCanvas), m_hDC(NULL), m_nMaxWidth(maxX), m_nMaxHeight(maxY), 
		  m_bOK(false), 
		  Width(width), Height(height), Alpha(0xFF), Layer(1), UpdateFlag(false), Index(0), 
		  Visible(false), ID(0), m_util(NULL)
{
	if (Width < XWINDOW_MIN_WIDTH)
		Width = XWINDOW_MIN_WIDTH;
	if (Height < XWINDOW_MIN_HEIGHT)
		Height = XWINDOW_MIN_HEIGHT;
	Previous.x = x;
	Previous.y = y;
	Current = Previous;
	memset(m_scParts, 0x00, sizeof(m_scParts));

	if (NULL == m_hParentWnd || NULL == m_hCanvas)
		throw std::exception("Input parameters are NULL.");

	HDC hdc = ::GetDC(m_hParentWnd);
	// Create memory device context
	m_hDC = ::CreateCompatibleDC(hdc);

	m_hMain = ::CreateCompatibleBitmap(hdc, maxX, maxY);
	// Create monochrome bitmap
	m_hMask = ::CreateBitmap(maxX, maxY, 1, 1, NULL);
	m_hBackup = ::CreateCompatibleBitmap(hdc, maxX, maxY);
	m_hBlend = ::CreateCompatibleBitmap(hdc, maxX, maxY);
	m_util = new MyDrawingUtil(hdc);
	::ReleaseDC(m_hParentWnd, hdc);

	SetRects();
	DrawBorders(XWINDOW_CR_ACTIVE_BORDER);

	m_bOK = true;
}

XWindow::~XWindow()
{		
	memset(m_scParts, 0x00, sizeof(m_scParts));
	memset(&Current, 0x00, sizeof(Current));
	memset(&Previous, 0x00, sizeof(Previous));

	Width = 0;
	Height = 0;
	ID = 0;
	Visible = false;
	UpdateFlag = false;
	Index = 0;
	Layer = 1;
	Alpha = 0;

	if (m_hBlend)
	{
		::DeleteObject(m_hBlend);
		m_hBlend = NULL;
	}

	if (m_hBackup)
	{
		::DeleteObject(m_hBackup);
		m_hBackup = NULL;
	}

	if (m_hMask)
	{
		::DeleteObject(m_hMask);
		m_hMask = NULL;
	}

	if (m_hMain)
	{
		::DeleteObject(m_hMain);
		m_hMain = NULL;
	}

	if (m_hDC)
	{
		::DeleteDC(m_hDC);
		m_hDC = NULL;
	}

	m_hParentWnd = NULL;

	m_bOK = false;
}
 
void XWindow::Draw(int x, int y, int width, int height)
{
	HBITMAP hDestOld = NULL;
	HBITMAP hSrcOld = NULL;
	HBITMAP hTmp = NULL;
	int x_offset = (x - Current.x);
	int y_offset = (y - Current.y);	

	// Completely transparent will draw nothing
	if (0 == Alpha)
		return;

	// 0. Backup the part of canvas
	m_util->CopyLayer(m_hBackup, x_offset, y_offset, width, height, m_hCanvas, x, y);

	RECT window = {x, y, x + Width, y + Height};
	RECT interRect;
	BOOL isInter;
	for (int i = 0; i < XWINDOW_PART_MAX; ++i)
	{
		// Calculate intersect rectangle
		isInter = ::IntersectRect(&interRect, &window, &m_scParts[i]);
		if (isInter)
		{
			m_util->Mix2Layers(m_hCanvas, 
				interRect.left, 
				interRect.top, 
				interRect.right - interRect.left, 
				interRect.bottom - interRect.top, 
				m_hMain, 
				interRect.left - Current.x, 
				interRect.top - Current.y, 
				m_hMask, m_hBlend,
				XWINDOW_PART_BODY == i ? Alpha : XWINDOW_BORDER_TRPRATE(Alpha));
		}
	}

	return;
}

void XWindow::Draw()
{
	this->Draw(Current.x, Current.y, Width, Height);
}
 
void XWindow::Erase(int flag)
{
	// Restore the part of canvas
	int x = (XWINDOW_ERASE_PREVIOUS == flag) ? Previous.x : Current.x;
	int y = (XWINDOW_ERASE_PREVIOUS == flag) ? Previous.y : Current.y;
	m_util->CopyLayer(m_hCanvas, x, y, Width, Height, m_hBackup, 0, 0);

	return;
}

void XWindow::Move(int x_offset, int y_offset)
{
	Previous = Current;
	Current.x += x_offset;
	Current.y += y_offset;
	SetRects();
}

void XWindow::Resize(int mode, int x_offset, int y_offset)
{
	Previous = Current;
	switch (mode)
	{
	case XWINDOW_RESIZE_MODE_N:
		if (Height + y_offset < XWINDOW_MIN_HEIGHT)
			return;
		Height -= y_offset;
		Current.y += y_offset;
		break;
	case XWINDOW_RESIZE_MODE_S:
		if (Height + y_offset < XWINDOW_MIN_HEIGHT)
			return;
		Height += y_offset;
		break;
	case XWINDOW_RESIZE_MODE_W:
		if (Width + x_offset < XWINDOW_MIN_WIDTH)
			return;
		Width -= x_offset;
		Current.x += x_offset;
		break;
	case XWINDOW_RESIZE_MODE_E:
		if (Width + x_offset < XWINDOW_MIN_WIDTH)
			return;
		Width += x_offset;
		break;
	case XWINDOW_RESIZE_MODE_NW:
		if (Width + x_offset < XWINDOW_MIN_WIDTH
			|| Height + y_offset < XWINDOW_MIN_HEIGHT)
			return;
		Width -= x_offset;
		Height -= y_offset;
		Current.x += x_offset;
		Current.y += y_offset;
		break;
	case XWINDOW_RESIZE_MODE_SE:
		if (Width + x_offset < XWINDOW_MIN_WIDTH
			|| Height + y_offset < XWINDOW_MIN_HEIGHT)
			return;
		Width += x_offset;
		Height += y_offset;
		break;
	case XWINDOW_RESIZE_MODE_NE:
		if (Width + x_offset < XWINDOW_MIN_WIDTH
			|| Height + y_offset < XWINDOW_MIN_HEIGHT)
			return;
		Width += x_offset;
		Height -= y_offset;
		Current.y += y_offset;
		break;
	case XWINDOW_RESIZE_MODE_SW:
		if (Width + x_offset < XWINDOW_MIN_WIDTH
			|| Height + y_offset < XWINDOW_MIN_HEIGHT)
			return;
		Width -= x_offset;
		Height += y_offset;
		Current.x += x_offset;
		break;
	default:
		;
	}

	SetRects();
	DrawBorders(XWINDOW_CR_ACTIVE_BORDER);

	return;
}

XWindowRegion XWindow::HitTest(const POINT& point)
{
	XWindowRegion result = XWINDOW_REGION_NONE;
	int idx = XWINDOW_PART_TITLE;

	POINT p = {point.x - Current.x, point.y - Current.y};
	if (p.x < 0 || p.y < 0)
		return result;
	RECT window = {0, 0, Width, Height};

	for (; idx < XWINDOW_PART_MAX; ++idx)
	{
		if (::PtInRect(&m_scParts[idx], point))
			break;
	}

	RECT CloseButton = 
	{Width - XWINDOW_BORDER_CORNER_R / 2 - XWINDOW_TITLE_BTN_SIZE, 
	 XWINDOW_TITLE_BTN_DIS, 
	 Width - XWINDOW_BORDER_CORNER_R / 2,  
	 XWINDOW_TITLE_HEIGHT - XWINDOW_TITLE_BTN_DIS};
	switch (idx)
	{
	case XWINDOW_PART_TITLE:
		if (::PtInRect(&CloseButton, p))
		{
			result = XWINDOW_REGION_TITLE_CLOSE;
			break;
		}
		if (p.x < XWINDOW_BORDER_CORNER_R / 2 && p.y < XWINDOW_BORDER_CORNER_R / 2)
		{
			if (p.y > XWINDOW_BORDER_CORNER_R / 2 - p.x)
			{
				result = XWINDOW_REGION_TL_CORNER;
			}
			break;
		}
		if (p.x > Width - XWINDOW_BORDER_CORNER_R / 2 && 
			p.y < XWINDOW_BORDER_CORNER_R / 2)
		{
			if (p.y > p.x - (Width - XWINDOW_BORDER_CORNER_R / 2))
			{
				result = XWINDOW_REGION_TR_CORNER;				
			}
			break;
		}
		if (p.x < XWINDOW_BORDER_WIDTH)
		{
			result = XWINDOW_REGION_LEFT;
			break;
		}
		if (p.x > Width - XWINDOW_BORDER_WIDTH)
		{
			result = XWINDOW_REGION_RIGHT;
			break;
		}
		if (p.y < XWINDOW_BORDER_WIDTH)
		{
			result = XWINDOW_REGION_TOP;
			break;
		}
		result = XWINDOW_REGION_TITLE;
		break;
	case XWINDOW_PART_BOTTUM:
		if (p.x < XWINDOW_BORDER_WIDTH)
		{
			result = XWINDOW_REGION_BL_CORNER;
			break;
		}
		if (p.x > Width - XWINDOW_BORDER_WIDTH)
		{
			result = XWINDOW_REGION_BR_CORNER;
			break;
		}
		result = XWINDOW_REGION_BOTTUM;
		break;
	case XWINDOW_PART_LBORDER:
		result = XWINDOW_REGION_LEFT;
		break;
	case XWINDOW_PART_RBORDER:
		result = XWINDOW_REGION_RIGHT;
		break;
	case XWINDOW_PART_BODY:
		result = XWINDOW_REGION_BODY;
		break;
	default:
		;
	}

	return result;
}

void XWindow::SetRects(void)
{
	// Set title rectangle
	m_scParts[XWINDOW_PART_TITLE].left = Current.x;
	m_scParts[XWINDOW_PART_TITLE].top = Current.y;
	m_scParts[XWINDOW_PART_TITLE].right = Current.x + Width;
	m_scParts[XWINDOW_PART_TITLE].bottom = Current.y + XWINDOW_TITLE_HEIGHT;

	// Set bottum rectangle
	m_scParts[XWINDOW_PART_BOTTUM].left = Current.x;
	m_scParts[XWINDOW_PART_BOTTUM].top = Current.y + Height - XWINDOW_BORDER_WIDTH;
	m_scParts[XWINDOW_PART_BOTTUM].right = Current.x + Width;
	m_scParts[XWINDOW_PART_BOTTUM].bottom = Current.y + Height;

	// Set left border rectangle
	m_scParts[XWINDOW_PART_LBORDER].left = Current.x;
	m_scParts[XWINDOW_PART_LBORDER].top = Current.y + XWINDOW_TITLE_HEIGHT;
	m_scParts[XWINDOW_PART_LBORDER].right = Current.x + XWINDOW_BORDER_WIDTH;
	m_scParts[XWINDOW_PART_LBORDER].bottom = Current.y + Height - XWINDOW_BORDER_WIDTH;

	// Set right border rectangle
	m_scParts[XWINDOW_PART_RBORDER].left = Current.x + Width - XWINDOW_BORDER_WIDTH;
	m_scParts[XWINDOW_PART_RBORDER].top = Current.y + XWINDOW_TITLE_HEIGHT;
	m_scParts[XWINDOW_PART_RBORDER].right = Current.x + Width;
	m_scParts[XWINDOW_PART_RBORDER].bottom = Current.y + Height - XWINDOW_BORDER_WIDTH;

	// Set body rectangle
	m_scParts[XWINDOW_PART_BODY].left = Current.x + XWINDOW_BORDER_WIDTH;
	m_scParts[XWINDOW_PART_BODY].top = Current.y + XWINDOW_TITLE_HEIGHT;
	m_scParts[XWINDOW_PART_BODY].right = Current.x + Width - XWINDOW_BORDER_WIDTH;
	m_scParts[XWINDOW_PART_BODY].bottom = Current.y + Height - XWINDOW_BORDER_WIDTH;

	return;
}

void XWindow::DrawBorders(COLORREF color, bool drawMask)
{
	HBITMAP hDestOld = NULL;

	// Set color. Pen and Brush are same.
	HBRUSH hOldBrush = NULL;
	HPEN hOldPen = NULL;
	
	HBRUSH hBrush = ::CreateSolidBrush(color);
	hOldBrush = (HBRUSH) ::SelectObject(m_hDC, hBrush);
	hOldPen = (HPEN) ::SelectObject(m_hDC, ::GetStockObject(NULL_PEN));

	hDestOld = (HBITMAP)::SelectObject(m_hDC, m_hMain);
	// Reset
	::BitBlt(m_hDC, 0, 0, m_nMaxWidth, m_nMaxHeight, NULL, 0, 0, BLACKNESS);

	// Draw a round rectangle (title)
	::RoundRect(m_hDC, 0, 0,
		Width + 1, XWINDOW_TITLE_HEIGHT + XWINDOW_BORDER_CORNER_R + 1, 
		XWINDOW_BORDER_CORNER_R, XWINDOW_BORDER_CORNER_R);
	// Draw a rectangle (bottom)
	::Rectangle(m_hDC, 0, Height - XWINDOW_BORDER_WIDTH,
		Width + 1, Height + 1);
	// Draw a rectangle (left border)
	::Rectangle(m_hDC, 0, XWINDOW_TITLE_HEIGHT,
		XWINDOW_BORDER_WIDTH + 1, Height - XWINDOW_BORDER_WIDTH + 1);
	// Draw a rectangle (right border)
	::Rectangle(m_hDC, Width - XWINDOW_TITLE_HEIGHT, XWINDOW_TITLE_HEIGHT,
		Width + 1, Height - XWINDOW_BORDER_WIDTH + 1);
	// Create mask
	if (drawMask)
	{
		::SelectObject(m_hDC, hDestOld);
		m_util->CreateMask(0x00000000, m_hMain, m_hMask, Width, XWINDOW_TITLE_HEIGHT);
		hDestOld = (HBITMAP)::SelectObject(m_hDC, m_hMain);
	}
	// Draw close button
	RECT button = 
	{Width - XWINDOW_BORDER_CORNER_R / 2 - XWINDOW_TITLE_BTN_SIZE, 
	 XWINDOW_TITLE_BTN_DIS, 
	 Width - XWINDOW_BORDER_CORNER_R / 2,  
	 XWINDOW_TITLE_HEIGHT - XWINDOW_TITLE_BTN_DIS};
	::DrawFrameControl(m_hDC, &button, DFC_CAPTION, DFCS_CAPTIONCLOSE);
	// Set body color. Pen and Brush are same.
	::SelectObject(m_hDC, hOldBrush);
	::DeleteObject(hBrush);
	hBrush = ::CreateSolidBrush(XWINDOW_CR_BODY);
	hOldBrush = (HBRUSH) ::SelectObject(m_hDC, hBrush);
	// Draw a rectangle (body)
	::Rectangle(m_hDC, XWINDOW_BORDER_WIDTH, XWINDOW_TITLE_HEIGHT,
		Width - XWINDOW_BORDER_WIDTH + 1, Height - XWINDOW_BORDER_WIDTH + 1);

	::SelectObject(m_hDC, hDestOld);

	::SelectObject(m_hDC, hOldBrush);
	::SelectObject(m_hDC, hOldPen);
	::DeleteObject(hBrush);
	hBrush = NULL;

	return;
}