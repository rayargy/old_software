// SMADErrorLog.cpp: implementation of the CSMADErrorLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SMADErrorLog.h"
#include "registry.h"

#define REGKEY "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSMADErrorLog* CSMADErrorLog::c_instance = NULL;
FILE* CSMADErrorLog::m_pLog = NULL;
DWORD CSMADErrorLog::m_dwDebugLevel = 0;

CSMADErrorLog::CSMADErrorLog()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;

	hKey = Reg.OpenRegKey(&lRes,REGKEY);
	if(lRes == ERROR_SUCCESS)
	{
		m_sSMADLog = Reg.FetchStrValue(&lRes,hKey,"SMADLog");
		m_dwDebugLevel = Reg.FetchDWValue(&lRes,hKey,"DebugLevel");
		Reg.CloseRegKey(hKey);
	}

	if(m_sSMADLog != "")
		m_pLog = fopen(m_sSMADLog.GetBuffer(m_sSMADLog.GetLength()),"a");
	else
		m_pLog = fopen("C:\\SMADDbg.Log","a");
}

CSMADErrorLog::~CSMADErrorLog()
{
	fclose(m_pLog);
}

CSMADErrorLog* CSMADErrorLog::GetInstance()
{
	if(c_instance == NULL)
		c_instance = new CSMADErrorLog;
	return c_instance;
}

void CSMADErrorLog::DestroyInstance()
{
	delete c_instance;
	c_instance = NULL;
}

void CSMADErrorLog::WriteLogEntry(CString sLogEntry, DWORD dwDebug)
{
	CString sMessage;
	if(dwDebug <= m_dwDebugLevel)
	{
		DWORD dwThreadID = GetCurrentThreadId();
		sMessage.Format ( "%u\tDebug:%u\t%s\n", dwThreadID, dwDebug, sLogEntry);
		fprintf ( m_pLog, "%s" , sMessage );
		fflush(m_pLog);
	}
}