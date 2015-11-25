// MyDrawingUtils.cpp : ���� DLL Ӧ�ó���ĵ���������
//
#include "stdafx.h"
#include <exception>
#include "MyDrawingUtils.h"


namespace MyDrawingUtils
{

	// �����ѵ�����Ĺ��캯����
	// �й��ඨ�����Ϣ������� MyDrawingUtils.h
	MyDrawingUtil::MyDrawingUtil(HDC hdc)
	{
		if (hdc == NULL)
			throw std::exception("HDC is NULL.");

		// Create memory device context
		m_hDestDC = ::CreateCompatibleDC(hdc);
		m_hSrcDC = ::CreateCompatibleDC(hdc);

		if (NULL == m_hDestDC || NULL == m_hSrcDC)
			throw std::exception("Create failed.");

		return;
	}

	MyDrawingUtil::~MyDrawingUtil(void)
	{
		if (NULL != m_hDestDC)
			::DeleteDC(m_hDestDC);

		if (NULL != m_hSrcDC)
			::DeleteDC(m_hSrcDC);
	}

	/// <summary>
	/// Ϊָ��ͼƬ���ɵ�ɫ�ɰ�
	/// </summary>
	/// <param name="crBackGround">����͸���ı���ɫ</param>
	/// <param name="hSrcBmp">ԴͼƬ</param>
	/// <param name="hMask">���ɵ��ɰ�</param>
	/// <param name="width">��</param>
	/// <param name="height">��</param>
	/// <param name="range">����ɫ����Χ</param>
	void MyDrawingUtil::CreateMask(COLORREF crBackGround, HBITMAP hSrcBmp, HBITMAP hMask, 
		int width, int height, int range)
	{
		HBITMAP hDestOld = NULL;
		HBITMAP hSrcOld = NULL;
		COLORREF crOld = ::SetBkColor(m_hSrcDC, crBackGround);

		// Select mask to DC
		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hMask);
		// Select frame to DC
		hSrcOld = (HBITMAP)::SelectObject(m_hSrcDC, hSrcBmp);

		// Color range
		if (range != 0)
		{
			int r_off = 0;
			int g_off = 0;
			int b_off = 0;
			COLORREF pix = 0x00000000;
			for (int y = 0; y < height; ++y)
			{
				for (int x = 0; x < width; ++x)
				{
					pix = ::GetPixel(m_hSrcDC, x, y);
					r_off = ((pix & 0x00FF0000) >> 16)
						- ((crBackGround & 0x00FF0000) >> 16);
					g_off = ((pix & 0x0000FF00) >> 8)
						- ((crBackGround & 0x0000FF00) >> 8);
					b_off = (pix & 0x000000FF)
						- (crBackGround & 0x000000FF);
					if (abs(r_off) + abs(g_off) + abs(b_off) < range)
						::SetPixel(m_hSrcDC, x, y, crBackGround);
				}
			}
		}

		// Reset to black
		::BitBlt(m_hDestDC, 0, 0, width, height, NULL, 0, 0, BLACKNESS);
		// Color convert to make mask
		::BitBlt(m_hDestDC, 0, 0, width, height, m_hSrcDC, 0, 0, SRCCOPY);

		// Set background to white
		::SetBkColor(m_hSrcDC, 0x00FFFFFF);
		// Bmp | Mask
		// Make source bmp's transparent part to white
		::BitBlt(m_hSrcDC, 0, 0, width, height, m_hDestDC, 0, 0, SRCPAINT);

