#if !defined(AFX_MD5CHECKSUM_H__2BC7928E_4C15_11D3_B2EE_A4A60E20D2C3__INCLUDED_)
#define AFX_MD5CHECKSUM_H__2BC7928E_4C15_11D3_B2EE_A4A60E20D2C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/*****************************************************************************************


*****************************************************************************************/
class CMD5Checksum
{
public:
	static CString GetMD5OfString(CString strString);
	//interface functions for the RSA MD5 calculation
	static CString GetMD5(const CString& strFilePath);

protected:
	//constructor/destructor
	CMD5Checksum();
	virtual ~CMD5Checksum() {};

	//RSA MD5 implementation
	void Transform(BYTE Block[64]);
	void Update(BYTE* Input, ULONG nInputLen);
	CString Final();
	inline DWORD RotateLeft(DWORD x, int n);
	inline void FF(DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
	inline void GG(DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
	inline void HH(DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
	inline void II(DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);

	//utility functions
	inline void DWordToByte(BYTE* Output, DWORD* Input, UINT nLength);
	inline void ByteToDWord(DWORD* Output, BYTE* Input, UINT nLength);

private:
	BYTE  m_lpszBuffer[64];  //input buffer
	ULONG m_nCount[2];   //number of bits, modulo 2^64 (lsb first)
	ULONG m_lMD5[4];   //MD5 checksum
};

#endif // !defined(AFX_MD5CHECKSUM_H__2BC7928E_4C15_11D3_B2EE_A4A60E20D2C3__INCLUDED_)