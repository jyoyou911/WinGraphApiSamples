#define STRICT
#define WIN32_LEAN_AND_MEAN

#include "KWindow.h"

static const TCHAR szMessage[] = _T("Hello World");
static const TCHAR szFace[] = _T("Times New Roman");
static const TCHAR szHint[] = _T("Press ESC to quit");
static const TCHAR szProgram[] = _T("Hello3");

static void CenterText(HDC hDC, int x, int y, LPCTSTR szFace, LPCTSTR szMessage, int point)
{
	HFONT hFont = CreateFont(-point * GetDeviceCaps(hDC, LOGPIXELSY) / 72, 
		0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE, 
		ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
		PROOF_QUALITY, VARIABLE_PITCH, szFace);
	//assert(hFont);

	HGDIOBJ hOld = SelectObject(hDC, hFont);

	SetTextAlign(hDC, TA_CENTER | TA_BASELINE);

	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(0, 0, 0xFF));
	TextOut(hDC, x, y, szMessage, _tcslen(szMessage));
	SelectObject(hDC, hOld);
	DeleteObject(hFont);

	return;
}

class KHelloWindow : public KWindow
{
	void OnKeyDown(WPARAM wParam,LPARAM lParam)
	{
		if (VK_ESCAPE == wParam)
			PostMessage(m_hWnd, WM_CLOSE, 0, 0);

		return;
	}

	void OnDraw(HDC hDC)
	{
		TextOut(hDC, 0, 0, szHint, lstrlen(szHint));
		CenterText(hDC, 
			GetDeviceCaps(hDC, HORZRES) / 2, 
			GetDeviceCaps(hDC, VERTRES) / 2, 
			szFace, szMessage, 72);

		return;
	}
};

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmd, int nShow)
{
	KHelloWindow win;

	win.CreateEX(0, szProgram, szProgram, WS_POPUP, 0, 0, 
		GetSystemMetrics(SM_CXSCREEN), 
		GetSystemMetrics(SM_CYSCREEN), 
		NULL, NULL, hInst);

	win.ShowWindow(nShow);
	win.UpdateWindow();

	return win.MessageLoop();
}