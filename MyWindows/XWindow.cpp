// XWindow.cpp: XWindow
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include "XWindow.h"

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

XWindow::XWindow(HWND hParent, HDC hDestDC, HDC hSrcDC, HBITMAP hCanvas, 
				 int width, int height, int x, int y, int maxX, int maxY)
		: m_hParentWnd(hParent), m_hMain(NULL), m_hMask(NULL), m_hBackup(NULL), m_hBlend(NULL), 
		  m_hCanvas(hCanvas), m_hDestDC(hDestDC), m_hSrcDC(hSrcDC), m_nMaxWidth(maxX), m_nMaxHeight(maxY), 
		  m_bOK(false), 
		  Width(width), Height(height), Alpha(0xFF), Layer(1), UpdateFlag(false), Index(0), 
		  Visible(false), ID(0)
{
	if (Width < XWINDOW_MIN_WIDTH)
		Width = XWINDOW_MIN_WIDTH;
	if (Height < XWINDOW_MIN_HEIGHT)
		Height = XWINDOW_MIN_HEIGHT;
	Previous.x = x;
	Previous.y = y;
	Current = Previous;
	memset(m_scParts, 0x00, sizeof(m_scParts));

	if (NULL == m_hParentWnd || NULL == m_hDestDC || NULL == m_hSrcDC || NULL == m_hCanvas)
		return;

	HDC hdc = ::GetDC(m_hParentWnd);
	m_hMain = ::CreateCompatibleBitmap(hdc, maxX, maxY);
	// Create monochrome bitmap
	m_hMask = ::CreateBitmap(maxX, maxY, 1, 1, NULL);
	m_hBackup = ::CreateCompatibleBitmap(hdc, maxX, maxY);
	m_hBlend = ::CreateCompatibleBitmap(hdc, maxX, maxY);
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

	m_hParentWnd = NULL;

	m_bOK = false;
}

