// KDIB.cpp: KDIB クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyDrawingUtils.h"

namespace MyDrawingUtils
{

	KDIB::KDIB()
		: m_pBMI(NULL)
	{
		m_Flags = 0;
	}

	KDIB::~KDIB()
	{
		this->ReleaseDIB();
	}

	bool KDIB::Create(int width, int height, int bitcount)
	{
		BITMAPINFO * pDIB = (BITMAPINFO *) new BYTE[(width * height * bitcount) >> 3];
		if (pDIB)
			return TRUE;
		else
			return FALSE;
	}

	void KDIB::ReleaseDIB(void)
	{
		if (m_pBMI)
			delete[] m_pBMI;

		m_Flags = 0;

		return;
	}

	bool KDIB::AttachDIB(BITMAPINFO * pDIB, BYTE * pBits, int flags)
	{
		if (IsBadReadPtr(pDIB, sizeof(BITMAPCOREHEADER)))
			return false;

		ReleaseDIB();

		m_pBMI = pDIB;
		m_Flags = flags;

		DWORD size = *(DWORD *)pDIB; // always DWORD size

		int compression;
		// gather information from bitmap information header structures
		switch (size)
		{
		case sizeof(BITMAPCOREHEADER) :
		{
			BITMAPCOREHEADER * pHeader = (BITMAPCOREHEADER *)pDIB;

			m_nWidth = pHeader->bcWidth;
			m_nHeight = pHeader->bcHeight;
			m_nPlanes = pHeader->bcPlanes;
			m_nBitCount = pHeader->bcBitCount;
			m_nImageSize = 0;
			compression = BI_RGB;

			if (m_nBitCount <= 8)
			{
				m_nClrUsed = 1 << m_nBitCount;
				m_nClrImpt = m_nClrUsed;
				m_pRGBTRIPLE = (RGBTRIPLE *)((BYTE *)m_pBMI + size);
				m_pBits = (BYTE *)& m_pRGBTRIPLE[m_nClrUsed];
			}
			else
				m_pBits = (BYTE *)m_pBMI + size;
			break;
		}

		case sizeof(BITMAPINFOHEADER) :
#if (WINVER >= 0x0400)
		case sizeof(BITMAPV4HEADER) :
#endif
#if (WINVER >= 0x0500)
		case sizeof(BITMAPV5HEADER) :
#endif
		{
			BITMAPINFOHEADER * pHeader = &m_pBMI->bmiHeader;

			m_nWidth = pHeader->biWidth;
			m_nHeight = pHeader->biHeight;
			m_nPlanes = pHeader->biPlanes;
			m_nBitCount = pHeader->biBitCount;
			m_nImageSize = pHeader->biSizeImage;
			compression = pHeader->biCompression;

			m_nClrUsed = pHeader->biClrUsed;
			m_nClrImpt = pHeader->biClrImportant;

			if (m_nBitCount <= 8)
				if (m_nClrUsed == 0)    // 0 means full color table
					m_nClrUsed = 1 << m_nBitCount;

			if (m_nClrUsed)   // has a color table
			{
				if (m_nClrImpt == 0)    // 0 means all important
					m_nClrImpt = m_nClrUsed;

				if (compression == BI_BITFIELDS)
				{
					m_pBitFields = (DWORD *)((BYTE *)pDIB + size);
					m_pRGBQUAD = (RGBQUAD *)((BYTE *)pDIB + size +
						3 * sizeof(DWORD));
				}
				else
					m_pRGBQUAD = (RGBQUAD *)((BYTE *)pDIB + size);

				m_pBits = (BYTE *)& m_pRGBQUAD[m_nClrUsed];
			}
			else
			{
				if (compression == BI_BITFIELDS)
				{
					m_pBitFields = (DWORD *)((BYTE *)pDIB + size);
					m_pBits = (BYTE *)m_pBMI + size +
						3 * sizeof(DWORD);
				}
				else
					m_pBits = (BYTE *)m_pBMI + size;
			}
			break;
		}

		default:
			return false;
		}

		if (pBits)
			m_pBits = pBits;

		// precalculate information DIB parameters
		m_nColorDepth = m_nPlanes * m_nBitCount;
		m_nBPS = (m_nWidth * m_nBitCount + 31) / 32 * 4;

		if (m_nHeight < 0) // top-down bitmap
		{
			m_nHeight = -m_nHeight;    // change to positive
			m_nDelta = m_nBPS;         // forward
			m_pOrigin = m_pBits;        // scan0 .. scanN-1
		}
		else
		{
			m_nDelta = -m_nBPS;       // backward
			m_pOrigin = m_pBits + (m_nHeight - 1) * m_nBPS * m_nPlanes;
			// scanN-1..scan0
		}

		if (m_nImageSize == 0)
			m_nImageSize = m_nBPS * m_nPlanes * m_nHeight;

		// convert compression mode to image format
		switch (m_nBitCount)
		{
		case 0:
			/*
			if ( compression==BI_JPEG )
				m_nImageFormat = DIB_JPEG;
			else if ( compression==BI_PNG )
				m_nImageFormat = DIB_PNG;
			else
			  return false;
			  */

		case 1:
			m_nImageFormat = DIB_1BPP;
			break;

		case 2:
			m_nImageFormat = DIB_2BPP;
			break;

		case 4:
			if (compression == BI_RLE4)
				m_nImageFormat = DIB_4BPPRLE;
			else
				m_nImageFormat = DIB_4BPP;
			break;

		case 8:
			if (compression == BI_RLE8)
				m_nImageFormat = DIB_8BPPRLE;
			else
				m_nImageFormat = DIB_8BPP;
			break;

		case 16:
			if (compression == BI_BITFIELDS)
				m_nImageFormat = DIB_16RGBbitfields;
			else
				m_nImageFormat = DIB_16RGB555; // see below
			break;

		case 24:
			m_nImageFormat = DIB_24RGB888;
			break;

		case 32:
			if (compression == BI_BITFIELDS)
				m_nImageFormat = DIB_32RGBbitfields;
			else
				m_nImageFormat = DIB_32RGB888; // see below
			break;

		default:
			return false;
		}

		// try to understand bit fields
		if (compression == BI_BITFIELDS)
		{
			DWORD red = m_pBitFields[0];
			DWORD green = m_pBitFields[1];
			DWORD blue = m_pBitFields[2];

			if ((blue == 0x001F) && (green == 0x03E0) && (red == 0x7C00))
				m_nImageFormat = DIB_16RGB555;
			else if ((blue == 0x001F) && (green == 0x07E0) && (red == 0xF800))
				m_nImageFormat = DIB_16RGB565;
			else if ((blue == 0x00FF) && (green == 0xFF00) && (red == 0xFF0000))
				m_nImageFormat = DIB_32RGB888;
		}

		return true;
	}

