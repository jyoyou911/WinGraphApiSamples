// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� MYDRAWINGUTILS_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// MYDRAWINGUTILS_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef MYDRAWINGUTILS_EXPORTS
#define MYDRAWINGUTILS_API __declspec(dllexport)
#else
#define MYDRAWINGUTILS_API __declspec(dllimport)
#endif

#include <windows.h>

namespace MyDrawingUtils
{

	// �����Ǵ� MyDrawingUtils.dll ������
	class MYDRAWINGUTILS_API MyDrawingUtil {
	public:
		MyDrawingUtil(HDC);
		virtual ~MyDrawingUtil(void);

		
		void CreateMask(COLORREF crBackGround,
			HBITMAP hSrcBmp, HBITMAP hMask, int width, int height, int range);

		void CopyLayer(HBITMAP hDestBmp, HBITMAP hSrcBmp, 
			int xDest, int yDest, int width, int height, int xSrc, int ySrc, DWORD rop = SRCCOPY);

		void Mix2Layers(HBITMAP hDestBmp, HBITMAP hSrcBmp, HBITMAP hMaskBmp, HBITMAP hBlend,
			int x, int y, int width, int height, int x_offset, int y_offset, int alpha);
		
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