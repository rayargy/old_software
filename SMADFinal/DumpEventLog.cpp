// DumpEventLog.cpp: implementation of the CDumpEventLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DumpEventLog.h"
#include "registry.h"
#include "ReadEventLogs.h"

#define REGPATH "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDumpEventLog::CDumpEventLog()
{
	CRegistry CReg;
	long lRes = 0;
	HKEY hKey;
	CString sTemp;
	DWORD lpcbData = 0,lpType = 0;

	m_Log->GetInstance();
	m_Log->WriteLogEntry("CDumpEventLog::CDumpEventLog() Constructor",0);

	hKey = CReg.OpenRegKey(&lRes,REGPATH);
	if(lRes == ERROR_SUCCESS)
	{
		m_strMdbName = CReg.FetchStrValue(&lRes,hKey,"EventMdb");

		sTemp = CReg.FetchStrValue(&lRes,hKey,"LogsToSave");
		if(lRes == ERROR_SUCCESS)
		{
			for(int iPos = sTemp.Find(';'); iPos != -1;iPos = sTemp.Find(';'))
			{
				m_slLogName.AddTail(sTemp.Left(iPos));
				sTemp = sTemp.Mid(iPos + 1);
			}
			if(sTemp != "")
				m_slLogName.AddTail(sTemp);

		}
		CReg.CloseRegKey(hKey);

		CString sBackupPath = REGPATH;
		sBackupPath += "\\Event";
		hKey = CReg.OpenRegKey(&lRes,sBackupPath.GetBuffer(sBackupPath.GetLength() + 1));
		sBackupPath.ReleaseBuffer();
		if(lRes == ERROR_SUCCESS)
		{
			m_strEventLogPath = CReg.FetchStrValue(&lRes,hKey,"TempDbPath");
			CReg.CloseRegKey(hKey);
		}
	}
}

CDumpEventLog::~CDumpEventLog()
{

}

BOOL CDumpEventLog::FileExists(CString strFileName)
{
	HANDLE hDbFile;
	BOOL bRetVal = FALSE;
	CString strMsg;

	hDbFile = CreateFile(
		strFileName,						// File to open
		GENERIC_READ,						// We are reading
		FILE_SHARE_READ,					// Others can read too
		NULL,
		OPEN_EXISTING,						// Open only if exists
		FILE_FLAG_SEQUENTIAL_SCAN,			// We are reading sequentially
		0);									// No template needed

	if(hDbFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDbFile);
		strMsg.Format("\tCDumpEventLog::FileExists(): %s already exists!",strFileName);
		m_Log->WriteLogEntry(strMsg,0);
		bRetVal = TRUE;
	}
	else
		bRetVal = FALSE;

	return(bRetVal);
}

BOOL CDumpEventLog::DumpAndClearEventLog(CString strLogName)
{
	HANDLE hEvtLog;
	CString strEventLog;
	LPVOID lpMsgBuf;
	CString strMsg;

	hEvtLog = OpenEventLog(NULL,strLogName);
	if(hEvtLog)
	{
		strEventLog = m_strEventLogPath + strLogName + ".Evt";
		if(!ClearEventLog(hEvtLog,strEventLog))
		{
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,0,NULL);

			strMsg.Format("\tCDumpEventLog::DumpAndClearEventLog(): %s",(LPTSTR)lpMsgBuf);
			strMsg.TrimRight();
			m_Log->WriteLogEntry(strMsg,0);
			LocalFree(lpMsgBuf);
		}
	}
	else
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);

		strMsg.Format("\tCDumpEventLog::DumpAndClearEventLog(): %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_Log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}

	return(TRUE);
}

void CDumpEventLog::DumpLogs()
{
	CString strMsg;

	POSITION pos;
	for(pos = m_slLogName.GetHeadPosition(); pos != NULL;)
	{
		CString sLogName = m_slLogName.GetNext(pos);
		CString sLogFile = m_strEventLogPath;
		sLogFile += sLogName;
		sLogFile += ".evt";

		strMsg.Format("\tCDumpEventLog::DumpLogs(): Processing %s log",sLogName);
		m_Log->WriteLogEntry(strMsg,0);

		if(!FileExists(sLogFile))
			DumpAndClearEventLog(sLogName);

		CReadEventLogs RE = CReadEventLogs(sLogName,m_strMdbName);
		RE.ReadLog(sLogFile);
	}
}