	bool KDIB::LoadBitmap(HMODULE hModule, LPCTSTR pBitmapName)
	{
		HRSRC   hRes = FindResource(hModule, pBitmapName, RT_BITMAP);
		if (hRes == NULL)
			return false;

		HGLOBAL hGlb = LoadResource(hModule, hRes);

		if (hGlb == NULL)
			return false;

		BITMAPINFO * pDIB = (BITMAPINFO *)LockResource(hGlb);

		if (pDIB == NULL)
			return false;

		return AttachDIB(pDIB, NULL, DIB_BMI_READONLY | DIB_BITS_READONLY);
	}

	bool KDIB::LoadFile(const TCHAR * pFileName)
	{
		if (pFileName == NULL)
			return false;

		HANDLE handle = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (handle == INVALID_HANDLE_VALUE)
			return false;

		BITMAPFILEHEADER bmFH;

		DWORD dwRead = 0;
		ReadFile(handle, &bmFH, sizeof(bmFH), &dwRead, NULL);

		if ((bmFH.bfType == 0x4D42) &&
			(bmFH.bfSize <= GetFileSize(handle, NULL)))
		{
			BITMAPINFO * pDIB = (BITMAPINFO *) new BYTE[bmFH.bfSize];

			if (pDIB)
			{
				ReadFile(handle, pDIB, bmFH.bfSize, &dwRead, NULL);
				CloseHandle(handle);

				return AttachDIB(pDIB, NULL, DIB_BMI_NEEDFREE);
			}
		}
		CloseHandle(handle);

		return false;
	}

}