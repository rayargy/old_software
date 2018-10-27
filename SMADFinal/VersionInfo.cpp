// VersionInfo.cpp: implementation of the CVersionInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VersionInfo.h"
#include "winver.h"
#include "registry.h"
#include "VersionSet.h"

#define FILELISTLOC "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVersionInfo::CVersionInfo()
{
	CRegistry Reg;
	HKEY hKey;
	DWORD dwValType = REG_EXPAND_SZ,dwValSize = 0;
	LPBYTE lpBuf;
	long lRes = 0;
	int iEnd = 0,iLen = 0;

	m_Log->GetInstance();
	m_Log->WriteLogEntry("CVersionInfo::CVersionInfo() Constructor",0);

	hKey = Reg.OpenRegKey(&lRes,FILELISTLOC);
	if(lRes == ERROR_SUCCESS)
	{
		LPTSTR lpTemp;

		lRes = Reg.QueryRegValue(hKey,"FileVersion",&dwValType,NULL,&dwValSize);
		lpBuf = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwValSize + 1);
		lRes = Reg.QueryRegValue(hKey,"FileVersion",&dwValType,lpBuf,&dwValSize);
		DWORD dwRequiredSize = ExpandEnvironmentStrings((LPCTSTR)lpBuf,NULL,0);
		lpTemp = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwRequiredSize);
		dwRequiredSize = ExpandEnvironmentStrings((LPCTSTR)lpBuf,lpTemp,dwRequiredSize);
		LocalFree(lpBuf);

		CString sFile = lpTemp;
		LocalFree(lpTemp);

		for(int iPos = sFile.Find(';'); iPos != -1;iPos = sFile.Find(';'))
		{
			m_sVersionList.AddHead(sFile.Left(iPos));
			sFile = sFile.Mid(iPos + 1);
		}
		if(sFile != "")
			m_sVersionList.AddHead(sFile);


		lRes = Reg.QueryRegValue(hKey,"VersionMdb",&dwValType,NULL,&dwValSize);
		lpBuf = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwValSize + 1);
		lRes = Reg.QueryRegValue(hKey,"VersionMdb",&dwValType,lpBuf,&dwValSize);

		dwRequiredSize = ExpandEnvironmentStrings((LPCTSTR)lpBuf,NULL,0);
		lpTemp = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwRequiredSize);
		dwRequiredSize = ExpandEnvironmentStrings((LPCTSTR)lpBuf,lpTemp,dwRequiredSize);
		LocalFree(lpBuf);

		m_sMdbPath = lpTemp;
		LocalFree(lpTemp);

		Reg.CloseRegKey(hKey);
	}
}

CVersionInfo::~CVersionInfo()
{

}

void CVersionInfo::GetFileVersion()
{
	DWORD dwTest = 0,dwHandle = 0;
	UINT uiSubLen = 0;
	LPSTR lpBuf;
    static LPTSTR   lpszBuff;
	CString strMsg;

	DWORD dwSize = GetFileVersionInfoSize
		(m_sFileToCheck.GetBuffer(m_sFileToCheck.GetLength()),&dwTest);
	lpBuf = (LPSTR)LocalAlloc(LMEM_ZEROINIT,dwSize);
	BOOL bSuccess = GetFileVersionInfo
		(m_sFileToCheck.GetBuffer(m_sFileToCheck.GetLength()),dwHandle,dwSize,lpBuf);

    lpszBuff = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, dwSize * sizeof(TCHAR) );

	bSuccess = FilerGetVersion(m_sFileToCheck.GetBuffer(m_sFileToCheck.GetLength()),dwSize,lpszBuff);

	bSuccess = GetFileInfo();

	if(bSuccess)
	{
		CDaoDatabase* dbVer;
		CVersionSet* pVersionSet;
		try
		{
			AfxDaoInit();
			dbVer = new CDaoDatabase;
			dbVer->Open(m_sMdbPath,false,false,_T(""));
			pVersionSet = new CVersionSet(dbVer);
			pVersionSet->Open();
			dbVer->m_pWorkspace->BeginTrans();
			pVersionSet->AddNew();
			pVersionSet->m_LastModDate = m_FileDateTime;
			pVersionSet->m_FileSize = m_sFileSize;
			pVersionSet->m_ProductName = gVKArray[0].szValue;
			pVersionSet->m_ProductVersion = gVKArray[1].szValue;
			pVersionSet->m_OriginalFilename = gVKArray[2].szValue;
			if(pVersionSet->m_OriginalFilename == "")
				pVersionSet->m_OriginalFilename = m_sFileToCheck;
			pVersionSet->m_FileDescription = gVKArray[3].szValue;
			pVersionSet->m_FileVersion = gVKArray[4].szValue;
			pVersionSet->m_CompanyName = gVKArray[5].szValue;
			pVersionSet->m_LegalCopyright = gVKArray[6].szValue;
			pVersionSet->m_LegalTrademarks = gVKArray[7].szValue;
			pVersionSet->m_InternalName = gVKArray[8].szValue;
			pVersionSet->m_PrivateBuild = gVKArray[9].szValue;
			pVersionSet->m_SpecialBuild = gVKArray[10].szValue;
			pVersionSet->m_Comments = gVKArray[11].szValue;
			pVersionSet->Update();
			dbVer->m_pWorkspace->CommitTrans();
			pVersionSet->Close();
			delete pVersionSet;
			dbVer->Close();
			delete dbVer;
			AfxDaoTerm();
		}
		catch(CDaoException *e)
		{
			strMsg.Format("\tCVersionInfo::GetFileVersion() failed: %s %u\n\t\t\t%s",
				e->m_pErrorInfo->m_lErrorCode,
				e->m_pErrorInfo->m_strSource,
				e->m_pErrorInfo->m_strDescription);
			m_Log->WriteLogEntry(strMsg,0);

			e->Delete();
		}
	}

	LocalFree(lpszBuff);
	LocalFree(lpBuf);
}

