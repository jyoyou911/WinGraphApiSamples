#define STRICT
#define WIN32_LEAN_AND_MEAN

#define __in 
#define __in_opt 
#define __in_bcount(x) 
#define __in_bcount_opt(x)
#define __in_ecount(x)
#define __in_ecount_opt(x)
#define __out 
#define __out_opt 
#define __out_bcount(x) 
#define __out_bcount_opt(x) 
#define __out_ecount(x)
#define __out_ecount_opt(x)
#define __out_ecount_part_opt(p1, p2)
#define __inout 
#define __inout_opt

typedef char UINT8;

#include "KWindow.h"

#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

static const TCHAR szMessage[] = _T("Direct2D Test");
static const TCHAR szFace[] = _T("Times New Roman");
static const TCHAR szHint[] = _T("Press ESC to quit");
static const TCHAR szProgram[] = _T("D2DTest");


class KD2DWindow : public KWindow
{
public:
	KD2DWindow();
	virtual ~KD2DWindow();

protected:
	void OnCreate(HWND hWnd);

	void OnKeyDown(WPARAM wParam,LPARAM lParam)
	{
		if (VK_ESCAPE == wParam)
		{
			this->Destory();
			PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		}

		return;
	}

	void OnDraw(HDC hDC)
	{		

		return;
	}

	void Destory(void);
private:
	D2D1Factory* m_pD2D1Factory;
	ID2D1HwndRenderTarget* m_pRT;
	bool m_bValid;
};

KD2DWindow::KD2DWindow()
	: m_pD2D1Factory(NULL), m_pRT(NULL), m_bValid(false);
{}

KD2DWindow::~KD2DWindow()
{}

void KD2DWindow::OnCreate(HWND hWnd)
{
	HRESULT hr = NULL;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

	if (!SUCCEEDED(hr))
		return;

	// Obtain the size of the drawing area.
	RECT rc;
	GetClientRect(hWnd, &rc);

	// Create a Direct2D render target			
	hr = m_pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hWnd,
			D2D1::SizeU(
				rc.right - rc.left,
				rc.bottom - rc.top)
		),
		&m_pRT);

	if (SUCCEEDED(hr))
		m_bValid = true;

	return;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmd, int nShow)
{
	KHelloWindow win;

	win.CreateEX(0, szProgram, szProgram, WS_POPUP, 0, 0, 
		400, 
		300, 
		NULL, NULL, hInst);

	win.ShowWindow(nShow);
	win.UpdateWindow();

	return win.MessageLoop();
}