// SaveRegKey.h: interface for the CSaveRegKey class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAVEREGKEY_H__876601C4_5EB6_11D3_80D5_0090271354E9__INCLUDED_)
#define AFX_SAVEREGKEY_H__876601C4_5EB6_11D3_80D5_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSaveRegKey  
{
public:
	CString m_sRegFile;
	void SaveKey(CString);
	CSaveRegKey();
	virtual ~CSaveRegKey();
	CSMADErrorLog* m_Log;

};

#endif // !defined(AFX_SAVEREGKEY_H__876601C4_5EB6_11D3_80D5_0090271354E9__INCLUDED_)
