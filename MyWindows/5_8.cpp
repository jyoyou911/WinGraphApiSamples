#include <windows.h>
#include <tchar.h>

const TCHAR szProgram[] = _T("Window Region");
const TCHAR szRectWin[] = _T("Rectangular Window");
const TCHAR szEptcWin[] = _T("Elliptic Window");

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
  HWND hWnd = CreateWindow(_T("EDIT"), NULL,
                  WS_OVERLAPPEDWINDOW,
                  10, 10, 200, 100, GetDesktopWindow(),
                  NULL, hInstance, NULL);
  HRGN hRgn = CreateEllipticRgn(0, 0, 200, 100);
  SetWindowRgn(hWnd, hRgn, TRUE);
  ShowWindow(hWnd, SW_SHOW);

  SetWindowText(hWnd, szRectWin);
  //MessageBox(NULL, szRectWin, szProgram, MB_OK);

  SetWindowText(hWnd, szEptcWin);
  MessageBox(NULL, szEptcWin, szProgram, MB_OK);

  DestroyWindow(hWnd);

  return 0;
}
