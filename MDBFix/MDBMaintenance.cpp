// MDBMaintenance.cpp: implementation of the CMDBMaintenance class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MDBMaintenance.h"
#include "registry.h"
//#include <dbdaoerr.h>

CDaoDatabase* CMDBMaintenance::m_daodb = NULL;
CDaoTableDef* CMDBMaintenance::m_DaoTableDef = NULL;
CDaoQueryDef* CMDBMaintenance::m_DaoQueryDef = NULL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMDBMaintenance::CMDBMaintenance(CString sDbKey)
{
	CRegistry CReg;
	HKEY hDb;
	LPTSTR szDbKey;
	long lRes = 0;

	GetDaoPtr();
	CDaoTableDef m_DaoTableDef(m_daodb);

	m_strDbKey = sDbKey;
	szDbKey = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,sDbKey.GetLength() + 1);
	strcpy(szDbKey,sDbKey);
	hDb = CReg.OpenRegKey(&lRes,szDbKey,HKEY_LOCAL_MACHINE);
	LocalFree(szDbKey);

	if(!GetRegValue(hDb,m_strTempDb,"TempDb"))
		m_strTempDb = "TransTemp.mdb";
	if(!GetRegValue(hDb,m_strTempDbPath,"TempDbPath"))
		m_strTempDbPath = "C:\\LogFiles\\";
	if(!GetRegValue(hDb,m_strBackupPath,"BackupPath"))
		m_strBackupPath = "C:\\LogFiles\\Backup\\";

	CReg.CloseRegKey(hDb);

	hDb = CReg.OpenRegKey(&lRes,
		"SOFTWARE\\CTRSystems\\PARCS-CS\\Transactions",HKEY_LOCAL_MACHINE);
	if(!GetRegValue(hDb,m_strDestinationDb,"BufferLoc"))
		m_strDestinationDb = "C:\\LogFiles\\TransBuff.mdb";
	CReg.CloseRegKey(hDb);
}

CMDBMaintenance::~CMDBMaintenance()
{

}

CDaoDatabase* CMDBMaintenance::GetDaoPtr()
{
	if(m_daodb == NULL)
		m_daodb = new CDaoDatabase;
	AfxDaoInit();
	return m_daodb;
}

void CMDBMaintenance::DestroyDaoPtr()
{
	if(m_daodb != NULL)
		delete m_daodb;
	AfxDaoTerm();
}

BOOL CMDBMaintenance::GetRegValue(HKEY hTempDb,CString &sValue,CString sValueName)
{
	CRegistry CReg;
	long lRes = 0;
	LPTSTR szValueName;

	szValueName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,sValueName.GetLength() + 1);
	strcpy(szValueName,sValueName);
	sValue = CReg.FetchStrValue(&lRes,hTempDb,szValueName);
	if(lRes) return(false);
	return(true);

}

BOOL CMDBMaintenance::InitMdb(long lDaoError)
{
	BOOL bRetVal = FALSE;

	CString sDbRegPath = "SOFTWARE\\CTRSystems\\PARCS-CS\\Access\\TransBuff";

	// Backup the corrupt database if it exists!
	// if it doesn't exist it doesn't matter because it will be recreated!
	CopyFile(m_strDestinationDb,
		m_strBackupPath + CTime::GetCurrentTime().Format("%Y%m%d%H%M%S") + "TB.mdb",false);

	// we will attempt to repair the database once, if the RepairDatabase member
	// function still returns unrecognizable database format then the database
	// is unrepairable. In this case, the database will be created from scratch.
	for(int iAttempts = 1; iAttempts < 6; iAttempts++)
	{
		if(DBDAOERR(lDaoError) == E_DAO_FileNotFound ||
			DBDAOERR(lDaoError) == E_DAO_InvalidDatabaseM)
		{
			// attempt repair
			bRetVal = RepairMdb(&lDaoError);
			if(!bRetVal)
				bRetVal = CreateMdb(&lDaoError);
			if(bRetVal) break;
		}
	}
	// if not bRetVal report dao error;
	return(bRetVal);
}

