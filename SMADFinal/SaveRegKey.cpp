// SaveRegKey.cpp: implementation of the CSaveRegKey class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SaveRegKey.h"
#include "registry.h"
#include "SMADErrorLog.h"

#define REGKEY "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSaveRegKey::CSaveRegKey()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;

	m_Log->GetInstance();
	m_Log->WriteLogEntry("CSaveRegKey::CSaveRegKey() Constructor",0);

	hKey = Reg.OpenRegKey(&lRes,REGKEY);
	if(lRes == ERROR_SUCCESS)
	{
		m_sRegFile = Reg.FetchStrValue(&lRes,hKey,"RegFile");
	}
}

CSaveRegKey::~CSaveRegKey()
{

}

void CSaveRegKey::SaveKey(CString sKeyToSave)
{
	long lRes = 0;
	HKEY hKey;
	CRegistry Reg;
	CString strMsg;
	LPVOID lpMsgBuf;

	// adjustment of privileges should probably be moved into a global area
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;
   
	if(!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES,&hToken ))
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);

		strMsg.Format("\tCSaveRegKey::SaveKey(): %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_Log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}

	if(!LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &luid))
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);

		strMsg.Format("\tCSaveRegKey::SaveKey(): %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_Log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}

	tp.PrivilegeCount           = 1;
	tp.Privileges[0].Luid       = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tp,
		sizeof(TOKEN_PRIVILEGES),NULL, NULL );
	if (DWORD dw = GetLastError() != ERROR_SUCCESS)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);

		strMsg.Format("\tCSaveRegKey::SaveKey(): %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_Log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}

	hKey = Reg.OpenRegKey(&lRes,sKeyToSave.GetBuffer(sKeyToSave.GetLength()));
	if(lRes == ERROR_SUCCESS)
	{
		lRes = RegSaveKey(hKey,m_sRegFile,(LPSECURITY_ATTRIBUTES)NULL);
		if(lRes != ERROR_SUCCESS)
		{
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,0,NULL);

			strMsg.Format("\tCSaveRegKey::SaveKey(): %s",(LPTSTR)lpMsgBuf);
			strMsg.TrimRight();
			m_Log->WriteLogEntry(strMsg,0);
			LocalFree(lpMsgBuf);
		}
		Reg.CloseRegKey(hKey);
	}

	// Revoke all privileges this process holds (including backup)
	AdjustTokenPrivileges( hToken, TRUE, NULL, 0, NULL, NULL);

	// close handle to process token
	CloseHandle(hToken);
}
