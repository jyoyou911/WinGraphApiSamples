// XSprite.cpp: XSprite クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include <math.h>

#include "XSprite.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////
 
XSprite::XSprite(HWND hParent)
		: m_hParentWnd(hParent), m_hBackup(NULL), m_nIndex(0), Alpha(0), Layer(1), 
		  UpdateFlag(false), Count(0), ID(0)
{
	memset(m_hFrames, 0x00, XSPRITE_FRAME_MAX * sizeof(XHBITMAP));
	memset(m_hMasks, 0x00, XSPRITE_FRAME_MAX * sizeof(XHBITMAP));
	//memset(m_hBackups, 0x00, XSPRITE_FRAME_MAX * sizeof(HBITMAP));
	memset(m_scSizes, 0x00, XSPRITE_FRAME_MAX * sizeof(XSpriteFrameSize));
	memset(&Previous, 0x00, sizeof(Previous));
	memset(&Current, 0x00, sizeof(Current));
}

XSprite::~XSprite()
{
	int i;

	for (i = 0; i < XSPRITE_FRAME_MAX; ++i)
	{
		if (NULL != m_hMasks[i])
		{
			m_hMasks[i]->DecRef();			
			if (m_hMasks[i]->IsNoRef())
				delete m_hMasks[i];
			m_hMasks[i] = NULL;
		}

		if (NULL != m_hFrames[i])
		{
			m_hFrames[i]->DecRef();
			if (m_hFrames[i]->IsNoRef())
				::DeleteObject(m_hFrames[i]);
			m_hFrames[i] = NULL;
		}
	}
	if (m_hBackup)
	{
		::DeleteObject(m_hBackup);
		m_hBackup = NULL;
	}

	memset(m_hFrames, 0x00, XSPRITE_FRAME_MAX * sizeof(XHBITMAP));
	memset(m_hMasks, 0x00, XSPRITE_FRAME_MAX * sizeof(XHBITMAP));
	//memset(m_hBackups, 0x00, XSPRITE_FRAME_MAX * sizeof(HBITMAP));
	memset(m_scSizes, 0x00, XSPRITE_FRAME_MAX * sizeof(XSpriteFrameSize));
	memset(&Previous, 0x00, sizeof(Previous));
	memset(&Current, 0x00, sizeof(Current));

	ID = 0;
	Count = 0;
	UpdateFlag = false;
	Layer = 0;
	Alpha = 0;
	m_nIndex = 0;
	m_hParentWnd = NULL;
}

void XSprite::SetFrame(PXHBITMAP hFrame, PXHBITMAP hMask, int index)
{
	m_hFrames[index] = hFrame;
	hFrame->IncRef();
	m_hMasks[index] = hMask;
	hMask->IncRef();

	return;
}
 
void XSprite::AddFrame(PXHBITMAP hFrame, PXHBITMAP hMask, int width, int height)
{
	if (Count == XSPRITE_FRAME_MAX)
		return;
	
	m_hFrames[Count] = hFrame;
	hFrame->IncRef();
	m_scSizes[Count].width = width;
	m_scSizes[Count].height = height;
	m_hMasks[Count] = hMask;
	hMask->IncRef();	

	if (NULL == m_hBackup)
	{
		HDC hdc = ::GetDC(m_hParentWnd);
		m_hBackup = ::CreateCompatibleBitmap(hdc, width, height);
		::ReleaseDC(m_hParentWnd, hdc);
	}

	++Count;

	return;
}

void XSprite::Next(void)
{
	++m_nIndex == Count ? m_nIndex = 0 : 0;

	return;
}

int XSprite::GetIndex(void)
{
	return m_nIndex;
}

void XSprite::SetIndex(int index)
{
	m_nIndex = index;
	return;
}
 
void XSprite::ResetIndex(void)
{
	m_nIndex = 0;

	return;
}
 
void XSprite::SetSize(int wight, int height, int index)
{
	m_scSizes[index].width = wight;
	m_scSizes[index].height = height;

	return;
}

int XSprite::Width(void)
{
	return m_scSizes[m_nIndex].width;
}
 
int XSprite::Height(void)
{
	return m_scSizes[m_nIndex].height;
}
 
