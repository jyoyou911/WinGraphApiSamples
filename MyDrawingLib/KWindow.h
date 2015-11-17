#pragma once

#include "stdafx.h"
#include <tchar.h>


namespace MyDrawing
{

	class KWindow
	{
	public:
		MYDRAWINGLIBDLL_API KWindow()
			: m_hWnd(NULL) {};
		MYDRAWINGLIBDLL_API virtual ~KWindow() {};

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

	public:
		MYDRAWINGLIBDLL_API static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);

		MYDRAWINGLIBDLL_API virtual void GetWndClassEx(WNDCLASSEX& wc);

		MYDRAWINGLIBDLL_API virtual bool CreateEX(DWORD dwExStyle, LPCTSTR lpszClass,
			LPCTSTR lpszName, DWORD dwStyle,
			int x, int y, int nWidth, int nHeight,
			HWND hParent, HMENU hMenu, HINSTANCE hInstance);

		MYDRAWINGLIBDLL_API bool RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst);

		MYDRAWINGLIBDLL_API virtual WPARAM MessageLoop(void);

		MYDRAWINGLIBDLL_API BOOL ShowWindow(int nCmdShow) const
		{
			return ::ShowWindow(m_hWnd, nCmdShow);
		}

		MYDRAWINGLIBDLL_API BOOL UpdateWindow(void) const
		{
			return ::UpdateWindow(m_hWnd);
		}

	protected:
		HWND m_hWnd;
		int  m_nX;
		int  m_nY;
		int  m_nWidth;
		int  m_nHeight;
	};

}