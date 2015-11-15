// XSprite.h: XSprite クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XSPRITE_H__E1DBE8F5_EF52_40D1_A749_9275BC178F6D__INCLUDED_)
#define AFX_XSPRITE_H__E1DBE8F5_EF52_40D1_A749_9275BC178F6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <wingdi.h>

#define XSPRITE_FRAME_MAX 10

typedef struct tag_XSpriteFrameSize
{
	int width;
	int height;
} XSpriteFrameSize;

template <typename T>
class XSHARED_HANDLE
{
public:
	XSHARED_HANDLE(T handle)
		: m_count(0), m_handle(handle)
	{}

	~XSHARED_HANDLE()
	{if(m_handle) ::DeleteObject(m_handle);}

	void IncRef() {++m_count;}
	void DecRef() {--m_count;}
	bool IsNoRef() {return m_count? false : true;}
	T& GetHandle() {return m_handle;}
	T& operator *() const {return m_handle;}

private:
	T m_handle;
	int m_count;
};

typedef XSHARED_HANDLE<HBITMAP> XHBITMAP, *PXHBITMAP;

class XSprite  
{
public:
	enum 
	{
		XSPRITE_ERASE_PREVIOUS = 0, 
		XSPRITE_ERASE_CURRENT, 
	};

	XSprite(HWND hParent);
	virtual ~XSprite();

	// Transparent: white (0x1)
	// Opaque     : black (0x0)
	int CreateMask(HDC hDest, HDC hSrc, COLORREF crBackGround = 0x00FFFFFF, 
		int range = 0, int index = 0);
	
	void AddFrame(PXHBITMAP hFrame, PXHBITMAP hMask, int weight, int height);

	void Next(void);
	int  GetIndex(void);
	void SetIndex(int index);
	void ResetIndex(void);
	void SetFrame(PXHBITMAP hFrame, PXHBITMAP hMask, int index = 0);
	void SetSize(int wight, int height, int index = 0);
	int  Width(void);
	int  Height(void);	

	void Draw(HDC hDest, HDC hSrc, HBITMAP hCanvas, int x, int y, int width, int height);
	void Draw(HDC hDest, HDC hSrc, HBITMAP hCanvas);
	void Erase(HDC hDest, HDC hSrc, HBITMAP hCanvas, int flag = XSPRITE_ERASE_CURRENT);	

protected:
	HWND m_hParentWnd;
	int m_nIndex;
	PXHBITMAP m_hFrames[XSPRITE_FRAME_MAX];
	PXHBITMAP m_hMasks[XSPRITE_FRAME_MAX];
	HBITMAP m_hBackup;
	XSpriteFrameSize m_scSizes[XSPRITE_FRAME_MAX];
	XSpriteFrameSize m_scValidSizes[XSPRITE_FRAME_MAX];

public:
	POINT Previous;
	POINT Current;
	int   Alpha;
	int   Layer;
	bool  UpdateFlag;
	bool  Visible;
	int   Count;
	UINT  ID;
};

#endif // !defined(AFX_XSPRITE_H__E1DBE8F5_EF52_40D1_A749_9275BC178F6D__INCLUDED_)
