// TransRegistry.h: interface for the CTransRegistry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRANSREGISTRY_H__87789423_7017_11D1_AEAF_00AA00C24DFE__INCLUDED_)
#define AFX_TRANSREGISTRY_H__87789423_7017_11D1_AEAF_00AA00C24DFE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define STATUS_NEW_TRANS FALSE
#define STATUS_READ_TRANS TRUE

// The minimum number of transactions that will
// be found in the registry (in order to keep the
// queue long enough that TransWriter has time to
// read new transactions before CommServer loops
// around the queue and overwrites them).
#define MIN_NUMTRANS 50

#include "Registry.h"
class Registry;

class CTransRegistry : public CObject  
{
public:
	BOOL ReadNextTrans(CString*, long*);
	BOOL WriteTrans(CString);
	CTransRegistry();
	virtual ~CTransRegistry();

private:
	HKEY OpenTransKey(DWORD);
	BOOL GetStatusByte(DWORD);
	DWORD m_dwNextToWrite;
	DWORD m_dwNextToRead;
	DWORD m_dwNumTrans;
	CRegistry* m_Reg;
	HKEY m_hKeyHandle;
};

#endif // !defined(AFX_TRANSREGISTRY_H__87789423_7017_11D1_AEAF_00AA00C24DFE__INCLUDED_)
