// ReadEventLogs.h: interface for the ReadEventLogs class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CREADEVENTLOGS_H__C472BDA4_4BD2_11D3_808E_0090271354E9__INCLUDED_)
#define AFX_CREADEVENTLOGS_H__C472BDA4_4BD2_11D3_808E_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CReadEventLogs  
{
public:
	bool OpenMdb();
	static CDaoDatabase* m_pMdb;
	CString m_strMdbName;
	BOOL ConvertSIDToString(PSID,LPTSTR,LPDWORD);
	void UpdateEventMdb(DWORD &);
	CString GetParameterText(CString);
	CString GetEventText(DWORD);
	LPTSTR m_lpstr[40];
	CString m_strMessageFile;
	CString GetEventMessage();
	PEVENTLOGRECORD m_eRec;
	CString m_strEventText;
	CString m_strCategory;
	CString m_strAppName;
	CString m_strLogName;
	CString GetCategoryText(WORD);
	void ReadLog(CString);
	static HANDLE m_hEventLog;
	CReadEventLogs(CString,CString);
	virtual ~CReadEventLogs();
	CSMADErrorLog* m_Log;

};

#endif // !defined(AFX_CREADEVENTLOGS_H__C472BDA4_4BD2_11D3_808E_0090271354E9__INCLUDED_)
