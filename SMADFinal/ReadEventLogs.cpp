// ReadEventLogs.cpp: implementation of the ReadEventLogs class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReadEventLogs.h"
#include "EventInsertSet.h"

#define EVENT_BUFFER_SIZE 2048

HANDLE CReadEventLogs::m_hEventLog = NULL;
CDaoDatabase* CReadEventLogs::m_pMdb = NULL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReadEventLogs::CReadEventLogs(CString strInLog,CString sMdbIn)
{
	m_strLogName = strInLog;
	m_strMdbName = sMdbIn;
}

CReadEventLogs::~CReadEventLogs()
{
}

void CReadEventLogs::ReadLog(CString sLogName)
{
	LPTSTR lpBuffer,lpTemp;
	CString strMsg;
	LPVOID lpMsgBuf;
	BOOL bSuccess  = FALSE;
	DWORD dwBytesRead = 0,dwBytesToRead = 0,dwBytesNeeded = 0,
		dwNumEvents = 0,dwAdded = 0,dwOffset = 0,dwLastError = 0;

	m_hEventLog = OpenBackupEventLog(NULL,sLogName);
	if(m_hEventLog != NULL)
	{
		if(GetNumberOfEventLogRecords(m_hEventLog,&dwNumEvents))
		{
			strMsg.Format("\tCReadEventLogs::ReadLog(): Events in %s = %u",
				sLogName,dwNumEvents);
			m_Log->WriteLogEntry(strMsg,0);
		}
		else
		{
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,0,NULL);
			strMsg = (LPTSTR)lpMsgBuf;
			strMsg.TrimRight();
			strMsg = "\tCReadEventLogs::ReadLog(): " + strMsg;
			m_Log->WriteLogEntry(strMsg,0);
			LocalFree(lpMsgBuf);
		}



		lpBuffer = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,EVENT_BUFFER_SIZE);
		dwBytesToRead = EVENT_BUFFER_SIZE;
		bSuccess = ReadEventLog(m_hEventLog,EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
			0,(EVENTLOGRECORD *)lpBuffer,dwBytesToRead,&dwBytesRead,&dwBytesNeeded);
		dwLastError = GetLastError();

		lpTemp = lpBuffer;
		
		if(OpenMdb())
		{
			while(dwBytesRead != 0)
			{
				if(dwOffset < dwBytesRead)
				{
					m_eRec = (EVENTLOGRECORD *)lpTemp;

					m_strAppName = &lpTemp[sizeof(EVENTLOGRECORD)];

					m_strMessageFile = "CategoryMessageFile";
					m_strCategory = GetCategoryText(m_eRec->EventCategory);

					m_strEventText = GetEventMessage();

					UpdateEventMdb(dwAdded);

					dwOffset += m_eRec->Length;
					lpTemp += m_eRec->Length;
				}
				else
				{
					ZeroMemory(lpBuffer,EVENT_BUFFER_SIZE);
					dwBytesToRead = EVENT_BUFFER_SIZE;

					bSuccess = ReadEventLog(m_hEventLog,EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
						0,(EVENTLOGRECORD *)lpBuffer,dwBytesToRead,&dwBytesRead,&dwBytesNeeded);
					dwLastError = GetLastError();
			
					dwOffset = 0;
					lpTemp = lpBuffer;
				}

			}
		}

		if(m_pMdb->IsOpen())
		{
			m_pMdb->m_pWorkspace->CommitTrans();
			m_pMdb->Close();
			delete m_pMdb;
			m_pMdb = NULL;
		}

		LocalFree(lpBuffer);
		CloseEventLog(m_hEventLog);
		if(dwLastError == ERROR_HANDLE_EOF)
			DeleteFile(sLogName);
		else
		{
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,NULL,dwLastError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,0,NULL);
			strMsg = (LPTSTR)lpMsgBuf;
			strMsg.TrimRight();
			strMsg = "\tCReadEventLogs::ReadLog(): " + strMsg;
			m_Log->WriteLogEntry(strMsg,0);
			LocalFree(lpMsgBuf);
		}

		strMsg.Format("\tCReadEventLogs::ReadLog(): %u Events were read from %s and inserted into %s",
			dwAdded,sLogName,m_strMdbName);
		m_Log->WriteLogEntry(strMsg,0);
	}
	else
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);
		strMsg = (LPTSTR)lpMsgBuf;
		strMsg.TrimRight();
		strMsg = "\tCReadEventLogs::ReadLog(): " + strMsg;
		m_Log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}
}