		if (hDestOld)
			::SelectObject(m_hDestDC, hDestOld);
		if (hSrcOld)
			::SelectObject(m_hSrcDC, hSrcOld);
		::SetBkColor(m_hSrcDC, crOld);
	}

	/// <summary>
	/// ����ͼ����
	/// </summary>
	/// <param name="hDestBmp">Ŀ���</param>
	/// <param name="hSrcBmp">Դ��</param>
	/// <param name="xDest">Ŀ������X����</param>
	/// <param name="yDest">Ŀ������Y����</param>
	/// <param name="width">��</param>
	/// <param name="height">��</param>
	/// <param name="xSrc">Դ�����X����</param>
	/// <param name="ySrc">Դ�����Y����</param>
	/// <param name="rop">������Ĭ��Ϊ���ƣ�</param>
	void MyDrawingUtil::CopyLayer(HBITMAP hDestBmp, int xDest, int yDest, int width, int height,
		HBITMAP hSrcBmp, int xSrc, int ySrc, DWORD rop)
	{
		HBITMAP hDestOld = NULL;
		HBITMAP hSrcOld = NULL;

		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hDestBmp);
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hSrcBmp);
		::BitBlt(m_hDestDC, xDest, yDest, width, height, m_hSrcDC, xSrc, ySrc, rop);
		// Restore
		if (hDestOld)
			::SelectObject(m_hDestDC, hDestOld);
		if (hSrcOld)
			::SelectObject(m_hSrcDC, hSrcOld);
	}

	/// <summary>
	/// �������ͼ���㡣����͸��ɫ�Ļ��ƺ�Alpha��ϡ�
	/// </summary>
	/// <param name="hDestBmp">Ŀ���</param>
	/// <param name="hSrcBmp">Դ��</param>
	/// <param name="hMaskBmp">Դ���ɰ�</param>
	/// <param name="hBlend">Alpha������ڴ�ͼ��</param>
	/// <param name="x">Ŀ������X����</param>
	/// <param name="y">Ŀ������Y����</param>
	/// <param name="width">��</param>
	/// <param name="height">��</param>
	/// <param name="x_offset">Դ�����X����</param>
	/// <param name="y_offset">Դ�����Y����</param>
	/// <param name="alpha">��϶�</param>
	void MyDrawingUtil::Mix2Layers(HBITMAP hDestBmp, int x, int y, int width, int height,
		HBITMAP hSrcBmp, int x_offset, int y_offset,
		HBITMAP hMaskBmp, HBITMAP hBlend, int alpha)
	{
		HBITMAP hDestOld = NULL;
		HBITMAP hSrcOld = NULL;

		// Completely transparent will draw nothing
		if (0 == alpha)
			return;

		// 1. Copy the part of dest bmp
		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hBlend);
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hDestBmp);
		::BitBlt(m_hDestDC, 0, 0, width, height,
			m_hSrcDC, x, y,
			SRCCOPY);
		::SelectObject(m_hSrcDC, hSrcOld);
		::SelectObject(m_hDestDC, hDestOld);

		// 2. Alpha blend
		//	Transparent: garbage
		//	Opaque     : blend of src bmp and dest bmp
		BLENDFUNCTION blend = { (BYTE)AC_SRC_OVER, (BYTE)0, (BYTE)alpha, (BYTE)0 };
		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hBlend);
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hSrcBmp);
		::AlphaBlend(m_hDestDC, 0, 0, width, height,
			m_hSrcDC, x_offset, y_offset, width, height,
			blend);
		::SelectObject(m_hSrcDC, hSrcOld);
		::SelectObject(m_hDestDC, hDestOld);

		// 3. Dest bmp XOR blend result
		//	Transparent: garbage XOR dest bmp
		//	Opaque     : blend result XOR dest bmp
		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hDestBmp);
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hBlend);
		::BitBlt(m_hDestDC, x, y, width, height,
			m_hSrcDC, x_offset, y_offset,
			//SRCCOPY);
			SRCINVERT);
		::SelectObject(m_hSrcDC, hSrcOld);
		::SelectObject(m_hDestDC, hDestOld);

		// 4. Dest bmp & Mask
		//	Transparent: garbage XOR dest bmp
		//	Opaque     : black(0x0)
		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hDestBmp);
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hMaskBmp);
		::BitBlt(m_hDestDC, x, y, width, height,
			m_hSrcDC, x_offset, y_offset,
			//SRCCOPY);
			SRCAND);
		::SelectObject(m_hSrcDC, hSrcOld);
		::SelectObject(m_hDestDC, hDestOld);

		// 5. Again: Dest bmp XOR blend result
		//	Transparent: dest bmp 
		//	Opaque     : blend result
		hDestOld = (HBITMAP) ::SelectObject(m_hDestDC, hDestBmp);
		hSrcOld = (HBITMAP) ::SelectObject(m_hSrcDC, hBlend);
		::BitBlt(m_hDestDC, x, y, width, height,
			m_hSrcDC, 0, 0,
			SRCINVERT);
		::SelectObject(m_hSrcDC, hSrcOld);
		::SelectObject(m_hDestDC, hDestOld);

		return;
	}

};