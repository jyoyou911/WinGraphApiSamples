// XSprite.h: XSprite
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <wingdi.h>
#include <vector>
#include <memory>


class XSpriteHelper
{
public:
	/// <summary>
	/// 各种Windows Handle的删除仿函数
	/// </summary>
	template<class T>
	struct HAnythingDeleter
	{
		void operator()(T* p)
		{
			if (p != nullptr && *p != NULL)
				::DeleteObject(*p);
		}
	};

	/// <summary>
	/// HDC的删除仿函数
	/// </summary>
	struct HDCDeleter
	{
		void operator()(HDC* p)
		{
			if (p != nullptr && *p != NULL)
				::DeleteDC(*p);
		}
	};

public:
	std::shared_ptr<HBITMAP>& MakeSharedHBitmap(HBITMAP* bmp)
	{
		return std::shared_ptr<HBITMAP>(bmp, m_hBitmapDeleter);
	}
	std::shared_ptr<HDC>& MakeSharedHDC(HDC* hdc)
	{
		return std::shared_ptr<HDC>(hdc, m_hDCDeleter);
	}

private:
	HAnythingDeleter<HBITMAP> m_hBitmapDeleter;
	HDCDeleter m_hDCDeleter;
};
	
extern XSpriteHelper g_spriteHelper;

class XSprite  
{
public:

	XSprite(std::shared_ptr<HDC> pDest, std::shared_ptr<HDC> pSrc);
	virtual ~XSprite();
	
	void SetCyclePic(std::shared_ptr<HBITMAP> cyclePic, int width, int height);
	void GetSize(const int& width, const int& height);

	void SetFrameRegions(std::vector<RECT>&& regions);
	void GetFrameRegions(const std::vector<RECT>& regions);

	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam) {}

	virtual bool GenerateNextFrame(const HBITMAP& hFrame, const int& x, const int& y, const int& width, const int& height);

public:
	POINT Position;

protected:
	std::shared_ptr<HDC> m_hdcDest;
	std::shared_ptr<HDC> m_hdcSrc;
	std::vector<RECT> m_regions;
	std::shared_ptr<HBITMAP> m_cyclePic;
	std::shared_ptr<HBITMAP> m_mask;
	HBITMAP m_hBackup;
};