void XWindow::Paint(HBITMAP hDestBmp, HBITMAP hSrcBmp, HBITMAP hMaskBmp, 
		int x, int y, int width, int height, int x_offset, int y_offset, int alpha)
{
	HBITMAP hDestOld = NULL;
	HBITMAP hSrcOld = NULL;

	// Completely transparent will draw nothing
	if (0 == alpha)
		return;

	if (alpha != 0xFF)
	{
		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, m_hBlend);
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hDestBmp);	
		// 1. Copy the part of dest bmp
		::BitBlt(m_hDestDC, 0, 0, width, height, 
			m_hSrcDC, x,y,
			SRCCOPY);
		::SelectObject(m_hSrcDC, hSrcOld);
		::SelectObject(m_hDestDC, hDestOld);
		
		// 2. Dest bmp | Mask
		// Result in hTmp
		//	Transparent: white 
		//	Opaque     : dest bmp		
		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, m_hBlend);
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hMaskBmp);
		::BitBlt(m_hDestDC, 0, 0, width, height, 
			m_hSrcDC, x_offset, y_offset,
			SRCPAINT);
		::SelectObject(m_hSrcDC, hSrcOld);
		::SelectObject(m_hDestDC, hDestOld);
		
		// 3. Alpha blend
		// Result in hTmp
		//	Transparent: white 
		//	Opaque     : blend of src bmp and dest bmp
		BLENDFUNCTION blend = { AC_SRC_OVER, 0, alpha, 0 };
		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, m_hBlend);
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hSrcBmp);
		::AlphaBlend(m_hDestDC, 0, 0, width, height, 
			m_hSrcDC, x_offset, y_offset, width, height, 
			blend);	
		::SelectObject(m_hSrcDC, hSrcOld);
		::SelectObject(m_hDestDC, hDestOld);
	}

	// 4. Dest bmp | Mask
	// Result in hCanvas
	//	Transparent: dest bmp 
	//	Opaque     : white
	hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hDestBmp);
	hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hMaskBmp);	
	// Mask black/white exchange
	COLORREF crOldText = ::SetTextColor(m_hDestDC, 0x00FFFFFF);
	COLORREF crOldBk   = ::SetBkColor(m_hDestDC, 0x00000000);	
	// hMaskBmp/hDestBmp hold
	::BitBlt(m_hDestDC, x, y, width, height, 
		m_hSrcDC, x_offset, y_offset, 
		//SRCCOPY);
		SRCPAINT);
	::SetTextColor(m_hDestDC, crOldText);
	::SetBkColor(m_hDestDC, crOldBk);
	::SelectObject(m_hSrcDC, hSrcOld);
	::SelectObject(m_hDestDC, hDestOld);

	hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hDestBmp);
	if (alpha != 0xFF)
	{
		// 5. Dest & src bmps Blend result
		// Result in hDestBmp
		//	Transparent: dest bmp 
		//	Opaque     : blend of dest and src bmps		
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, m_hBlend);
		// hTmp/hDestBmp hold
		::BitBlt(m_hDestDC, x, y, width, height, 
			m_hSrcDC, 0, 0, 
			SRCAND);
		::SelectObject(m_hSrcDC, hSrcOld);
	}
	else
	{
		// 5. Dest bmp & Src bmp
		// Result in hDestBmp
		//	Transparent: canvas 
		//	Opaque     : frame
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hSrcBmp);
		// hSrcBmp/hDestBmp hold
		::BitBlt(m_hDestDC, x, y, width, height, 
			m_hSrcDC, x_offset, y_offset, 
			SRCAND);
		::SelectObject(m_hSrcDC, hSrcOld);
	}
	::SelectObject(m_hDestDC, hDestOld);

	return;
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
	hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, m_hBackup);
	hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, m_hCanvas);
	::BitBlt(m_hDestDC, x_offset, y_offset, width, height, 
		m_hSrcDC, x, y, 
		SRCCOPY);
	// Restore
	if (hDestOld)
		::SelectObject(m_hDestDC, hDestOld);
	if (hSrcOld)
		::SelectObject(m_hSrcDC, hSrcOld);

	RECT window = {x, y, x + Width, y + Height};
	RECT interRect;
	BOOL isInter;
	for (int i = 0; i < XWINDOW_PART_MAX; ++i)
	{
		// Calculate intersect rectangle
		isInter = ::IntersectRect(&interRect, &window, &m_scParts[i]);
		if (isInter)
		{
			Paint(m_hCanvas, m_hMain, m_hMask, 
				interRect.left, 
				interRect.top, 
				interRect.right - interRect.left, 
				interRect.bottom - interRect.top, 
				interRect.left - Current.x, 
				interRect.top - Current.y, 
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
	HBITMAP hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, m_hBackup);
	HBITMAP hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, m_hCanvas);	
	int x = (XWINDOW_ERASE_PREVIOUS == flag) ? Previous.x : Current.x;
	int y = (XWINDOW_ERASE_PREVIOUS == flag) ? Previous.y : Current.y;
	::BitBlt(m_hDestDC, x, y, Width, Height, 
		m_hSrcDC, 0, 0, 
		SRCCOPY);

	// Restore
	::SelectObject(m_hDestDC, hDestOld);
	::SelectObject(m_hSrcDC, hSrcOld);

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
	hOldBrush = (HBRUSH) ::SelectObject(m_hDestDC, hBrush);
	hOldPen = (HPEN) ::SelectObject(m_hDestDC, ::GetStockObject(NULL_PEN));

	hDestOld = (HBITMAP)::SelectObject(m_hDestDC, m_hMain);
	// Reset
	::BitBlt(m_hDestDC, 0, 0, m_nMaxWidth, m_nMaxHeight, NULL, 0, 0, BLACKNESS);

	// Draw a round rectangle (title)
	::RoundRect(m_hDestDC, 0, 0, 
		Width + 1, XWINDOW_TITLE_HEIGHT + XWINDOW_BORDER_CORNER_R + 1, 
		XWINDOW_BORDER_CORNER_R, XWINDOW_BORDER_CORNER_R);
	// Draw a rectangle (bottom)
	::Rectangle(m_hDestDC, 0, Height - XWINDOW_BORDER_WIDTH, 
		Width + 1, Height + 1);
	// Draw a rectangle (left border)
	::Rectangle(m_hDestDC, 0, XWINDOW_TITLE_HEIGHT, 
		XWINDOW_BORDER_WIDTH + 1, Height - XWINDOW_BORDER_WIDTH + 1);
	// Draw a rectangle (right border)
	::Rectangle(m_hDestDC, Width - XWINDOW_TITLE_HEIGHT, XWINDOW_TITLE_HEIGHT, 
		Width + 1, Height - XWINDOW_BORDER_WIDTH + 1);
	// Create mask
	if (drawMask)
	{
		::SelectObject(m_hDestDC, hDestOld);
		CreateMask(0x00000000, m_hMain, m_hMask, Width, XWINDOW_TITLE_HEIGHT);
		hDestOld = (HBITMAP)::SelectObject(m_hDestDC, m_hMain);
	}
	// Draw close button
	RECT button = 
	{Width - XWINDOW_BORDER_CORNER_R / 2 - XWINDOW_TITLE_BTN_SIZE, 
	 XWINDOW_TITLE_BTN_DIS, 
	 Width - XWINDOW_BORDER_CORNER_R / 2,  
	 XWINDOW_TITLE_HEIGHT - XWINDOW_TITLE_BTN_DIS};
	::DrawFrameControl(m_hDestDC, &button, DFC_CAPTION, DFCS_CAPTIONCLOSE);
	// Set body color. Pen and Brush are same.
	::SelectObject(m_hDestDC, hOldBrush);
	::DeleteObject(hBrush);
	hBrush = ::CreateSolidBrush(XWINDOW_CR_BODY);
	hOldBrush = (HBRUSH) ::SelectObject(m_hDestDC, hBrush);
	// Draw a rectangle (body)
	::Rectangle(m_hDestDC, XWINDOW_BORDER_WIDTH, XWINDOW_TITLE_HEIGHT, 
		Width - XWINDOW_BORDER_WIDTH + 1, Height - XWINDOW_BORDER_WIDTH + 1);

	::SelectObject(m_hDestDC, hDestOld);

	::SelectObject(m_hDestDC, hOldBrush);
	::SelectObject(m_hDestDC, hOldPen);
	::DeleteObject(hBrush);
	hBrush = NULL;

	return;
}