CString CReadEventLogs::GetCategoryText(WORD wCategory)
{
	CString strReturn;
	LPVOID lpMsgBuf;

	if(wCategory != 0)
	{
		CString strRegKey = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\";
		strRegKey += m_strLogName + "\\";
		strRegKey += m_strAppName;

		HKEY hModuleKey;
		LPTSTR lpval;

		DWORD dwSize,dwType = REG_EXPAND_SZ;

		long lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,strRegKey,0,KEY_ALL_ACCESS,&hModuleKey);

		lRes = RegQueryValueEx(hModuleKey,"CategoryMessageFile",NULL,&dwType,NULL,&dwSize);
		lpval = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwSize + 1);
		lRes = RegQueryValueEx(hModuleKey,"CategoryMessageFile",NULL,&dwType,(LPBYTE)lpval,&dwSize);

		RegCloseKey(hModuleKey);

		LPTSTR sLib;
		DWORD dwBufSiz = ExpandEnvironmentStrings(lpval,NULL,0);
		sLib = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwBufSiz);
		ExpandEnvironmentStrings(lpval,sLib,dwBufSiz);
		CString strMsgFile = sLib;
		LocalFree(sLib);


		////////////////////////////////////////////////////////
		HINSTANCE hModule = NULL;
		CString strMsg;

		int iPos = 0;
		do
		{
			iPos = strMsgFile.Find(";");
			if(iPos == -1)
				iPos = strMsgFile.GetLength();

			// Find The Correct Message file there could be more than one
			if(hModule != NULL)
			{
				FreeLibrary(hModule);
				hModule = NULL;
			}

			hModule = LoadLibraryEx(strMsgFile.Left(iPos),NULL,LOAD_LIBRARY_AS_DATAFILE);
			strMsgFile = strMsgFile.Mid(iPos + 1);

		}
		// wCategory must change for parameter files
		while(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |
			FORMAT_MESSAGE_ARGUMENT_ARRAY,hModule,wCategory,MAKELANGID(LANG_NEUTRAL,
			SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,(LPTSTR *)NULL) && iPos > 0);

		if(hModule != NULL)
		{
			FreeLibrary(hModule);
			hModule = NULL;
		}

		strReturn = (LPTSTR)lpMsgBuf;
		if(strReturn != "")
			LocalFree(lpMsgBuf);

		char *s = " ";
		if(strReturn == "")
			strReturn = ltoa((long)wCategory,s,10);

	}
	else
		strReturn = "None";

	return(strReturn);
}

CString CReadEventLogs::GetEventMessage()
{
	LPTSTR lpBuffer = (LPTSTR)m_eRec;
	int iOffset = m_eRec->StringOffset;
	int p,iNumStrings = m_eRec->NumStrings;
	CString strParamMsg;

	for(p = 0; p < iNumStrings; p++)
	{
		strParamMsg = &lpBuffer[iOffset];
		if(strstr(&lpBuffer[iOffset],"%%"))
		{
			m_strMessageFile = "ParameterMessageFile";
			strParamMsg = GetParameterText(strParamMsg);
		}
		m_lpstr[p] = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,strParamMsg.GetLength() + 1);
		strcpy(m_lpstr[p],strParamMsg);
		iOffset += strlen(&lpBuffer[iOffset]) + 1;
	}

	m_strMessageFile = "EventMessageFile";
	m_strEventText = GetEventText(m_eRec->EventID);

	p = 0;
	while(p < iNumStrings)
	{
		LocalFree(m_lpstr[p]);
		p++;
	}

	return(m_strEventText);
}

