// KWindow.h: KWindow クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KWINDOW_H__A92ABE37_35F8_4EA5_A716_42CE59A141DD__INCLUDED_)
#define AFX_KWINDOW_H__A92ABE37_35F8_4EA5_A716_42CE59A141DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <tchar.h>

class KWindow  
{
public:
	KWindow()
		: m_hWnd(NULL) {}
	virtual ~KWindow() {}
	
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
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, 
		WPARAM wParam, LPARAM lParam);

	virtual void GetWndClassEx(WNDCLASSEX& wc);

	virtual bool CreateEX(DWORD dwExStyle, LPCTSTR lpszClass, 
		LPCTSTR lpszName, DWORD dwStyle, 
		int x, int y, int nWidth, int nHeight, 
		HWND hParent, HMENU hMenu, HINSTANCE hInstance);

	bool RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst);

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
	HWND m_hWnd;
	int  m_nX;
	int  m_nY;
	int  m_nWidth;
	int  m_nHeight;
};

/*
class KToolbar
{
   HWND      m_hToolTip;
   UINT      m_ControlID;
public:

  HWND       m_hWnd;

  KToolbar(): m_hWnd(NULL), m_hToolTip(NULL), m_controlID(0){}

  void Create(HWND hParent, HINSTANCE hInstance,
              UINT nControlID, const TBBUTTON * pButtons,
              int nCount);

  void Resize(HWND hParent, int width, int height);
};

typedef enum
{
  PANE_1,
  PANE_2,
  PANE_3
};

class KStatusWindow
{

public:
  HWND m_hWnd;
  UINT m_ControlID;
  KStatusWindow() : m_hWnd(NULL), m_controlID(0) {}

  void Create(HWND hParent, UINT nControlID);
  void Resize(HWND hParent, int width, int height);
  void SetText(int pane, HINSTANCE hInst, int messid,
               int param=0);
  void SetText(int pane, LPCTSTR message);
};
*/

#endif // !defined(AFX_KWINDOW_H__A92ABE37_35F8_4EA5_A716_42CE59A141DD__INCLUDED_)