BOOL CMDBMaintenance::RepairMdb(long *lDaoErr)
{
	CString strMsg,sTemp;
	LPTSTR szMessage;

	sTemp = m_strTempDbPath + m_strTempDb;

	// if possible, we will backup the database before repairing!
	// if we have made it to here, then the file move could not fail
	// from a file sharing violation because the corrupt database is inaccessable!
	MoveFileEx(m_strDestinationDb,sTemp,
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

	try
	{
		m_daodb->m_pWorkspace->RepairDatabase(sTemp);
	}
	catch(CDaoException *e)
	{
		strMsg  = "Repair Failed: " + e->m_pErrorInfo->m_strSource \
				+ "\t" + e->m_pErrorInfo->m_strDescription;
		szMessage = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,strMsg.GetLength() + 10);
		sprintf(szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
		memcpy(lDaoErr,&e->m_pErrorInfo->m_lErrorCode,sizeof(long));
//				DebugWriteMessage ( szMessage, 1 );
		LocalFree(szMessage);
		e->Delete();
		return(false);
	}

	try
	{
		m_daodb->m_pWorkspace->CompactDatabase(sTemp,
			m_strDestinationDb,dbLangGeneral);
	}
	catch(CDaoException *e)
	{
		strMsg  = "1st Compact " + e->m_pErrorInfo->m_strSource \
				+ "\t" + e->m_pErrorInfo->m_strDescription;
		szMessage = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,strMsg.GetLength() + 10);
		sprintf(szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
		memcpy(lDaoErr,&e->m_pErrorInfo->m_lErrorCode,sizeof(long));
//				DebugWriteMessage ( szMessage, 1 );
		LocalFree(szMessage);
		e->Delete();
		return(false);
	}
	DeleteFile(sTemp);
	return(true);
}

BOOL CMDBMaintenance::CreateMdb(long *lDaoErr)
{
	BOOL bDbExists = FALSE;

	do
	{
		try
		{
			m_daodb->Create(m_strDestinationDb,dbLangGeneral,0);
			bDbExists = TRUE;
		}
		catch(CDaoException *e)
		{
			if(e->m_pErrorInfo->m_lErrorCode == 3204)
				if(!DeleteFile(m_strDestinationDb))
					DWORD dwError = GetLastError();
			e->Delete();
		}
	} while(!bDbExists);
	if(bDbExists)
	{
		if(!CreateTableDef()) DeleteFile(m_strDestinationDb);
		if(!CreateQueryDef()) DeleteFile(m_strDestinationDb);
	}
	return(bDbExists);
}

BOOL CMDBMaintenance::CreateTableDef()
{
	CDaoTableDef t(m_daodb);
	m_DaoTableDef = &t;
	BOOL bRetVal = TRUE;
	long lRes = 0;
	DWORD dwSubKeys = 0, dwMaxSubKeyLen = 0, dwValues = 0,
		dwMaxValueNameLen = 0, dwMaxValueLen = 0, dwMSubKeyLen = 0,
		dwKeyCount = 0;
	CRegistry CReg;
	HKEY hTableKey;
	LPTSTR sTableName,sTableKey;
	FILETIME ft;

	sTableKey = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,
		m_strDbKey.GetLength() + strlen("\\TableDefs") + 1);
	strcpy(sTableKey,m_strDbKey);
	strcat(sTableKey,"\\TableDefs");
	hTableKey = CReg.OpenRegKey(&lRes,sTableKey,HKEY_LOCAL_MACHINE);
	LocalFree(sTableKey);
	if(CReg.m_dwDisposition == REG_CREATED_NEW_KEY) return(FALSE);

	CReg.QueryRegKey(hTableKey,&dwSubKeys,&dwMaxSubKeyLen,&dwValues,
		&dwMaxValueNameLen,&dwMaxValueLen);

	sTableName = (char*)LocalAlloc(LMEM_ZEROINIT,dwMaxSubKeyLen + 1);
	dwMSubKeyLen = dwMaxSubKeyLen + 1;
	while(dwKeyCount < dwSubKeys)
	{
		lRes = CReg.EnumRegKey(hTableKey,sTableName,&dwMSubKeyLen,&ft,dwKeyCount);
		try
		{
			m_DaoTableDef->Create(sTableName,0,sTableName);
		}
		catch(CDaoException *e)
		{
			bRetVal = FALSE;
			e->Delete();
			break;
		}
		if(CreateColumns(hTableKey,sTableName))
			dwKeyCount++;
		else
		{
			bRetVal = FALSE;
			break;
		}
		dwMSubKeyLen = dwMaxSubKeyLen + 1;
		m_DaoTableDef->Close();
	}

	LocalFree(sTableName);
	CReg.CloseRegKey(hTableKey);
	return(bRetVal);

}

