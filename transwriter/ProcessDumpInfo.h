// ProcessDumpInfo.h: interface for the CProcessDumpInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSDUMPINFO_H__1577EA25_CBA5_11D2_A3D9_00E098031625__INCLUDED_)
#define AFX_PROCESSDUMPINFO_H__1577EA25_CBA5_11D2_A3D9_00E098031625__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CProcessDumpInfo  
{
public:
	void SetConnectString( CString sConnect );
	CString GetConnectString();
	void SetDebugLevel( int iDebugLevel );
	int GetDebugLevel();
	CProcessDumpInfo();
	virtual ~CProcessDumpInfo();

private:
	CString m_sConnect;
	int m_iDebugLevel;
};

#endif // !defined(AFX_PROCESSDUMPINFO_H__1577EA25_CBA5_11D2_A3D9_00E098031625__INCLUDED_)
