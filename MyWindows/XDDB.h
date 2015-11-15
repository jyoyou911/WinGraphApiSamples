// XDDB.h: XDDB クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XDDB_H__BEEF80E7_0CAD_4B81_89AC_07C2EFD0AE26__INCLUDED_)
#define AFX_XDDB_H__BEEF80E7_0CAD_4B81_89AC_07C2EFD0AE26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class XDDB  
{
public:
	XDDB(int width, int height);
	virtual ~XDDB();
	HBITMAP Handle(void) {return m_hBitMap};
private:
	HBITMAP m_hBitMap;
	int m_nWidth;
	int m_nHeight;
};

#endif // !defined(AFX_XDDB_H__BEEF80E7_0CAD_4B81_89AC_07C2EFD0AE26__INCLUDED_)
