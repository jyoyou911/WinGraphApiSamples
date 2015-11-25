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

	// 此类是从 MyDrawingUtils.dll 导出的
	class MYDRAWINGUTILS_API MyDrawingUtil {
	public:
		MyDrawingUtil(HDC);
		virtual ~MyDrawingUtil(void);

		
		void CreateMask(COLORREF crBackGround,
			HBITMAP hSrcBmp, HBITMAP hMask, int width, int height, int range = 50);

		void CopyLayer(HBITMAP hDestBmp, int xDest, int yDest, int width, int height,
			HBITMAP hSrcBmp, int xSrc, int ySrc, DWORD rop = SRCCOPY);

		void Mix2Layers(HBITMAP hDestBmp, int x, int y, int width, int height, 
			HBITMAP hSrcBmp, int x_offset, int y_offset, 
			HBITMAP hMaskBmp, HBITMAP hBlend, int alpha = 0xFF);
		
	private:
		HDC m_hDestDC;
		HDC m_hSrcDC;
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