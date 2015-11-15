// KWindow.cpp: KWindow クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include "KWindow.h"
#include <assert.h>


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

LRESULT KWindow::WndProc(HWND hWnd, UINT uMsg, 
		WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		this->OnCreate(hWnd);
		return 0;
	case WM_KEYDOWN:
		this->OnKeyDown(wParam, lParam);
		return 0;
	case WM_MOUSEMOVE:
		this->OnMouseMove(wParam, lParam);
		return 0;
	case WM_LBUTTONDOWN:
		this->OnLButtonDown(wParam, lParam);
		return 0;
	case WM_LBUTTONUP:
		this->OnLButtonUp(wParam, lParam);
		return 0;
	case WM_TIMER:
		this->OnTimer((UINT) wParam);
		return 0;
	case WM_PAINT:
		;
		PAINTSTRUCT ps;
		BeginPaint(m_hWnd, &ps);
		this->OnDraw(ps.hdc);
		EndPaint(m_hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

LRESULT CALLBACK KWindow::WindowProc(HWND hWnd, UINT uMsg, 
		WPARAM wParam, LPARAM lParam)
{
	KWindow* pWindow;

	if (WM_NCCREATE == uMsg) 
	{
		assert(!IsBadReadPtr((void*)lParam, sizeof(CREATESTRUCT)));
		MDICREATESTRUCT* pMDIC = 
			(MDICREATESTRUCT*) ((LPCREATESTRUCT)lParam)->lpCreateParams;
		pWindow = (KWindow*) pMDIC->lParam;
		assert(!IsBadReadPtr(pWindow, sizeof(KWindow)));
		SetWindowLong(hWnd, GWL_USERDATA, (LONG) pWindow);
	}
	else
		pWindow = (KWindow*) GetWindowLong(hWnd, GWL_USERDATA);

	if (pWindow)
		return pWindow->WndProc(hWnd, uMsg, wParam, lParam);
	else 
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool KWindow::RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst)
{
	WNDCLASSEX wc;

	if (!GetClassInfoEx(hInst, lpszClass, &wc))
	{
		GetWndClassEx(wc);
		wc.hInstance = hInst;
		wc.lpszClassName = lpszClass;
		if (!RegisterClassEx(&wc))
			return false;
	}

	return true;
}

bool KWindow::CreateEX(DWORD dwExStyle, LPCTSTR lpszClass, 
		LPCTSTR lpszName, DWORD dwStyle, 
		int x, int y, int nWidth, int nHeight, 
		HWND hParent, HMENU hMenu, HINSTANCE hInstance)
{
	if (!this->RegisterClass(lpszClass, hInstance))
		return false;

	// Record window info
	m_nX = x;
	m_nY = y;
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	// Call sys create API
	MDICREATESTRUCT mdic;
	memset(&mdic, 0x00, sizeof(mdic));
	mdic.lParam = (LPARAM) this;
	m_hWnd = CreateWindowEx(dwExStyle, lpszClass, lpszName, 
		dwStyle, x, y, nWidth, nHeight, hParent, hMenu, hInstance, &mdic);

	return m_hWnd != NULL;
}

void KWindow::GetWndClassEx(WNDCLASSEX& wc)
{
	memset(&wc ,0x00, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style  = 0;
	wc.lpfnWndProc = this->WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = NULL;
	wc.hIcon = NULL;
	wc.hCursor = NULL; //LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = NULL;
	wc.lpszMenuName = NULL;
	wc.hIconSm = NULL;

	return;
}

WPARAM KWindow::MessageLoop(void)
{
	MSG msg;
	
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}