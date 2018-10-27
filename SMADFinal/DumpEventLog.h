// DumpEventLog.h: interface for the CDumpEventLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DUMPEVENTLOG_H__6BBCA752_4B41_11D3_808C_0090271354E9__INCLUDED_)
#define AFX_DUMPEVENTLOG_H__6BBCA752_4B41_11D3_808C_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CDumpEventLog  
{
public:
	void DumpLogs();
	BOOL DumpAndClearEventLog(CString);
	BOOL FileExists(CString);
	CStringList m_slLogName;
	CString m_strEventLogPath;
	CString m_strMdbName;
	CDumpEventLog();
	virtual ~CDumpEventLog();
	CSMADErrorLog* m_Log;

};

#endif // !defined(AFX_DUMPEVENTLOG_H__6BBCA752_4B41_11D3_808C_0090271354E9__INCLUDED_)
