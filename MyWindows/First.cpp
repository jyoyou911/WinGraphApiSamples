#define STRICT
#include <windows.h>
#include <tchar.h>
#include <assert.h>

static const TCHAR szOperation[] = _T("open");
static const TCHAR szAddress[] = _T("www.google.com");

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HINSTANCE hResult = ShellExecute(NULL, szOperation, szAddress, NULL, NULL, SW_SHOWNORMAL);
	assert(hResult > (HINSTANCE) HINSTANCE_ERROR);

	return 0;
}