CString CReadEventLogs::GetEventText(DWORD lEventID)
{
	LPTSTR lpBuffer = (LPTSTR)m_eRec;
	CString strReturn;
	LPVOID lpMsgBuf = NULL;

	CString strRegKey = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\";
	strRegKey += m_strLogName + "\\";
	strRegKey += m_strAppName;

	HKEY hModuleKey;
	LPTSTR lpval;

	DWORD dwSize,dwType = REG_EXPAND_SZ;

	long lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,strRegKey,0,KEY_ALL_ACCESS,&hModuleKey);

	lRes = RegQueryValueEx(hModuleKey,"EventMessageFile",NULL,&dwType,NULL,&dwSize);
	lpval = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwSize + 1);
	lRes = RegQueryValueEx(hModuleKey,"EventMessageFile",NULL,&dwType,(LPBYTE)lpval,&dwSize);

	RegCloseKey(hModuleKey);

	LPTSTR sLib;
	DWORD dwBufSiz = ExpandEnvironmentStrings(lpval,NULL,0);
	sLib = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwBufSiz);
	ExpandEnvironmentStrings(lpval,sLib,dwBufSiz);
	CString strMsgFile = sLib;
	LocalFree(sLib);


	////////////////////////////////////////////////////////
	HINSTANCE hModule = NULL;
	CString strMsg;

	int iPos = 0;
	do
	{
		iPos = strMsgFile.Find(";");
		if(iPos == -1)
			iPos = strMsgFile.GetLength();

		// Find The Correct Message file there could be more than one
		if(hModule != NULL)
		{
			FreeLibrary(hModule);
			hModule = NULL;
		}

		hModule = LoadLibraryEx(strMsgFile.Left(iPos),NULL,LOAD_LIBRARY_AS_DATAFILE);
		strMsgFile = strMsgFile.Mid(iPos + 1);

	}
	// lEventID must change for parameter files
	while(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |
		FORMAT_MESSAGE_ARGUMENT_ARRAY,hModule,lEventID,MAKELANGID(LANG_NEUTRAL,
		SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,(LPTSTR *)m_lpstr) && iPos > 0);
			
	strReturn = (LPTSTR)lpMsgBuf;
	if(lpMsgBuf != NULL)
		LocalFree(lpMsgBuf);

	if(hModule != NULL)
	{
		FreeLibrary(hModule);
		hModule = NULL;
	}

	if(strReturn == "")
	{
		LPTSTR s = " ";

		CString strTempHex;

		strTempHex.Format("%08X",m_eRec->EventID);
		strTempHex.ReleaseBuffer();
		for(int iPos = 0; iPos < 4; iPos++)
		{
			strTempHex.SetAt(iPos,_T('0'));
		}
		strTempHex = "0x" + strTempHex;
		lEventID = strtol(strTempHex,&s,16);

		strReturn.Format("The description for Event ID ( %u ) in Source ( %s ) could not be found. It contains the following insertion string(s):\n",lEventID,m_strAppName);
		for(int p = 0; p < m_eRec->NumStrings; p++)
		{
			strReturn += m_lpstr[p];
			strReturn += "\n";
		}
	}

	DWORD f;
	LPTSTR sTemp = strReturn.GetBuffer(strReturn.GetLength());
	LPTSTR t;

	while(true)
	{
		f = strcspn(sTemp,"\r\n\t");
		t = sTemp + f;
		if(*t != '\0')
			*t = ' ';
		else
			break;
		sTemp = t + 1;
	}

	strReturn.ReleaseBuffer();


	return(strReturn);
}

CString CReadEventLogs::GetParameterText(CString strReturn)
{

	CString strRegKey = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\";
	strRegKey += m_strLogName + "\\";
	strRegKey += m_strAppName;

	HKEY hModuleKey;
	LPTSTR lpval;

	DWORD dwSize,dwType = REG_EXPAND_SZ;

	long lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,strRegKey,0,KEY_ALL_ACCESS,&hModuleKey);

	lRes = RegQueryValueEx(hModuleKey,"ParameterMessageFile",NULL,&dwType,NULL,&dwSize);
	lpval = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwSize + 1);
	lRes = RegQueryValueEx(hModuleKey,"ParameterMessageFile",NULL,&dwType,(LPBYTE)lpval,&dwSize);

	RegCloseKey(hModuleKey);

	LPTSTR sLib;
	DWORD dwBufSiz = ExpandEnvironmentStrings(lpval,NULL,0);
	sLib = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwBufSiz);
	ExpandEnvironmentStrings(lpval,sLib,dwBufSiz);
	CString strMsgFile = sLib;
	LocalFree(sLib);


	////////////////////////////////////////////////////////
	HINSTANCE hModule = NULL;
	CString strMsg;

	CString sTemp = strReturn;
	int iPos = sTemp.Find("%");
	while(iPos != -1)
	{
		sTemp = sTemp.Mid(iPos + 1);
		iPos = sTemp.Find("%");
	}

	char *s = " ";
	DWORD dwErrorID = strtol(sTemp,&s,10);
	LPVOID sErrorText;

	// the logic is not quite right must fix
	if(strMsgFile.GetLength() != 0)
	{
		int iPos = 0;
		do
		{
			iPos = strMsgFile.Find(";");
			if(iPos == -1)
				iPos = strMsgFile.GetLength();

			// Find The Correct Message file there could be more than one
			if(hModule != NULL)
			{
				FreeLibrary(hModule);
				hModule = NULL;
			}

			hModule = LoadLibraryEx(strMsgFile.Left(iPos),NULL,LOAD_LIBRARY_AS_DATAFILE);
			strMsgFile = strMsgFile.Mid(iPos + 1);
		}
		// lEventID must change for parameter files
		while(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |
			FORMAT_MESSAGE_ARGUMENT_ARRAY,hModule,dwErrorID,MAKELANGID(LANG_NEUTRAL,
			SUBLANG_DEFAULT),(LPTSTR)&sErrorText,0,(LPTSTR *)NULL) && iPos > 0);
	}

	strReturn = (LPTSTR)sErrorText;
	LocalFree(sErrorText);
	return(strReturn);
}

