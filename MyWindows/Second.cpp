//#pragma comment(linker, "-merge:.rdata=.text")
#pragma comment(linker, "-align:512")
//#pragma comment(linker, "/nodefaultlib:\"libcd.lib\"")
//#pragma comment(linker, "/entry:WinMainCRTStartup")
//#pragma comment(linker, "/entry:MyMain")
#pragma comment(linker, "/opt:nowin98")

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>
//#include <assert.h>

void CenterText(HDC hDC, int x, int y, LPCTSTR szFace, LPCTSTR szMessage, int point)
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

static const TCHAR szMessage[] = _T("Hello World");
static const TCHAR szFace[] = _T("Times New Roman");


extern "C" void WinMainCRTStartup(void)
//extern "C" void MyMain(void)
{
	HDC hDC = GetDC(NULL);
	//assert(hDC);

	CenterText(hDC, GetSystemMetrics(SM_CXSCREEN) / 2, 
		GetSystemMetrics(SM_CYSCREEN) / 2, 
		szFace, szMessage, 72);

	ReleaseDC(NULL, hDC);
	ExitProcess(0);

	return;
}