// Transparent: white (0x1)
// Opaque     : black (0x0)
void XWindow::CreateMask(COLORREF crBackGround, 
			HBITMAP hSrcBmp, HBITMAP hMask, int width, int height, int range)
{
	HBITMAP hDestOld = NULL;
	HBITMAP hSrcOld = NULL;
	COLORREF crOld = ::SetBkColor(m_hSrcDC, crBackGround);

	// Select mask to DC
	hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hMask);
	// Select frame to DC
	hSrcOld = (HBITMAP)::SelectObject(m_hSrcDC, hSrcBmp);	
	
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
				pix = ::GetPixel(m_hSrcDC, x, y);
				r_off = ((pix & 0x00FF0000) >> 16) 
					- ((crBackGround & 0x00FF0000) >> 16);
				g_off = ((pix & 0x0000FF00) >> 8) 
					- ((crBackGround & 0x0000FF00) >> 8);
				b_off = (pix & 0x000000FF) 
					- (crBackGround & 0x000000FF);
				if (abs(r_off) + abs(g_off) + abs(b_off) < range)
					::SetPixel(m_hSrcDC, x, y, crBackGround);
			}
		}
	}

	// Reset to black
	::BitBlt(m_hDestDC, 0, 0, m_nMaxWidth, m_nMaxHeight, NULL, 0, 0, BLACKNESS);
	// Color convert to make mask
	::BitBlt(m_hDestDC, 0, 0, width, height, m_hSrcDC, 0, 0, SRCCOPY);
	// Set background to white
	::SetBkColor(m_hSrcDC, 0x00FFFFFF);
	// Bmp | Mask
	// Make source bmp's transparent part to white
	::BitBlt(m_hSrcDC, 0, 0, width, height, m_hDestDC, 0, 0, SRCPAINT);

	if (hDestOld)
		::SelectObject(m_hDestDC, hDestOld);
	if (hSrcOld)
		::SelectObject(m_hSrcDC, hSrcOld);
	::SetBkColor(m_hSrcDC, crOld);
}