void CReadEventLogs::UpdateEventMdb(DWORD & dwCount)
{
	CEventInsertSet *pSet;
	CString strMsg;
	LPTSTR lpBuffer = (LPTSTR)m_eRec;

	CTime tTemp = CTime(m_eRec->TimeWritten);
	CString sEventTime = tTemp.Format("%H:%M:%S");
	CString sEventDate = tTemp.Format("%Y-%m-%d");

	LPTSTR sTempEvent = m_strEventText.GetBuffer(m_strEventText.GetLength());

	int il = m_strEventText.GetLength();
	try
	{
//		m_pMdb = new CDaoDatabase;
//		m_pMdb->Open(m_strMdbName,false,false,"");
//		m_pMdb->m_pWorkspace->BeginTrans();
		pSet = new CEventInsertSet(m_pMdb);
		pSet->m_sDefaultSQL = m_strLogName;

		pSet->Open();
		pSet->AddNew();

		pSet->m_Category = m_strCategory;
		pSet->m_Computer = &lpBuffer[sizeof(EVENTLOGRECORD) + strlen(&lpBuffer[sizeof(EVENTLOGRECORD)]) + 1];

		pSet->m_Description = m_strEventText;


		CString sHexData,sASCIIData,sTest;
		BYTE bPos;
		
		LPTSTR sTemp = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,m_eRec->DataLength + 1);
		CopyMemory(sTemp,lpBuffer + m_eRec->DataOffset,m_eRec->DataLength);
		for(DWORD dwPos = 0; dwPos < m_eRec->DataLength; dwPos++)
		{
			bPos = *(sTemp + dwPos);
			sTest.Format("%02X",bPos);
			sHexData += sTest;
			sHexData += " ";
			if(bPos == 0)
				sASCIIData += ".";
			else
				sASCIIData += bPos;
		}
		LocalFree(sTemp);

		pSet->m_HexData = sHexData;
		pSet->m_ASCIIData = sASCIIData;

		pSet->m_EventDate.ParseDateTime(sEventDate);

		CString strTempHex;

		strTempHex.Format("%08X",m_eRec->EventID);
		strTempHex.ReleaseBuffer();
		for(int iPos = 0; iPos < 4; iPos++)
		{
			strTempHex.SetAt(iPos,_T('0'));
		}
		strTempHex = "0x" + strTempHex;
		char *s = " ";
		pSet->m_EventID = strtol(strTempHex,&s,16);

		pSet->m_EventTime.ParseDateTime(sEventTime);
		pSet->m_Source = m_strAppName;

		switch(m_eRec->EventType)
		{
			case EVENTLOG_ERROR_TYPE:
				pSet->m_Type = "Error event";
				break;
			case EVENTLOG_WARNING_TYPE:
				pSet->m_Type = "Warning event";
				break;
			case EVENTLOG_INFORMATION_TYPE:
				pSet->m_Type = "Information event";
				break;
			case EVENTLOG_AUDIT_SUCCESS:
				pSet->m_Type = "Success Audit event";
				break;
			case EVENTLOG_AUDIT_FAILURE:
				pSet->m_Type = "Failure Audit event";
				break;
			default:
				pSet->m_Type = "Unknown event type";
		}

		if(m_eRec->UserSidLength)
		{
			LPTSTR strUser;
			SID_NAME_USE eUse;
			PSID pSidUser;
			LPTSTR DomainName = NULL;
			DWORD dwDomainName = 0,dwSidSize = 0;

			pSidUser = (PSID)LocalAlloc(LMEM_ZEROINIT,m_eRec->UserSidLength);
			BOOL bC = CopySid(m_eRec->UserSidLength,pSidUser,(SID *)(lpBuffer + m_eRec->UserSidOffset));

			SECURITY_DESCRIPTOR PS;
			BOOL lpB;
			GetSecurityDescriptorOwner(&PS,&pSidUser,&lpB);
			strUser = (LPTSTR)&PS.Owner;

			LookupAccountSid(NULL,pSidUser,strUser,&dwSidSize,DomainName,&dwDomainName,&eUse);
			DWORD dwLerror = GetLastError();

			//			if(dwLerror == ERROR_INSUFFICIENT_BUFFER ERROR_NONE_MAPPED)
			if(dwLerror == ERROR_INSUFFICIENT_BUFFER)
			{
				strUser = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwSidSize);
				DomainName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwDomainName);
				LookupAccountSid(NULL,pSidUser,strUser,&dwSidSize,DomainName,&dwDomainName,&eUse);
				
				pSet->m_User += DomainName;
				if(pSet->m_User != "")
					pSet->m_User += "\\";
				pSet->m_User += strUser;
				
				LocalFree(DomainName);
				LocalFree(strUser);
			}
			else
			{
				if(dwLerror == ERROR_NONE_MAPPED)
				{
					DWORD dwSidStringSize = 0;
					LPTSTR szSidString;

					ConvertSIDToString(pSidUser,NULL,&dwSidStringSize);
					dwLerror = GetLastError();

					if(dwLerror = ERROR_INSUFFICIENT_BUFFER)
					{
						szSidString = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwSidStringSize + 1);
						ConvertSIDToString(pSidUser,szSidString,&dwSidStringSize);
						pSet->m_User = szSidString;
						LocalFree(szSidString);
					}
				}
				else
					pSet->m_User = "Unknown User";
			}
			LocalFree(pSidUser);
		}
		else
			pSet->m_User = "N/A";

		pSet->Update();
		pSet->Close();
