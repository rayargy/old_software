// EventLog.h: interface for the EventLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTLOG_H__FE23D975_FD35_11D0_A5C8_00AA00BBBEE3__INCLUDED_)
#define AFX_EVENTLOG_H__FE23D975_FD35_11D0_A5C8_00AA00BBBEE3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CEventLog  
{
public:
	BOOL AuditFailure(DWORD dwEventID, LPCTSTR * szMessage, WORD wNumElements);
	BOOL AuditSuccess(DWORD dwEventID, LPCTSTR * szMessage, WORD wNumElements);
	BOOL PostInfo(DWORD dwEventID, LPCTSTR * szMessage, WORD wNumElements);
	BOOL PostWarning(DWORD dwEventID, LPCTSTR * szMessage, WORD wNumElements);
	BOOL PostError(DWORD dwEventID, LPCTSTR * szMessage, WORD wNumElements);
	CString GetAppName();
	void SetAppName(CString AppName);
	CEventLog();
	CEventLog(CString AppName);
	virtual ~CEventLog();

private:
	BOOL PostEvent(WORD wCategory, DWORD dwEventID, LPCTSTR * szMessage, WORD wNumElements);
	void InitEventLogger();
	CString m_sAppName;
};

#endif // !defined(AFX_EVENTLOG_H__FE23D975_FD35_11D0_A5C8_00AA00BBBEE3__INCLUDED_)