BOOL CMDBMaintenance::CreateQueryDef()
{
	CDaoQueryDef q(m_daodb);
	m_DaoQueryDef = &q;
	CString sDbQueryKey;
	long lRes = 0;
	DWORD dwSubKeys = 0, dwMaxSubKeyLen = 0, dwValues = 0,
		dwMaxValueNameLen = 0, dwMaxValueLen = 0, dwMSubKeyLen = 0,
		dwKeyCount = 0;
	CRegistry CReg;
	HKEY hQueryKey;
	LPTSTR sQueryName,sQueryKey;
	FILETIME ft;

	sQueryKey = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,m_strDbKey.GetLength() + strlen("\\QueryDefs") + 1);
	strcpy(sQueryKey,m_strDbKey);
	strcat(sQueryKey,"\\QueryDefs");
	hQueryKey = CReg.OpenRegKey(&lRes,sQueryKey,HKEY_LOCAL_MACHINE);
	LocalFree(sQueryKey);
	if(CReg.m_dwDisposition == REG_CREATED_NEW_KEY) return(FALSE);

	CReg.QueryRegKey(hQueryKey,&dwSubKeys,&dwMaxSubKeyLen,&dwValues,
		&dwMaxValueNameLen,&dwMaxValueLen);

	sQueryName = (char*)LocalAlloc(LMEM_ZEROINIT,dwMaxSubKeyLen + 1);
	dwMSubKeyLen = dwMaxSubKeyLen + 1;
	while(dwKeyCount < dwSubKeys)
	{
		lRes = CReg.EnumRegKey(hQueryKey,sQueryName,&dwMSubKeyLen,&ft,dwKeyCount);
		CreateQuery(hQueryKey,sQueryName);
		dwKeyCount++;
		dwMSubKeyLen = dwMaxSubKeyLen + 1;
	}
	LocalFree(sQueryName);
		
	CReg.CloseRegKey(hQueryKey);
	return(true);

}

BOOL CMDBMaintenance::CreateColumns(HKEY hDbKey, LPTSTR szTableName)
{
	LPTSTR sColumnName;
	HKEY hTableName;
	CRegistry CReg;
	long lRes = 0;
	DWORD dwcbName = 0,dwType = 0,dwcbValue = 0,dwIndex = 0,dwSubKeys = 0,
		dwMaxSubKeyLen = 0,dwValues = 0,dwMaxValueNameLen = 0,dwMaxValueLen = 0,
		dwMValueLen = 0,dwMValueNameLen = 0,dwMSubKeyLen = 0,dwKeyCount = 0;
	FILETIME ft;
	CDaoFieldInfo fInfo;
	CDaoIndexInfo iInfo;
	CDaoIndexFieldInfo fiInfo;

	hTableName = CReg.OpenRegKey(&lRes,szTableName,hDbKey);

	CReg.QueryRegKey(hTableName,&dwSubKeys,&dwMaxSubKeyLen,&dwValues,
		&dwMaxValueNameLen,&dwMaxValueLen);

	sColumnName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwMaxSubKeyLen + 1);
	dwMSubKeyLen = dwMaxSubKeyLen + 1;
	while(dwKeyCount < dwSubKeys)
	{
		lRes = CReg.EnumRegKey(hTableName,sColumnName,&dwMSubKeyLen,&ft,dwKeyCount);
		CRegistry RegColumn;
		HKEY hColumnKey = RegColumn.OpenRegKey(&lRes,sColumnName,hTableName);
		GetColumnInfo(hColumnKey,fInfo);
		try
		{
			m_DaoTableDef->CreateField(fInfo);
		}
		catch(CDaoException *e)
		{
			e->Delete();
		}
		CString sPrimary = RegColumn.FetchStrValue(&lRes,hColumnKey,"PrimaryKey");
		if(sPrimary == "1")
		{
			GetIndexInfo(hColumnKey,iInfo,fiInfo);
			try
			{
				m_DaoTableDef->CreateIndex(iInfo);
			}
			catch(CDaoException *e)
			{
				e->Delete();
			}
		}

		// get field type is a primary key?
		// create field index
		RegColumn.CloseRegKey(hColumnKey);
		dwKeyCount++;
		dwMSubKeyLen = dwMaxSubKeyLen + 1;
	}
	try
	{
		m_DaoTableDef->Append();
	}
	catch(CDaoException *e)
	{
		e->Delete();
	}
	LocalFree(sColumnName);

	CReg.CloseRegKey(hTableName);

	return(true);

}