void CVersionInfo::GetFileVersions()
{
	CString strMsg;
	POSITION iPos;

	iPos = m_sVersionList.GetHeadPosition();
	while(iPos != NULL)
	{
		m_sFileToCheck = m_sVersionList.GetNext(iPos);
		strMsg.Format("\tCVersionInfo::GetFileVersions(): Processing %s",m_sFileToCheck);
		m_Log->WriteLogEntry(strMsg,0);
		GetFileVersion();
	}

}

BOOL CVersionInfo::FilerGetVersion(LPTSTR lpszFileName, DWORD dwBuffSize, LPTSTR szBuff)
{
    //
    // NUM_VERSION_INFO_KEYS in GLOBALS.H should be set to the number of entries in
    //   VersionKeys[].
    //

    CONST static TCHAR   *VersionKeys[] = {
            TEXT("ProductName"),
            TEXT("ProductVersion"),
            TEXT("OriginalFilename"),
            TEXT("FileDescription"),
            TEXT("FileVersion"),
            TEXT("CompanyName"),
            TEXT("LegalCopyright"),
            TEXT("LegalTrademarks"),
            TEXT("InternalName"),
            TEXT("PrivateBuild"),
            TEXT("SpecialBuild"),
            TEXT("Comments")
    };

    static TCHAR szNull[1] = TEXT("");
    LPVOID  lpInfo;
    UINT   cch;
    UINT    i;
    TCHAR   key[80];
    TCHAR   lpBuffer[10];
	CString VSList;
	bool bVersionAvail = false;

   

	bVersionAvail = GetFileVersionInfo(lpszFileName, 0, dwBuffSize, (LPVOID)szBuff );

	if(bVersionAvail)
	{
		DWORD p;
		VerQueryValue(szBuff, "\\VarFileInfo\\Translation", &lpInfo, &cch);

		memcpy(&p,lpInfo,sizeof(DWORD));
		wsprintf(lpBuffer, "%08X", p);
		CString lpTemp = lpBuffer;
		CString lp1 = lpTemp.Mid(4);
		lp1 += lpTemp.Left(4);

		wsprintf(lpBuffer,"%s",lp1.GetBuffer(lp1.GetLength()));
	}

    for (i = 0; i < NUM_VERSION_INFO_KEYS; i++) {
        lstrcpy(key, VERSION_INFO_KEY_ROOT);
        lstrcat(key, lpBuffer);
        lstrcat(key, "\\");
        lstrcat(key, VersionKeys[i]);

        //
        // If version info exists, and the key query is successful, add
        //  the value.  Otherwise, the value for the key is NULL.
        //
        if( dwBuffSize && VerQueryValue(szBuff, key, &lpInfo, &cch) )
            gVKArray[i].szValue = (TCHAR *)lpInfo;
        else
		{
			LANGID LangID = GetUserDefaultLangID();
			UINT SubLangID = GetACP();
			CString lid;
			lid.Format("%04X%04X",LangID,SubLangID);
	        lstrcpy(key, VERSION_INFO_KEY_ROOT);
			lstrcat(key, lid.GetBuffer(lid.GetLength()));
			lstrcat(key, "\\");
			lstrcat(key, VersionKeys[i]);
			if( dwBuffSize && VerQueryValue(szBuff, key, &lpInfo, &cch) )
				gVKArray[i].szValue = (TCHAR *)lpInfo;
			else
				gVKArray[i].szValue = szNull;
		}

    }

    return TRUE;
}

bool CVersionInfo::GetFileInfo()
{
	HANDLE hFile;
	BY_HANDLE_FILE_INFORMATION FileInfo;
	__int64 i64FileSize = 0,i64Temp = 0;
	char sint[21]; // max places in int64 plus sign and null
	CString sTempFileName = m_sFileToCheck;
	bool bVal = false;
	CString strMsg;
	LPVOID lpMsgBuf;

	hFile = CreateFile(
		sTempFileName,						// File to open
		GENERIC_READ,						// We are reading
		FILE_SHARE_READ,					// Others can read too
		NULL,
		OPEN_EXISTING,						// Open only if exists
		FILE_FLAG_SEQUENTIAL_SCAN,			// We are reading sequentially
		0);									// No template needed

	if(hFile != INVALID_HANDLE_VALUE)
	{
		GetFileInformationByHandle(hFile,&FileInfo);
		m_FileDateTime = COleDateTime(FileInfo.ftLastWriteTime);
		i64FileSize = FileInfo.nFileSizeHigh;
		i64FileSize = i64FileSize << 32;
		i64FileSize += FileInfo.nFileSizeLow;
		sprintf(sint,"%I64d",i64FileSize);
		m_sFileSize = sint;
		CloseHandle(hFile);
		bVal = true;
	}
	else
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);

		strMsg.Format("\tCVersionInfo::GetFileInfo(): %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_Log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}

	return bVal;
}
