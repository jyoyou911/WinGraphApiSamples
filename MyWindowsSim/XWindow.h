// XWindow.h: XWindow クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <wingdi.h>
#include "MyDrawingUtils.h"

using namespace MyDrawingUtils;

#define XWINDOW_BORDER_TRPRATE(alpha) ((alpha) * 3 / 4)

static const int XWINDOW_TITLE_HEIGHT = 24;
static const int XWINDOW_TITLE_BTN_DIS = 2;
static const int XWINDOW_TITLE_BTN_SIZE = XWINDOW_TITLE_HEIGHT - XWINDOW_TITLE_BTN_DIS * 2;
static const int XWINDOW_BORDER_WIDTH = 5;
static const int XWINDOW_BORDER_CORNER_R = 20;

static const int XWINDOW_MIN_WIDTH = 40;
static const int XWINDOW_MIN_HEIGHT = 40;
static const int XWINDOW_MAX_WIDTH = 1280;
static const int XWINDOW_MAX_HEIGHT = 1024;

typedef enum 
{
	XWINDOW_REGION_NONE = 0, 
	XWINDOW_REGION_TOP, 
	XWINDOW_REGION_BOTTUM, 
	XWINDOW_REGION_LEFT, 
	XWINDOW_REGION_RIGHT, 
	XWINDOW_REGION_TL_CORNER, 
	XWINDOW_REGION_TR_CORNER, 
	XWINDOW_REGION_BL_CORNER, 
	XWINDOW_REGION_BR_CORNER, 
	XWINDOW_REGION_TITLE, 
	XWINDOW_REGION_TITLE_CLOSE, 
	XWINDOW_REGION_BODY, 
} XWindowRegion;

enum
{
	XWINDOW_ACT_NOTHING = 0,	// Nothing
	XWINDOW_ACT_CLOSE,			// Close button is pushed
	XWINDOW_ACT_MOVE,			// Moving
	XWINDOW_ACT_RESIZE,			// Resizing
	XWINDOW_ACT_RESIZE_READY,	// Ready to resize
};

enum
{
	XWINDOW_RESIZE_MODE_N = 0, 
	XWINDOW_RESIZE_MODE_S, 
	XWINDOW_RESIZE_MODE_W, 
	XWINDOW_RESIZE_MODE_E, 
	XWINDOW_RESIZE_MODE_NW, 
	XWINDOW_RESIZE_MODE_SE, 
	XWINDOW_RESIZE_MODE_NE, 
	XWINDOW_RESIZE_MODE_SW, 
};


class XWindow
{
public:
	enum 
	{
		XWINDOW_ERASE_PREVIOUS = 0, 
		XWINDOW_ERASE_CURRENT, 
	};

	enum 
	{
		XWINDOW_PART_TITLE = 0, 
		XWINDOW_PART_BOTTUM, 
		XWINDOW_PART_LBORDER, 
		XWINDOW_PART_RBORDER, 
		XWINDOW_PART_BODY, 
		XWINDOW_PART_MAX, 
	};

	enum 
	{
		XWINDOW_CR_DEFAULT = 0x00D8E9EC,	// Light gray
		//XWINDOW_CR_ACTIVE_BORDER = 0x00825500,	// Blue
		XWINDOW_CR_ACTIVE_BORDER = 0x00484848,	// Black
		//XWINDOW_CR_INACTIVE_BORDER = 0x00E0967A,// White blue
		XWINDOW_CR_INACTIVE_BORDER = XWINDOW_CR_ACTIVE_BORDER | 0x00808080, // Light
		XWINDOW_CR_BODY = 0x00C0C0C0,		// Gray
		XWINDOW_CR_CLOSE_BTN = 0x004040F0,		// Red
	};

public:
	XWindow(HWND hParent, HDC hDestDC, HDC hSrcDC, HBITMAP hCanvas, 
		int width = XWINDOW_MIN_WIDTH, 
		int height = XWINDOW_MIN_HEIGHT, 
		int x = 0, int y = 0, 
		int maxX = XWINDOW_MIN_WIDTH, int maxY = XWINDOW_MIN_HEIGHT);
	virtual ~XWindow();

public:
	void Draw(int x, int y, int width, int height);
	void Draw(void);
	void Erase(int flag = XWINDOW_ERASE_CURRENT);	
	void Move(int x_offset, int y_offset);
	void Resize(int mode, int x_offset, int y_offset);
	XWindowRegion HitTest(const POINT& point);

public:
	POINT Previous;
	POINT Current;
	int   Width;
	int   Height;
	int   Alpha;
	int   Layer;
	bool  UpdateFlag;
	bool  Visible;
	int   Index;
	UINT  ID;

private:
	void SetRects(void);
	void DrawBorders(COLORREF color, bool drawMask = true);

private:
	HWND m_hParentWnd;
	RECT m_scParts[XWINDOW_PART_MAX];
	HBITMAP m_hMain;
	HBITMAP m_hMask;
	HBITMAP m_hBackup;
	HBITMAP m_hBlend;
	HBITMAP m_hCanvas;
	HDC m_hDC;
	bool m_bOK;
	int m_nMaxWidth;
	int m_nMaxHeight;
	MyDrawingUtil* m_util;
};