BOOL CMDBMaintenance::CreateQuery(HKEY hQKey, LPTSTR sQName)
{
	CRegistry CReg;
	CString sSql;
	HKEY hQDefKey;
	long lRes = 0;
	DWORD dwType = 0,dwData = 0,dwPos = 0;
	LPBYTE lpData;

	hQDefKey = CReg.OpenRegKey(&lRes,sQName,hQKey);
	if(lRes == ERROR_SUCCESS)
	{
		CReg.QueryRegValue(hQDefKey,"SQL",&dwType,NULL,&dwData);
		lpData = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwData + 1);
		CReg.QueryRegValue(hQDefKey,"SQL",&dwType,lpData,&dwData);
		// This Value must only be either a REG_SZ or a REG_MULTI_SZ
		if(dwType == REG_MULTI_SZ)
			while(dwPos < dwData)
			{
				dwPos = strlen((LPTSTR)lpData);
				if(*(lpData + dwPos + 1) != '\0')
					memset(lpData + dwPos,10,1);
				else
					dwPos = dwData;
			}
		try
		{
			m_DaoQueryDef->Create(sQName,(LPTSTR)lpData);
			m_DaoQueryDef->Append();
			m_DaoQueryDef->Close();
		}
		catch(CDaoException *e)
		{
			e->Delete();
		}
		CReg.CloseRegKey(hQDefKey);
	}
	return(true);

}

void CMDBMaintenance::GetColumnInfo(HKEY hCKey,CDaoFieldInfo &fInfo)
{
	CRegistry CReg;
	DWORD dwcbName = 0,dwType = 0,dwcbValue = 0,dwIndex = 0,dwSubKeys = 0,
		dwMaxSubKeyLen = 0,dwValues = 0,dwMaxValueNameLen = 0,dwMaxValueLen = 0,
		dwMValueLen = 0,dwMValueNameLen = 0,dwMSubKeyLen = 0,dwValCount = 0;
	LPTSTR lpValueName;
	LPBYTE Bval;
	long lRes = 0;

	fInfo.m_strName = "";
	fInfo.m_nType = 0;
	fInfo.m_lSize = 0;
	fInfo.m_lAttributes = 0;
	fInfo.m_nOrdinalPosition = 0;
	fInfo.m_bRequired = 0;
	fInfo.m_bAllowZeroLength = 0;
	fInfo.m_lCollatingOrder = 0;
	fInfo.m_strForeignName = "";
	fInfo.m_strSourceField = "";
	fInfo.m_strSourceTable = "";
	fInfo.m_strValidationRule = "";
	fInfo.m_strDefaultValue = "";

	CReg.QueryRegKey(hCKey,&dwSubKeys,&dwMaxSubKeyLen,&dwValues,
		&dwMaxValueNameLen,&dwMaxValueLen);

	lpValueName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwMaxValueNameLen + 1);
	dwMValueNameLen = dwMaxValueNameLen + 1;
	Bval = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwMaxValueLen + 1);
	dwMValueLen = dwMaxValueLen + 1;
	while(dwValCount < dwValues)
	{
		CReg.EnumRegValue(hCKey,lpValueName,&dwMValueNameLen,&dwType,
			Bval,&dwMValueLen,dwValCount);

		if(strcmp(lpValueName,"m_strName") == 0)
			fInfo.m_strName = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_nType") == 0)
				memcpy(&fInfo.m_nType,Bval,sizeof(fInfo.m_nType));
			else if(strcmp(lpValueName,"m_lSize") == 0)
				memcpy(&fInfo.m_lSize,Bval,sizeof(fInfo.m_lSize));
			else if(strcmp(lpValueName,"m_lAttributes") == 0)
				memcpy(&fInfo.m_lAttributes,Bval,sizeof(fInfo.m_lAttributes));
			else if(strcmp(lpValueName,"m_nOrdinalPosition") == 0)
				memcpy(&fInfo.m_nOrdinalPosition,Bval,sizeof(fInfo.m_nOrdinalPosition));
			else if(strcmp(lpValueName,"m_bRequired") == 0)
				memcpy(&fInfo.m_bRequired,Bval,sizeof(fInfo.m_bRequired));
			else if(strcmp(lpValueName,"m_bAllowZeroLength") == 0)
				memcpy(&fInfo.m_bAllowZeroLength,Bval,sizeof(fInfo.m_bAllowZeroLength));
			else if(strcmp(lpValueName,"m_lCollatingOrder") == 0)
				memcpy(&fInfo.m_lCollatingOrder,Bval,sizeof(fInfo.m_lCollatingOrder));
			else if(strcmp(lpValueName,"m_strForeignName") == 0)
				fInfo.m_strForeignName = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_strSourceField") == 0)
				fInfo.m_strSourceField = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_strSourceTable") == 0)
				fInfo.m_strSourceTable = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_strValidationRule") == 0)
				fInfo.m_strValidationRule = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_strDefaultValue") == 0)
				fInfo.m_strDefaultValue = (LPTSTR)Bval;
			ZeroMemory((LPBYTE)Bval,dwMaxValueLen + 1);
			dwMValueNameLen = dwMaxValueNameLen + 1;
			dwMValueLen = dwMaxValueLen + 1;
			dwValCount++;
	}
	LocalFree(lpValueName);
	LocalFree((LPBYTE)Bval);
}

