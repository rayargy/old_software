// SMADErrorLog.h: interface for the CSMADErrorLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMADERRORLOG_H__876601C6_5EB6_11D3_80D5_0090271354E9__INCLUDED_)
#define AFX_SMADERRORLOG_H__876601C6_5EB6_11D3_80D5_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSMADErrorLog  
{
public:
	void DestroyInstance();
	CSMADErrorLog* GetInstance();
	void WriteLogEntry(CString,DWORD);
	static FILE* m_pLog;
	static DWORD m_dwDebugLevel;
	static CSMADErrorLog *c_instance;
	CString m_sSMADLog;
	CSMADErrorLog();
	virtual ~CSMADErrorLog();

};

#endif // !defined(AFX_SMADERRORLOG_H__876601C6_5EB6_11D3_80D5_0090271354E9__INCLUDED_)