//		m_pMdb->m_pWorkspace->CommitTrans();
//		m_pMdb->Close();
		delete pSet;
		dwCount++;
	}
	catch(CDaoException *e)
	{
		strMsg.Format("\tCReadEventLogs::UpdateEventMdb() failed: %s %u\n\t\t\t%s",
			e->m_pErrorInfo->m_strSource,
			e->m_pErrorInfo->m_lErrorCode,
			e->m_pErrorInfo->m_strDescription);
		m_Log->WriteLogEntry(strMsg,0);

		if(pSet->IsOpen())
			pSet->Close();

		delete pSet;

		e->Delete();
	}
}

BOOL CReadEventLogs::ConvertSIDToString(PSID pSid,LPTSTR TextualSid,LPDWORD dwBufferLen)
{ 
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;


	// Test the SID for validity
    if(!IsValidSid(pSid)) return FALSE;

    psia=GetSidIdentifierAuthority(pSid);

    dwSubAuthorities=*GetSidSubAuthorityCount(pSid);

    // 
    // compute buffer length
    // S-SID_REVISION- + identifierauthority- + subauthorities- + NULL
    // 
    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

    // 
    // check provided buffer length.
    // If not large enough, indicate proper size and setlasterror
    // 
    if (*dwBufferLen < dwSidSize)
    {
        *dwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    // 
    // prepare S-SID_REVISION-
    // 
    dwSidSize=wsprintf(TextualSid, TEXT("S-%lu-"), dwSidRev );

    // 
    // prepare SidIdentifierAuthority
    // 
    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

    // 
    // loop through SidSubAuthorities
    // 
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        dwSidSize+=wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    return TRUE;
}

bool CReadEventLogs::OpenMdb()
{
	CString strMsg;

	try
	{
		m_pMdb = new CDaoDatabase;
		m_pMdb->Open(m_strMdbName,false,false,"");
		m_pMdb->m_pWorkspace->BeginTrans();
		return true;
	}
	catch(CDaoException* e)
	{
		strMsg.Format("\tCReadEventLogs::OpenMdb() failed: %s %u\n\t\t\t%s",
			e->m_pErrorInfo->m_strSource,
			e->m_pErrorInfo->m_lErrorCode,
			e->m_pErrorInfo->m_strDescription);
		m_Log->WriteLogEntry(strMsg,0);
		
		e->Delete();
	}
	return false;
}