BOOL CMDBMaintenance::GetIndexInfo(HKEY hCKey, CDaoIndexInfo &iInfo, CDaoIndexFieldInfo &fiInfo)
{
	CRegistry CReg;
	DWORD dwcbName = 0,dwType = 0,dwcbValue = 0,dwIndex = 0,dwSubKeys = 0,
		dwMaxSubKeyLen = 0,dwValues = 0,dwMaxValueNameLen = 0,dwMaxValueLen = 0,
		dwMValueLen = 0,dwMValueNameLen = 0,dwMSubKeyLen = 0,dwValCount = 0;
	LPTSTR lpValueName;
	LPBYTE Bval;
	long lRes = 0;

	fiInfo.m_strName = "";
	fiInfo.m_bDescending = true;

	iInfo.m_strName = "";
	iInfo.m_pFieldInfos = &fiInfo;
	iInfo.m_nFields = 1;
	iInfo.m_bPrimary = true;
	iInfo.m_bUnique = true;
	iInfo.m_bClustered = false;
	iInfo.m_bIgnoreNulls = false;
	iInfo.m_bRequired = true;
	iInfo.m_bForeign = false;
	iInfo.m_lDistinctCount = 0;

	CReg.QueryRegKey(hCKey,&dwSubKeys,&dwMaxSubKeyLen,&dwValues,
		&dwMaxValueNameLen,&dwMaxValueLen);

	lpValueName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwMaxValueNameLen + 1);
	dwMValueNameLen = dwMaxValueNameLen + 1;
	Bval = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwMaxValueLen + 1);
	dwMValueLen = dwMaxValueLen + 1;
	while(dwValCount < dwValues)
	{
		CReg.EnumRegValue(hCKey,lpValueName,&dwMValueNameLen,&dwType,
			Bval,&dwMValueLen,dwValCount);

		if(strcmp(lpValueName,"m_strName") == 0)
		{
			iInfo.m_strName = (LPTSTR)Bval;
			fiInfo.m_strName = (LPTSTR)Bval;
		}
			else if(strcmp(lpValueName,"m_bDescending") == 0)
				memcpy(&fiInfo.m_bDescending,Bval,sizeof(fiInfo.m_bDescending));
			else if(strcmp(lpValueName,"m_nFields") == 0)
				memcpy(&iInfo.m_nFields,Bval,sizeof(iInfo.m_nFields));
			else if(strcmp(lpValueName,"m_bPrimary") == 0)
				memcpy(&iInfo.m_bPrimary,Bval,sizeof(iInfo.m_bPrimary));
			else if(strcmp(lpValueName,"m_bUnique") == 0)
				memcpy(&iInfo.m_bUnique,Bval,sizeof(iInfo.m_bUnique));
			else if(strcmp(lpValueName,"m_bClustered") == 0)
				memcpy(&iInfo.m_bClustered,Bval,sizeof(iInfo.m_bClustered));
			else if(strcmp(lpValueName,"m_bIgnoreNulls") == 0)
				memcpy(&iInfo.m_bIgnoreNulls,Bval,sizeof(iInfo.m_bIgnoreNulls));
			else if(strcmp(lpValueName,"m_bRequired") == 0)
				memcpy(&iInfo.m_bRequired,Bval,sizeof(iInfo.m_bRequired));
			else if(strcmp(lpValueName,"m_bForeign") == 0)
				memcpy(&iInfo.m_bForeign,Bval,sizeof(iInfo.m_bForeign));
			else if(strcmp(lpValueName,"m_lDistinctCount") == 0)
				memcpy(&iInfo.m_lDistinctCount,Bval,sizeof(iInfo.m_lDistinctCount));

			ZeroMemory((LPBYTE)Bval,dwMaxValueLen + 1);
			dwMValueNameLen = dwMaxValueNameLen + 1;
			dwMValueLen = dwMaxValueLen + 1;
			dwValCount++;
	}
	LocalFree(lpValueName);
	LocalFree((LPBYTE)Bval);
	return(true);
}
