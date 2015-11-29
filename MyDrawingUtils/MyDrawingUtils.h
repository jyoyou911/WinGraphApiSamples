#pragma once

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MYDRAWINGUTILS_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MYDRAWINGUTILS_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef MYDRAWINGUTILS_EXPORTS
#define MYDRAWINGUTILS_API __declspec(dllexport)
#else
#define MYDRAWINGUTILS_API __declspec(dllimport)
#endif

#include <windows.h>

namespace MyDrawingUtils
{

	static const long OK = 0;
	static const long ERR = 1;

	typedef enum
	{
		DIB_1BPP,           //   2 color image, palette-based
		DIB_2BPP,           //   4 color image, palttte-based
		DIB_4BPP,           //  16 color image, palette-based
		DIB_4BPPRLE,        //  16 color image, palette-based, RLE compressed
		DIB_8BPP,           // 256 color image, palette-based
		DIB_8BPPRLE,        // 256 color image, palette-based, RLE compressed

		DIB_16RGB555,       // 15 bit RGB color image, 5-5-5
		DIB_16RGB565,       // 16 bit RGB color image, 5-6-5, 1 bit unused
		DIB_24RGB888,       // 24 bit RGB color image, 8-8-8
		DIB_32RGB888,       // 32 bit RGB color image, 8-8-8, 8 bit unused

		DIB_32RGBA8888,     // 32 bit RGBA color image, 8-8-8-8

		DIB_16RGBbitfields, // 16 bit RGB color image, nonstandard bit masks,
							// NT-only
		DIB_32RGBbitfields, // 32 bit RGB color image, nonstandard bit masks,
							// NT-only

		DIB_JPEG,           // embedded JPEG image
		DIB_PNG             // embedded PNG image
	}  DIBFormat;

	typedef enum
	{
		DIB_BMI_NEEDFREE = 1,
		DIB_BMI_READONLY = 2,
		DIB_BITS_NEEDFREE = 4,
		DIB_BITS_READONLY = 8
	} DIBLoadType;

	// 此类是从 MyDrawingUtils.dll 导出的
	class MYDRAWINGUTILS_API MyDrawingUtil {
	public:
		MyDrawingUtil(HDC);
		~MyDrawingUtil(void);

		
		void CreateMask(COLORREF crBackGround,
			HBITMAP hSrcBmp, HBITMAP hMask, int width, int height, int range = 50);

		void CopyLayer(HBITMAP hDestBmp, int xDest, int yDest, int width, int height,
			HBITMAP hSrcBmp, int xSrc, int ySrc, DWORD rop = SRCCOPY);

		void CopyZoomedLayer(HBITMAP hDestBmp, int xDest, int yDest, int wDest, int hDest,
			HBITMAP hSrcBmp, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop = SRCCOPY);

		void Mix2Layers(HBITMAP hDestBmp, int x, int y, int width, int height, 
			HBITMAP hSrcBmp, int x_offset, int y_offset, 
			HBITMAP hMaskBmp, HBITMAP hBlend, int alpha = 0xFF);

		long LoadBmp(LPCTSTR lpszBmpFileName, HBITMAP& hBmpHandle, BITMAP& scBmpInfo);

		
	private:
		HDC m_hDestDC;
		HDC m_hSrcDC;
	};

	class MYDRAWINGUTILS_API KDIB
	{
	public:
		KDIB();
		virtual ~KDIB();

		bool Create(int width, int height, int bitcount);

		bool AttachDIB(BITMAPINFO * pDIB, BYTE * pBits, int flags);
		bool LoadFile(const TCHAR * pFileName);
		bool LoadBitmap(HMODULE hModlue, LPCTSTR pBitmapName);
		void ReleaseDIB(void);          // release memory
		int  GetWidth(void)       const { return m_nWidth; }
		int  GetHeight(void)      const { return m_nHeight; }
		int  GetDepth(void)       const { return m_nColorDepth; }
		BITMAPINFO * GetBMI(void) const { return m_pBMI; }
		BYTE * GetBits(void)      const { return m_pBits; }
		int  GetBPS(void)         const { return m_nBPS; }

		bool IsCompressed(void) const
		{
			return (m_nImageFormat == DIB_4BPPRLE) ||
				(m_nImageFormat == DIB_8BPPRLE) ||
				(m_nImageFormat == DIB_JPEG) ||
				(m_nImageFormat == DIB_PNG);
		}

	protected:
		DIBFormat    m_nImageFormat;  // pixel array format
		int          m_Flags;         // DIB_BMI_NEEDFREE, ....
		BITMAPINFO * m_pBMI;          // BITMAPINFOHEADER + mask + color table
		BYTE       * m_pBits;         // pixel array

		RGBTRIPLE  * m_pRGBTRIPLE;    // OS/2 DIB color table within m_pBMI
		RGBQUAD    * m_pRGBQUAD;      // V3,4,5 DIB color table within m_pBMI
		int          m_nClrUsed;      // No. of colors in color table
		int          m_nClrImpt;      // Real color used
		DWORD      * m_pBitFields;    // 16, 32-bpp masks within m_pBMI

		int          m_nWidth;        // image pixel width
		int          m_nHeight;       // image pixel height, positive
		int          m_nPlanes;       // plane count
		int          m_nBitCount;     // bit per plane
		int          m_nColorDepth;   // bit count * plane count
		int          m_nImageSize;    // pixel array size

									  // precalculated values
		int          m_nBPS;          // byte per scan line, per plane
		BYTE *       m_pOrigin;       // point to logical origin
		int          m_nDelta;        // delta to next scan line


	};

	class MYDRAWINGUTILS_API KWindow
	{
	public:
		KWindow()
			: m_hWnd(NULL) {}
		virtual ~KWindow() {}

		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);

		virtual void GetWndClassEx(WNDCLASSEX& wc);

		virtual bool CreateEX(DWORD dwExStyle, LPCTSTR lpszClass,
			LPCTSTR lpszName, DWORD dwStyle,
			int x, int y, int nWidth, int nHeight,
			HWND hParent, HMENU hMenu, HINSTANCE hInstance);

		virtual bool RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst);

		virtual WPARAM MessageLoop(void);

		BOOL ShowWindow(int nCmdShow) const
		{
			return ::ShowWindow(m_hWnd, nCmdShow);
		}

		BOOL UpdateWindow(void) const
		{
			return ::UpdateWindow(m_hWnd);
		}

	protected:
		virtual void OnCreate(HWND hWnd) {}

		virtual void OnDraw(HDC hDC) = 0;

		virtual void OnKeyDown(WPARAM wParam, LPARAM lParam) {}

		virtual void OnMouseMove(WPARAM wParam, LPARAM lParam) {}

		virtual void OnLButtonDown(WPARAM wParam, LPARAM lParam) {}

		virtual void OnLButtonUp(WPARAM wParam, LPARAM lParam) {}

		virtual void OnTimer(UINT nIDEvent) {}

		virtual LRESULT WndProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);

	protected:
		HWND m_hWnd;
		int  m_nX;
		int  m_nY;
		int  m_nWidth;
		int  m_nHeight;
	};

};