void XSprite::Draw(HDC hDest, HDC hSrc, HBITMAP hCanvas, int x, int y, int width, int height)
{
	HBITMAP hDestOld = NULL;
	HBITMAP hSrcOld = NULL;
	HBITMAP hTmp = NULL;
	int x_offset = abs(x - Current.x);
	int y_offset = abs(y - Current.y);	

	// Completely transparent will draw nothing
	if (0 == Alpha)
		return;

	// 0. Backup the part of canvas
	hDestOld = (HBITMAP) ::SelectObject(hDest, m_hBackup);
	hSrcOld = (HBITMAP) ::SelectObject(hSrc, hCanvas);
	::BitBlt(hDest, x_offset, y_offset, width, height, 
		hSrc, x, y, 
		SRCCOPY);

	if (Alpha != 0xFF)
	{
		HDC hdc = ::GetDC(m_hParentWnd);
		hTmp = ::CreateCompatibleBitmap(hdc, width, height);
		::ReleaseDC(m_hParentWnd, hdc);
		::SelectObject(hDest, hTmp);
		// 1. Copy the part of canvas
		::BitBlt(hDest, 0, 0, width, height, 
			hSrc, x,y,
			SRCCOPY);
		
		// 2. Canvas | Mask
		// Result in hTmp
		//	Transparent: white 
		//	Opaque     : canvas 
		::SelectObject(hSrc, m_hMasks[m_nIndex]->GetHandle());
		::BitBlt(hDest, 0, 0, width, height, 
			hSrc, x_offset, y_offset,
			SRCPAINT);
		
		// 3. Alpha blend
		// Result in hTmp
		//	Transparent: white 
		//	Opaque     : blend of frame and canvas
		BLENDFUNCTION blend = { AC_SRC_OVER, 0, Alpha, 0 };
		::SelectObject(hSrc, m_hFrames[m_nIndex]->GetHandle());
		::AlphaBlend(hDest, 0, 0, width, height, 
			hSrc, x_offset, y_offset, width, height, 
			blend);	
	}

	// Mask black/white exchange
	COLORREF crOldText = ::SetTextColor(hDest, 0x00FFFFFF);
	COLORREF crOldBk   = ::SetBkColor(hDest, 0x00000000);	
	// 4. Canvas | Mask
	// Result in hCanvas
	//	Transparent: canvas 
	//	Opaque     : white
	::SelectObject(hSrc, m_hMasks[m_nIndex]->GetHandle());
	::SelectObject(hDest, hCanvas);
	::BitBlt(hDest, x, y, width, height, 
		hSrc, x_offset, y_offset, 
		SRCPAINT);

	if (Alpha != 0xFF)
	{
		// 5. Canvas & Blend result
		// Result in hCanvas
		//	Transparent: canvas 
		//	Opaque     : blend of frame and canvas
		::SelectObject(hSrc, hTmp);
		::BitBlt(hDest, x, y, width, height, 
			hSrc, 0, 0, 
			SRCAND);
	}
	else
	{
		// 5. Canvas & Frame
		// Result in hCanvas
		//	Transparent: canvas 
		//	Opaque     : frame
		::SelectObject(hSrc, m_hFrames[m_nIndex]->GetHandle());
		::BitBlt(hDest, x, y, width, height, 
			hSrc, x_offset, y_offset, 
			SRCAND);
	}
	
	if (hTmp)
		::DeleteObject(hTmp);
	// Restore
	if (hDestOld)
		::SelectObject(hDest, hDestOld);
	if (hSrcOld)
		::SelectObject(hSrc, hSrcOld);
	::SetTextColor(hDest, crOldText);
	::SetBkColor(hDest, crOldBk);	

	return;
}

void XSprite::Draw(HDC hDest, HDC hSrc, HBITMAP hCanvas)
{
	this->Draw(hDest, hSrc, hCanvas, Current.x, Current.y, 
		m_scSizes[m_nIndex].width, m_scSizes[m_nIndex].height);
}
 
void XSprite::Erase(HDC hDest, HDC hSrc, HBITMAP hCanvas, int flag)
{
	// Restore the part of canvas
	HBITMAP hSrcOld = (HBITMAP) ::SelectObject(hSrc, m_hBackup);
	HBITMAP hDestOld = (HBITMAP) ::SelectObject(hDest, hCanvas);	
	int x = (XSPRITE_ERASE_PREVIOUS == flag) ? Previous.x : Current.x;
	int y = (XSPRITE_ERASE_PREVIOUS == flag) ? Previous.y : Current.y;
	::BitBlt(hDest, x, y, m_scSizes[m_nIndex].width, m_scSizes[m_nIndex].height, 
		hSrc, 0, 0, 
		SRCCOPY);

	// Restore
	::SelectObject(hDest, hDestOld);
	::SelectObject(hSrc, hSrcOld);

	return;
}
