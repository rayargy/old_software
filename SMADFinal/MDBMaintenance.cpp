// MDBMaintenance.cpp: implementation of the CMDBMaintenance class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MDBMaintenance.h"
#include "registry.h"

CDaoDatabase* CMDBMaintenance::m_daodb = NULL;
CDaoTableDef* CMDBMaintenance::m_DaoTableDef = NULL;
CDaoQueryDef* CMDBMaintenance::m_DaoQueryDef = NULL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMDBMaintenance::CMDBMaintenance(CString sDbKey)
{
	// The constructor for this object requires the CString to be passed.
	// The string passed must be the location in the registry where the
	// Access database configuration info is stored. If a null string or
	// an incorrect string is passed, the constructor will attempt to use
	// default values. This does not mean that initMdb will be succesful
	// because the configuration info for the Mdb should also be under
	// this key. If the configuration info is not present in the registry,
	// the Mdb will not be rebuilt, however it may be successfully repaired

	CRegistry CReg;
	HKEY hDb;
	LPTSTR szDbKey;
	long lRes = 0;

	m_Log->GetInstance();
	CString strMsg = "CMDBMaintenance::CMDBMaintenance(): Performing construction of CMDBMaintenance";
	sprintf(m_szMessage,"%s: %s",strMsg,CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"));
	m_Log->WriteLogEntry(m_szMessage,1);

	// initialize dao
	GetDaoPtr();

	// now get all of the file info from the registry!
	m_strDbKey = sDbKey;
	szDbKey = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,sDbKey.GetLength() + 1);
	strcpy(szDbKey,sDbKey);
	hDb = CReg.OpenRegKey(&lRes,szDbKey,HKEY_LOCAL_MACHINE);
	if(lRes == ERROR_SUCCESS && CReg.m_dwDisposition == REG_CREATED_NEW_KEY)
	{
		CString strMsg = "\tCMDBMaintenance::CMDBMaintenance(): The specified registry key ";
		strMsg += sDbKey;
		strMsg += " did not exist!: ";
		strMsg += CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
		sprintf(m_szMessage,"%s",strMsg);
		m_Log->WriteLogEntry(m_szMessage,1);
	}

	LocalFree(szDbKey);

	if(!GetRegValue(hDb,m_strTempDb,"TempDb"))
		m_strTempDb = "TransTemp.mdb";
	if(!GetRegValue(hDb,m_strTempDbPath,"TempDbPath"))
		m_strTempDbPath = "C:\\LogFiles\\";
	if(!GetRegValue(hDb,m_strBackupPath,"BackupPath"))
		m_strBackupPath = "C:\\LogFiles\\Backup\\";

	if(lRes == ERROR_SUCCESS) CReg.CloseRegKey(hDb);

	hDb = CReg.OpenRegKey(&lRes,
		"SOFTWARE\\CTRSystems\\PARCS-CS\\Transactions",HKEY_LOCAL_MACHINE);
	if(lRes == ERROR_SUCCESS && CReg.m_dwDisposition == REG_CREATED_NEW_KEY)
	{
		CString strMsg = "\tCMDBMaintenance::CMDBMaintenance(): The specified registry key ";
		strMsg += "SOFTWARE\\CTRSystems\\PARCS-CS\\Transactions";
		strMsg += " did not exist!: ";
		strMsg += CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
		sprintf(m_szMessage,"%s",strMsg);
		m_Log->WriteLogEntry(m_szMessage,1);
	}

	if(!GetRegValue(hDb,m_strDestinationDb,"BufferLoc"))
		m_strDestinationDb = "C:\\LogFiles\\TransBuff.mdb";
	if(lRes == ERROR_SUCCESS) CReg.CloseRegKey(hDb);
}

CMDBMaintenance::~CMDBMaintenance()
{
	CString strMsg;

	// we're done with the mdb so close it and destroy the dao engine.
	vCloseDb();
	DestroyDaoPtr();

	strMsg = "CMDBMaintenance::~CMDBMaintenance(): Destructor for CMDBMaintenance";
	sprintf(m_szMessage,"%s: %s",strMsg,CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"));
	m_Log->WriteLogEntry(m_szMessage,1);
}

void CMDBMaintenance::GetDaoPtr()
{
	if(m_daodb == NULL)
		m_daodb = new CDaoDatabase;
	AfxDaoInit();
}

void CMDBMaintenance::DestroyDaoPtr()
{
	if(m_daodb != NULL)
	{
		delete m_daodb;
		m_daodb = NULL;
	}
	AfxDaoTerm();
}

BOOL CMDBMaintenance::GetRegValue(HKEY hTempDb,CString &sValue,CString sValueName)
{
	CRegistry CReg;
	LPVOID lpMsgBuf;
	long lRes = 0;
	LPTSTR szValueName;

	szValueName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,sValueName.GetLength() + 1);
	strcpy(szValueName,sValueName);
	// attempt to retrieve the requested value
	sValue = CReg.FetchStrValue(&lRes,hTempDb,szValueName);
	LocalFree(szValueName);
	if(lRes == ERROR_SUCCESS)
		return(TRUE);
	
	// if we fail to retrieve the value get a system message and write to the debug log
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,NULL,lRes,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,0,NULL);
	sprintf(m_szMessage,"\tCMDBMaintenance::GetRegValue() %s\tGetRegValue Failed to retrieve %s",(LPTSTR)lpMsgBuf,sValueName);
	m_Log->WriteLogEntry(m_szMessage,2);
	LocalFree(lpMsgBuf);
	return(FALSE);
}

int CMDBMaintenance::InitMdb()
{
	int iRetVal = 0;

	// iRetVal 1 = MDB repaired -1 = MDB Rebuilt 0 = failed
	// DisablePolling() disables polling at the commserver level!
	if(DisablePolling())
	{
		// attempt to make a copy of the possibly corrupt Mdb for research.
		BackupMDB();
		// attempt to repair the database.
		if(m_bRetVal = RepairMdb()) iRetVal = 1;
		if(!m_bRetVal)
			// if the Mdb is not repairable create a new one!
			if(m_bRetVal = CreateMdb()) iRetVal = -1;

		// if not bRetVal report dao error;
	}

	// if the database was either successfully repaired or recreated,
	// process the transdump file into the database. This will make sure
	// that no transactions have been missed!
	if(m_bRetVal) ProcessTransDump();

	MoveMDB();

	// EnablePolling() starts polling so that transactions will come through again!
	EnablePolling();

	return(iRetVal);
}

BOOL CMDBMaintenance::RepairMdb()
{
	CString strMsg,sTemp,sTempDb;

	sTemp = m_strBackupPath + m_strTempDb;
	sTempDb = m_strTempDbPath + m_strTempDb;

	// if we have made it to here, then the file move could not fail
	// from a file sharing violation because the corrupt database is inaccessable!
	// The only thing that is lost is a backup of the corrupt mdb
	// we will still attempt a repair and a rebuild if the repair fails.
	MoveFileEx(m_strDestinationDb,sTemp,MOVEFILE_COPY_ALLOWED |
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

	try
	{
		m_daodb->m_pWorkspace->RepairDatabase(sTemp);
		m_daodb->m_pWorkspace->CompactDatabase(sTemp,
			sTempDb,dbLangGeneral);
	}
	catch(CDaoException *e)
	{
		strMsg  = "\tCMDBMaintenance::RepairMdb(): Failed: " + e->m_pErrorInfo->m_strSource \
					+ "\t" + e->m_pErrorInfo->m_strDescription;
		sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
		m_Log->WriteLogEntry ( m_szMessage, 1 );

		e->Delete();
		return(FALSE);
	}

	// if we made it this far, delete the temporary copy of the mdb
	// because it was successfully repaired
	DeleteFile(sTemp);

	sprintf(m_szMessage,"\tCMDBMaintenance::RepairMdb(): Successful");
	m_Log->WriteLogEntry(m_szMessage,1);

	return(TRUE);
}

BOOL CMDBMaintenance::CreateMdb()
{
	CString strMsg;
	BOOL bDbExists = FALSE,bTablesCreated = FALSE,bQueriesCreated = FALSE;
	int iRetries = 0;
	CString sTemp = m_strTempDbPath + m_strTempDb;

	do
	{
		try
		{
			// this is the first step in the rebuild create a temporary mdb
			// so that no application will attempt to access it while we are
			// working on it.
			m_daodb->Create(sTemp,dbLangGeneral,0);
			bDbExists = TRUE;
		}
		catch(CDaoException *e)
		{
			strMsg  = "\tCMDBMaintenance::CreateMdb(): Failed: " + e->m_pErrorInfo->m_strSource \
						+ "\t" + e->m_pErrorInfo->m_strDescription;
			sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
			m_Log->WriteLogEntry ( m_szMessage, 1 );

			if(DBDAOERR(e->m_pErrorInfo->m_lErrorCode) == E_DAO_DatabaseDuplicate)
				if(!DeleteFile(sTemp))
				{
					LPVOID lpMsgBuf;

					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &lpMsgBuf,0,NULL);
						strMsg = (LPTSTR)lpMsgBuf;
					sprintf(m_szMessage,"\tCMDBMaintenance::CreateMdb(): %s\tCould not delete %s",strMsg,sTemp);
					m_Log->WriteLogEntry ( m_szMessage, 1 );
					LocalFree(lpMsgBuf);
				}
			iRetries++;
			e->Delete();
		}
		Sleep(100);
	} while(!bDbExists && iRetries < 5);

	if(bDbExists)
	{
		// create all table definitions and query definitions
		bTablesCreated = CreateTableDef();
		bQueriesCreated = CreateQueryDef();
		vCloseDb();
		if(!(bTablesCreated && bQueriesCreated))
			bDbExists = FALSE;
		else
		{
			sprintf(m_szMessage,"\tCMDBMaintenance::CreateMdb(): Successful");
			m_Log->WriteLogEntry(m_szMessage,1);
		}
	}
	return(bDbExists);
}

BOOL CMDBMaintenance::CreateTableDef()
{
	// initialize the Table pointer
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
	CString strMsg;

	sTableKey = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,
		m_strDbKey.GetLength() + strlen("\\TableDefs") + 1);
	strcpy(sTableKey,m_strDbKey);
	strcat(sTableKey,"\\TableDefs");
	hTableKey = CReg.OpenRegKey(&lRes,sTableKey,HKEY_LOCAL_MACHINE);
	if(lRes != ERROR_SUCCESS)
	{	// key open failed so write a debug message
		sprintf(m_szMessage,
			"\tCMDBMaintenance::CreateTableDef(): OpenRegKey failed: Error %l",sTableKey,lRes);
		m_Log->WriteLogEntry(m_szMessage,2);
		LocalFree(sTableKey);
		return(FALSE);
	}

	if(CReg.m_dwDisposition == REG_CREATED_NEW_KEY)
	{	// The key requested did not exist before opening it.
		sprintf(m_szMessage,
			"\tCMDBMaintenance::CreateTableDef(): Registry Key: %s did not previously exist",sTableKey);
		m_Log->WriteLogEntry(m_szMessage,2);
		LocalFree(sTableKey);
		CReg.CloseRegKey(hTableKey);
		return(FALSE);
	}

	CReg.QueryRegKey(hTableKey,&dwSubKeys,&dwMaxSubKeyLen,&dwValues,
		&dwMaxValueNameLen,&dwMaxValueLen);

	if(dwSubKeys == 0)
	{	// if dwSubKeys = 0 then there are no tables defined
		sprintf(m_szMessage,
			"\tCMDBMaintenance::CreateTableDef(): No Table Definitions present in %s",sTableKey);
		m_Log->WriteLogEntry(m_szMessage,2);
		LocalFree(sTableKey);
		CReg.CloseRegKey(hTableKey);
		return(FALSE);
	}
	// we done with sTableKey so deallocate
	LocalFree(sTableKey);

	sTableName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwMaxSubKeyLen + 1);
	dwMSubKeyLen = dwMaxSubKeyLen + 1;
	while(dwKeyCount < dwSubKeys)
	{	// if we get this far there are table definitions in the registry
		// we will enumerate each key under "TableDefs" to get the Table Info.
		lRes = CReg.EnumRegKey(hTableKey,sTableName,&dwMSubKeyLen,&ft,dwKeyCount);
		try
		{
			m_DaoTableDef->Create(sTableName,0,sTableName);
			if(bRetVal = CreateColumns(hTableKey,sTableName))
				dwKeyCount++;
			m_DaoTableDef->Append();
			dwMSubKeyLen = dwMaxSubKeyLen + 1;
			if(m_DaoTableDef->IsOpen())
				m_DaoTableDef->Close();
		}
		catch(CDaoException *e)
		{
			strMsg  = "\tCMDBMaintenance::CreateTableDef(): Failed: " + e->m_pErrorInfo->m_strSource \
						+ "\t" + e->m_pErrorInfo->m_strDescription;
			sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
			m_Log->WriteLogEntry ( m_szMessage, 1 );

			bRetVal = FALSE;
			e->Delete();
		}


		if(bRetVal == FALSE) break;
	}

	LocalFree(sTableName);	// we done with this deallocate
	CReg.CloseRegKey(hTableKey);	// close the registry key
	return(bRetVal);

}

BOOL CMDBMaintenance::CreateQueryDef()
{
	// initialize the Query pointer
	CDaoQueryDef q(m_daodb);
	m_DaoQueryDef = &q;

	long lRes = 0;
	DWORD dwSubKeys = 0, dwMaxSubKeyLen = 0, dwValues = 0,
		dwMaxValueNameLen = 0, dwMaxValueLen = 0, dwMSubKeyLen = 0,
		dwKeyCount = 0;
	CRegistry CReg;
	HKEY hQueryKey;
	LPTSTR sQueryName,sQueryKey;
	FILETIME ft;
	BOOL bRetVal = TRUE;

	sQueryKey = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,m_strDbKey.GetLength() + strlen("\\QueryDefs") + 1);
	strcpy(sQueryKey,m_strDbKey);
	strcat(sQueryKey,"\\QueryDefs");
	hQueryKey = CReg.OpenRegKey(&lRes,sQueryKey,HKEY_LOCAL_MACHINE);
	if(lRes != ERROR_SUCCESS)
	{	// key open failed so write a debug message
		sprintf(m_szMessage,
			"\tCMDBMaintenance::CreateQueryDef(): Could not open the  specified registry key did not exist before CReg.OpenRegKey\n\t%s",sQueryKey);
		m_Log->WriteLogEntry(m_szMessage,2);
		LocalFree(sQueryKey);
		return(FALSE);
	}

	if(CReg.m_dwDisposition == REG_CREATED_NEW_KEY)
	{	// The key requested did not exist before opening it.
		sprintf(m_szMessage,
			"\tCMDBMaintenance::CreateQueryDef(): The specified registry key did not exist before CReg.OpenRegKey\n\t%s",sQueryKey);
		m_Log->WriteLogEntry(m_szMessage,2);
		LocalFree(sQueryKey);
		CReg.CloseRegKey(hQueryKey);
		return(FALSE);
	}

	CReg.QueryRegKey(hQueryKey,&dwSubKeys,&dwMaxSubKeyLen,&dwValues,
		&dwMaxValueNameLen,&dwMaxValueLen);

	if(dwSubKeys == 0)
	{	// if dwSubKeys = 0 then there are no queries defined
		sprintf(m_szMessage,
			"\tCMDBMaintenance::CreateQueryDef(): No Query Definitions present in \n\t%s",sQueryKey);
		m_Log->WriteLogEntry(m_szMessage,2);
		LocalFree(sQueryKey);
		CReg.CloseRegKey(hQueryKey);
		return(FALSE);
	}
	LocalFree(sQueryKey);

	sQueryName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwMaxSubKeyLen + 1);
	dwMSubKeyLen = dwMaxSubKeyLen + 1;
	while(dwKeyCount < dwSubKeys)
	{	// if we get this far there are query definitions in the registry
		// we will enumerate each key under "QueryDefs" to get the Table Info.

		lRes = CReg.EnumRegKey(hQueryKey,sQueryName,&dwMSubKeyLen,&ft,dwKeyCount);

		sprintf(m_szMessage,
			"\tCMDBMaintenance::CreateQueryDef(): Creating Query\n\t%s",sQueryName);
		m_Log->WriteLogEntry(m_szMessage,4);

		bRetVal = CreateQuery(hQueryKey,sQueryName);
		dwKeyCount++;
		dwMSubKeyLen = dwMaxSubKeyLen + 1;
		if(bRetVal == FALSE) break;
	}

	LocalFree(sQueryName);	
	CReg.CloseRegKey(hQueryKey);
	return(bRetVal);

}


BOOL CMDBMaintenance::CreateColumns(HKEY hDbKey, LPTSTR szTableName)
{
	BOOL bRetVal = TRUE;
	LPTSTR sColumnName;
	CString strMsg;
	HKEY hTableName;
	CRegistry CReg;
	long lRes = 0;
	DWORD dwSubKeys = 0,dwMaxSubKeyLen = 0,dwValues = 0,dwMaxValueNameLen = 0,
		dwMaxValueLen = 0,dwMSubKeyLen = 0,dwKeyCount = 0;
	FILETIME ft;
	CDaoFieldInfo fInfo;
	CDaoIndexInfo *iInfo;
	CDaoIndexFieldInfo *fiInfo;

	hTableName = CReg.OpenRegKey(&lRes,szTableName,hDbKey);
	if(lRes != ERROR_SUCCESS)
	{	// key open failed so write a debug message
		sprintf(m_szMessage,
			"\tCMDBMaintenance::CreateColumns(): failed: CReg.OpenRegKey failed: %s",szTableName);
		m_Log->WriteLogEntry(m_szMessage,2);
		return(FALSE);
	}

	// We already know the key existed because it was enumerated by the registry function
	// do not need to check m_dwDisposition.

	CReg.QueryRegKey(hTableName,&dwSubKeys,&dwMaxSubKeyLen,&dwValues,
		&dwMaxValueNameLen,&dwMaxValueLen);

	if(dwSubKeys == 0)
	{	// if dwSubKeys = 0 then there are no Columns defined
		sprintf(m_szMessage,
			"\tCMDBMaintenance::CreateColumns(): No Column Definitions present in %s",szTableName);
		m_Log->WriteLogEntry(m_szMessage,2);
		CReg.CloseRegKey(hTableName);
		return(FALSE);
	}

	sColumnName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwMaxSubKeyLen + 1);
	dwMSubKeyLen = dwMaxSubKeyLen + 1;
	while(dwKeyCount < dwSubKeys)
	{
		lRes = CReg.EnumRegKey(hTableName,sColumnName,&dwMSubKeyLen,&ft,dwKeyCount);
		CRegistry RegColumn;
		// we know this key exists so go ahead and open if it returns an error its fatal
		HKEY hColumnKey = RegColumn.OpenRegKey(&lRes,sColumnName,hTableName);
		if(lRes == ERROR_SUCCESS)
		{
			sprintf(m_szMessage,"\tCMDBMaintenance::CreateColumns(): Retrieving info for %s.%s",
				szTableName,sColumnName);
			m_Log->WriteLogEntry ( m_szMessage, 4 );

			GetColumnInfo(hColumnKey,fInfo);
			try
			{
				m_DaoTableDef->CreateField(fInfo);
			}
			catch(CDaoException *e)
			{
				strMsg  = "\tCMDBMaintenance::CreateColumns(): Failed: " + e->m_pErrorInfo->m_strSource \
						+ "\t" + e->m_pErrorInfo->m_strDescription;
				sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
				m_Log->WriteLogEntry ( m_szMessage, 1 );
				bRetVal = FALSE;
				e->Delete();
			}

			CString sIndexed = RegColumn.FetchStrValue(&lRes,hColumnKey,"Indexed");
			if(lRes == ERROR_SUCCESS && sIndexed == "1")
			{
				iInfo = new CDaoIndexInfo;
				// Pass a NULL into GetIndexInfo in the fieldindexinfo param to get the
				// number of fields in the index so memory may be allocated.
				GetIndexInfo(hColumnKey,iInfo,NULL);

				fiInfo = new CDaoIndexFieldInfo[iInfo->m_nFields];

				GetIndexInfo(hColumnKey,iInfo,fiInfo);
				try
				{
					m_DaoTableDef->CreateIndex(*iInfo);
				}
				catch(CDaoException *e)
				{
					strMsg  = "\tCMDBMaintenance::CreateColumns(): Failed: " + e->m_pErrorInfo->m_strSource \
							+ "\t" + e->m_pErrorInfo->m_strDescription;
					sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
					m_Log->WriteLogEntry ( m_szMessage, 1 );
					bRetVal = FALSE;
					e->Delete();
				}
				delete[] fiInfo;
				delete iInfo;
			}

			// get field type is a primary key?
			// create field index
			RegColumn.CloseRegKey(hColumnKey);
			dwKeyCount++;
			dwMSubKeyLen = dwMaxSubKeyLen + 1;
		}
		else
		{
			sprintf(m_szMessage,"\tCMDBMaintenance::CreateColumns() Failed: Could not open %s\n\tGetLastError returned %l",sColumnName,GetLastError());
			m_Log->WriteLogEntry ( m_szMessage, 1 );
			bRetVal = FALSE;
		}

		if(bRetVal == FALSE) break;
	}

	LocalFree(sColumnName);
	CReg.CloseRegKey(hTableName);

	return(bRetVal);

}

BOOL CMDBMaintenance::CreateQuery(HKEY hQKey, LPTSTR sQName)
{
	BOOL bRetVal = TRUE;
	CRegistry CReg;
	HKEY hQDefKey;
	long lRes = 0;
	DWORD dwType = 0,dwData = 0,dwPos = 0;
	LPBYTE lpData;

	hQDefKey = CReg.OpenRegKey(&lRes,sQName,hQKey);
	if(lRes == ERROR_SUCCESS)
	{
		CReg.QueryRegValue(hQDefKey,"SQL",&dwType,NULL,&dwData);
		lpData = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwData + 1);
		lRes = CReg.QueryRegValue(hQDefKey,"SQL",&dwType,lpData,&dwData);
		// This Value must only be either a REG_SZ or a REG_MULTI_SZ
		if(lRes == ERROR_SUCCESS)
		{
			if(dwType == REG_MULTI_SZ || dwType == REG_SZ)
			{
				if(dwType == REG_MULTI_SZ)
					while(dwPos < dwData)
					{
						dwPos = strlen((LPTSTR)lpData);
						if(*(lpData + dwPos + 1) != '\0')
							// REG_MULTISZ values are a series of null terminated strings
							// with the last string being double nulled. Here we elminate
							// the nulls within the string-- make spaces
							memset(lpData + dwPos,10,1);
						else
							dwPos = dwData;
					}

				CString strDebugMsg;
				strDebugMsg.Format("\tCMDBMaintenance::CreateQuery()\n%s",(LPTSTR)lpData);
				m_Log->WriteLogEntry ( strDebugMsg.GetBuffer(strDebugMsg.GetLength() + 1), 4 );
				strDebugMsg.ReleaseBuffer();

				try
				{
					m_DaoQueryDef->Create(sQName,(LPTSTR)lpData);
					m_DaoQueryDef->Append();
					if(m_DaoQueryDef->IsOpen())
						m_DaoQueryDef->Close();
				}
				catch(CDaoException *e)
				{
					CString strMsg;
					strMsg  = "\tCMDBMaintenance::CreateQuery() Failed: " + e->m_pErrorInfo->m_strSource \
						+ "\t" + e->m_pErrorInfo->m_strDescription;
					sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
					m_Log->WriteLogEntry ( m_szMessage, 1 );
					bRetVal = FALSE;
					e->Delete();
				}
			}
			else
			{
				sprintf(m_szMessage,"\tCMDBMaintenance::CreateQuery(): Invalid value type");
				m_Log->WriteLogEntry(m_szMessage,4);
				bRetVal = FALSE;
			}
		}
		else
		{
			sprintf(m_szMessage,"\tCMDBMaintenance::CreateQuery(): failed to query reg value: SQL");
			m_Log->WriteLogEntry(m_szMessage,4);
			bRetVal = FALSE;
		}

		LocalFree(lpData);
		CReg.CloseRegKey(hQDefKey);
	}
	else
	{
		sprintf(m_szMessage,"\tCMDBMaintenance::CreateQuery(): failed to open reg key: %s",sQName);
		m_Log->WriteLogEntry(m_szMessage,4);
		bRetVal = FALSE;
	}

	return(bRetVal);

}

void CMDBMaintenance::GetColumnInfo(HKEY hCKey,CDaoFieldInfo &fInfo)
{
	CRegistry CReg;
	DWORD dwType = 0,dwSubKeys = 0,dwMaxSubKeyLen = 0,dwValues = 0,
		dwMaxValueNameLen = 0,dwMaxValueLen = 0,dwMValueLen = 0,
		dwMValueNameLen = 0,dwValCount = 0;
	LPTSTR lpValueName;
	LPBYTE Bval;
	long lRes = 0;

	// assign defaut values for the field information
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

		// values for each of the registry entries below is documented in
		// in the MFC CDaoFieldInfo structure and the source code for
		// DBDAOINT.H in Devstudio\vc\include.
		if(strcmp(lpValueName,"m_strName") == 0)
			// m_strName is expected to be a REG_SZ
			fInfo.m_strName = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_nType") == 0)
				// m_nType is expected to be a REG_DWORD
				memcpy(&fInfo.m_nType,Bval,sizeof(fInfo.m_nType));
			else if(strcmp(lpValueName,"m_lSize") == 0)
				// m_lSize is expected to be a REG_DWORD
				memcpy(&fInfo.m_lSize,Bval,sizeof(fInfo.m_lSize));
			else if(strcmp(lpValueName,"m_lAttributes") == 0)
				// m_lAttributes is expected to be a REG_DWORD
				memcpy(&fInfo.m_lAttributes,Bval,sizeof(fInfo.m_lAttributes));
			else if(strcmp(lpValueName,"m_nOrdinalPosition") == 0)
				// m_nOrdinalPosition is expected to be a REG_DWORD
				memcpy(&fInfo.m_nOrdinalPosition,Bval,sizeof(fInfo.m_nOrdinalPosition));
			else if(strcmp(lpValueName,"m_bRequired") == 0)
				// m_bRequired is expected to be a REG_DWORD
				memcpy(&fInfo.m_bRequired,Bval,sizeof(fInfo.m_bRequired));
			else if(strcmp(lpValueName,"m_bAllowZeroLength") == 0)
				// m_bAllowZeroLength is expected to be a REG_DWORD
				memcpy(&fInfo.m_bAllowZeroLength,Bval,sizeof(fInfo.m_bAllowZeroLength));
			else if(strcmp(lpValueName,"m_lCollatingOrder") == 0)
				// m_lCollatingOrder is expected to be a REG_DWORD
				memcpy(&fInfo.m_lCollatingOrder,Bval,sizeof(fInfo.m_lCollatingOrder));
			else if(strcmp(lpValueName,"m_strForeignName") == 0)
				// m_strForeignName is expected to be a REG_SZ
				fInfo.m_strForeignName = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_strSourceField") == 0)
				// m_strSourceField is expected to be a REG_SZ
				fInfo.m_strSourceField = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_strSourceTable") == 0)
				// m_strSourceTable is expected to be a REG_SZ
				fInfo.m_strSourceTable = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_strValidationRule") == 0)
				// m_strValidationRule is expected to be a REG_SZ
				fInfo.m_strValidationRule = (LPTSTR)Bval;
			else if(strcmp(lpValueName,"m_strDefaultValue") == 0)
				// m_strDefaultValue is expected to be a REG_SZ
				fInfo.m_strDefaultValue = (LPTSTR)Bval;

			ZeroMemory((LPBYTE)Bval,dwMaxValueLen + 1);
			dwMValueNameLen = dwMaxValueNameLen + 1;
			dwMValueLen = dwMaxValueLen + 1;
			dwValCount++;
	}

	sprintf(m_szMessage,
		"\tCMDBMaintenance::GetColumnInfo():\n\tCDaoFieldInfo structure values\n\
		m_strName = %s\n\
		m_nType = %d\n\
		m_lSize = %u\n\
		m_lAttributes = %u\n\
		m_nOrdinalPosition = %d\n\
		m_bRequired = %d\n\
		m_bAllowZeroLength = %d\n\
		m_lCollatingOrder = %u\n\
		m_strForeignName = %s\n\
		m_strSourceField = %s\n\
		m_strSourceTable = %s\n\
		m_strValidationRule = %s\n\
		m_strValidationText = %s\n\
		m_strDefaultValue = %s",
		fInfo.m_strName,
		fInfo.m_nType,
		fInfo.m_lSize,
		fInfo.m_lAttributes,
		fInfo.m_nOrdinalPosition,
		fInfo.m_bRequired,
		fInfo.m_bAllowZeroLength,
		fInfo.m_lCollatingOrder,
		fInfo.m_strForeignName,
		fInfo.m_strSourceField,
		fInfo.m_strSourceTable,
		fInfo.m_strValidationRule,
		fInfo.m_strValidationText,
		fInfo.m_strDefaultValue);
	m_Log->WriteLogEntry(m_szMessage,4);

	LocalFree(lpValueName);
	LocalFree((LPBYTE)Bval);
}

BOOL CMDBMaintenance::GetIndexInfo(HKEY hCKey, CDaoIndexInfo *iInfo, CDaoIndexFieldInfo *fiInfo)
{
	CRegistry CReg;
	DWORD dwType = 0,dwSubKeys = 0,dwMaxSubKeyLen = 0,dwValues = 0,
		dwMaxValueNameLen = 0,dwMaxValueLen = 0,dwMValueLen = 0,
		dwMValueNameLen = 0,dwValCount = 0,dwData = 0;
	LPTSTR lpValueName;
	LPBYTE Bval,lpData;
	CString strColumnList;
	HKEY hIndex;
	long lRes = 0;

	// assign some default values for the CDaoIndexInfo structure.
	iInfo->m_pFieldInfos = fiInfo;
	iInfo->m_nFields = 0;
	iInfo->m_bPrimary = false;
	iInfo->m_bUnique = false;
	iInfo->m_bClustered = false;
	iInfo->m_bIgnoreNulls = false;
	iInfo->m_bRequired = false;
	iInfo->m_bForeign = false;
	iInfo->m_lDistinctCount = 0;

	hIndex = CReg.OpenRegKey(&lRes,"Index",hCKey);
	if(lRes == ERROR_SUCCESS)
	{
		lRes = CReg.QueryRegValue(hIndex,"IndexFieldInfo",&dwType,NULL,&dwData);
		lpData = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwData + 1);
		lRes = CReg.QueryRegValue(hIndex,"IndexFieldInfo",&dwType,lpData,&dwData);

		// This value must be a REG_MULTISZ. It contains a structure like the
		// CDaoFieldIndexInfo. It contains a column name to include in the index
		// and a boolean value for ascending or descending order for the index.
		// If this value is not organized this way, The index creation will fail!
		if(lRes == ERROR_SUCCESS && dwType == REG_MULTI_SZ)
		{
			LPTSTR sData = (LPTSTR)lpData;
			DWORD dwPos = 0;
			int iCount = 0;
			if(dwType == REG_MULTI_SZ)
				while(*(sData) != '\0')
				{
					dwPos = strlen(sData);

					if(fiInfo != NULL)
					{
						// second time through file the CDaoFieldIndexInfo.
						fiInfo[iCount/2].m_strName = sData;
						strColumnList += "\t\t\t\tm_strName = ";
						strColumnList += sData;
						strColumnList += "\n";
						sData = sData + dwPos + 1;
						strColumnList += "\t\t\t\tm_bDescending = ";
						strColumnList += sData;
						strColumnList += "\n";
						dwPos = strlen(sData);
						fiInfo[iCount/2].m_bDescending = (BOOL)atoi(sData);
						iCount++;
					}
					iCount++;
					sData = sData + dwPos + 1;
				}
			iInfo->m_nFields = iCount/2;
		}
		LocalFree(lpData);
		CReg.CloseRegKey(hIndex);
	}

	if(fiInfo == NULL)
		return(true);

	hIndex = CReg.OpenRegKey(&lRes,"Index",hCKey);
	if(lRes == ERROR_SUCCESS)
	{

		CReg.QueryRegKey(hIndex,&dwSubKeys,&dwMaxSubKeyLen,&dwValues,
			&dwMaxValueNameLen,&dwMaxValueLen);

		lpValueName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwMaxValueNameLen + 1);
		dwMValueNameLen = dwMaxValueNameLen + 1;
		Bval = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwMaxValueLen + 1);
		dwMValueLen = dwMaxValueLen + 1;
		while(dwValCount < dwValues)
		{
			CReg.EnumRegValue(hIndex,lpValueName,&dwMValueNameLen,&dwType,
				Bval,&dwMValueLen,dwValCount);
				if(strcmp(lpValueName,"IndexName") == 0)
					// IndexName is a REG_SZ value and is the name assigned to the
					// index. This parameter cannot be null.
					iInfo->m_strName = Bval;
				else if(strcmp(lpValueName,"m_bPrimary") == 0)
					// m_bPrimary must by type REG_DWORD
					memcpy(&iInfo->m_bPrimary,Bval,sizeof(iInfo->m_bPrimary));
				else if(strcmp(lpValueName,"m_bUnique") == 0)
					// m_bUnique must by type REG_DWORD
					memcpy(&iInfo->m_bUnique,Bval,sizeof(iInfo->m_bUnique));
				else if(strcmp(lpValueName,"m_bClustered") == 0)
					// m_bClustered must by type REG_DWORD
					memcpy(&iInfo->m_bClustered,Bval,sizeof(iInfo->m_bClustered));
				else if(strcmp(lpValueName,"m_bIgnoreNulls") == 0)
					// m_bIgnoreNulls must by type REG_DWORD
					memcpy(&iInfo->m_bIgnoreNulls,Bval,sizeof(iInfo->m_bIgnoreNulls));
				else if(strcmp(lpValueName,"m_bRequired") == 0)
					// m_bRequired must by type REG_DWORD
					memcpy(&iInfo->m_bRequired,Bval,sizeof(iInfo->m_bRequired));
				else if(strcmp(lpValueName,"m_bForeign") == 0)
					// m_bForeign must by type REG_DWORD
					memcpy(&iInfo->m_bForeign,Bval,sizeof(iInfo->m_bForeign));
				else if(strcmp(lpValueName,"m_lDistinctCount") == 0)
					// m_lDistinctCount must by type REG_DWORD
					memcpy(&iInfo->m_lDistinctCount,Bval,sizeof(iInfo->m_lDistinctCount));

				ZeroMemory((LPBYTE)Bval,dwMaxValueLen + 1);
				dwMValueNameLen = dwMaxValueNameLen + 1;
				dwMValueLen = dwMaxValueLen + 1;
				dwValCount++;
		}

		sprintf(m_szMessage,"\tCMDBMaintenance::GetIndexInfo():\n\tCDaoIndexInfo structure values\n\
			m_strName = %s\n\
			m_pFieldInfos = \n%s\n\
			m_nFields = %d\n\
			m_bPrimary = %d\n\
			m_bUnique = %d\n\
			m_bClustered = %d\n\
			m_bIgnoreNulls = %d\n\
			m_bRequired = %d\n\
			m_bForeign = %d\n\
			m_lDistinctCount = %u",
			iInfo->m_strName,
			strColumnList,
			iInfo->m_nFields,
			iInfo->m_bPrimary,
			iInfo->m_bUnique,
			iInfo->m_bClustered,
			iInfo->m_bIgnoreNulls,
			iInfo->m_bRequired,
			iInfo->m_bForeign,
			iInfo->m_lDistinctCount);
		m_Log->WriteLogEntry(m_szMessage,4);

		LocalFree(lpValueName);
		LocalFree((LPBYTE)Bval);
		CReg.CloseRegKey(hIndex);
	}
	return(true);
}

BOOL CMDBMaintenance::DisablePolling()
{
	CRegistry CReg;
	HKEY hComConfigKey;
	long lRes = 0;
	BOOL bRetVal = FALSE;
	LPVOID lpMsgBuf;
	CString strMsg;

	hComConfigKey = CReg.OpenRegKey(&lRes,
		"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\CommunicationServer",HKEY_LOCAL_MACHINE);
	if(lRes == ERROR_SUCCESS)
	{
		if(CReg.m_dwDisposition == REG_OPENED_EXISTING_KEY)
		{
			LPBYTE Bval;
			BYTE BDisablePoll = 0;
			Bval = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,sizeof(BYTE));
			memcpy(Bval,&BDisablePoll,sizeof(BYTE));
			lRes = CReg.SetRegValue(hComConfigKey,"PollEnable",REG_BINARY,Bval,sizeof(BYTE));
			LocalFree(Bval);
			if(lRes == ERROR_SUCCESS)
			{
				sprintf(m_szMessage,"\tCMDBMaintenance::DisablePolling(): Polling Disabled");
				m_Log->WriteLogEntry(m_szMessage,1);
				bRetVal = TRUE;
			}
			else
			{
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,NULL,lRes,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,0,NULL);
				strMsg = (LPTSTR)lpMsgBuf;
				sprintf(m_szMessage,"\tCMDBMaintenance::DisablePolling(): %s\tCould not set PollEnable",strMsg);
				m_Log->WriteLogEntry(m_szMessage,2);
				LocalFree(lpMsgBuf);
				bRetVal = FALSE;
			}
		}
		else
		{
			sprintf(m_szMessage,"\tCMDBMaintenance::DisablePolling(): Could not disable polling Registry did not exist!");
			m_Log->WriteLogEntry(m_szMessage,2);
			bRetVal = FALSE;
		}
		CReg.CloseRegKey(hComConfigKey);
	}
	else
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,lRes,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);
		strMsg = (LPTSTR)lpMsgBuf;
		sprintf(m_szMessage,"\tCMDBMaintenance::DisablePolling(): %s\tCould not open the CommServer Key",strMsg);
		m_Log->WriteLogEntry(m_szMessage,2);
		LocalFree(lpMsgBuf);
		bRetVal = FALSE;
	}

	return(bRetVal);
}

BOOL CMDBMaintenance::EnablePolling()
{
	LPVOID lpMsgBuf;
	CRegistry CReg;
	HKEY hComConfigKey;
	long lRes = 0;
	BOOL bRetVal = FALSE;

	hComConfigKey = CReg.OpenRegKey(&lRes,
		"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\CommunicationServer",HKEY_LOCAL_MACHINE);
	if(lRes == ERROR_SUCCESS)
	{
		if(CReg.m_dwDisposition == REG_OPENED_EXISTING_KEY)
		{
			LPBYTE Bval;
			BYTE BDisablePoll = 255;
			Bval = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,sizeof(BYTE));
			memcpy(Bval,&BDisablePoll,sizeof(BYTE));
			lRes = CReg.SetRegValue(hComConfigKey,"PollEnable",REG_BINARY,Bval,sizeof(BYTE));
			LocalFree(Bval);
			if(lRes == ERROR_SUCCESS)
			{
				sprintf(m_szMessage,"\tCMDBMaintenance::EnablePolling(): Polling Enabled");
				m_Log->WriteLogEntry(m_szMessage,1);
				bRetVal = TRUE;
			}
			else
			{
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,NULL,lRes,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,0,NULL);
				sprintf(m_szMessage,"\tCMDBMaintenance::EnablePolling(): %s\tCould not set PollEnable",(LPTSTR)lpMsgBuf);
				m_Log->WriteLogEntry(m_szMessage,2);
				LocalFree(lpMsgBuf);
				bRetVal = FALSE;
			}
		}
		else
		{
			sprintf(m_szMessage,"\tCMDBMaintenance::EnablePolling(): Could not enable polling Registry key did not exist!");
			m_Log->WriteLogEntry(m_szMessage,2);
			bRetVal = FALSE;
		}
		CReg.CloseRegKey(hComConfigKey);
	}
	else
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,lRes,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);
		sprintf(m_szMessage,"\tCMDBMaintenance::EnablePolling(): %s\tCould not open the CommServer Key",(LPTSTR)lpMsgBuf);
		m_Log->WriteLogEntry(m_szMessage,2);
		LocalFree(lpMsgBuf);
		bRetVal = FALSE;
	}
	return(bRetVal);
}

void CMDBMaintenance::BackupMDB()
{
	HANDLE hDbFile;
	DWORD dwDbFilSiz = 0;
	ULARGE_INTEGER iUserSpace,iTotalSpace,iDiskFree;
	LPVOID lpMsgBuf;
	CString strMsg;

	hDbFile = CreateFile(
		m_strDestinationDb,					// File to open
		GENERIC_READ,						// We are reading
		FILE_SHARE_READ,					// Others can read too
		NULL,
		OPEN_EXISTING,						// Open only if exists
		FILE_FLAG_SEQUENTIAL_SCAN,			// We are reading sequentially
		0);									// No template needed

	if(hDbFile != INVALID_HANDLE_VALUE)
	{
		dwDbFilSiz = GetFileSize(hDbFile,NULL); // This file should never be larger than 4 gig!
		CloseHandle(hDbFile);
		if(dwDbFilSiz > 0)
		{
			if(GetDiskFreeSpaceEx(m_strBackupPath,&iUserSpace,&iTotalSpace,&iDiskFree))
			{
				if(iUserSpace.QuadPart/dwDbFilSiz >= 4)
				{
					// Backup the corrupt database if it exists!
					// if it doesn't exist it doesn't matter because it will be recreated!
					CopyFile(m_strDestinationDb,
						m_strBackupPath + CTime::GetCurrentTime().Format("%Y%m%d%H%M%S") + "TB.mdb",false);
				}
				else // not enough free space
				{
					sprintf(m_szMessage,
						"\tCMDBMaintenance::BackupMDB(): Disk free space to low: %ul to backup %s",
							iUserSpace.QuadPart,m_strDestinationDb);
					m_Log->WriteLogEntry(m_szMessage,1);
				}
			}
			else
			{
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,0,NULL);
				strMsg = (LPTSTR)lpMsgBuf;
				sprintf(m_szMessage,"\tCMDBMaintenance::BackupMDB(): %s\tGetDiskFreeSpaceEx failed",strMsg);
				m_Log->WriteLogEntry(m_szMessage,4);
				LocalFree(lpMsgBuf);
			}
		}
		else
		{
			sprintf(m_szMessage,"\tCMDBMaintenance::BackupMDB(): failed %s is zero bytes",m_strDestinationDb);
			m_Log->WriteLogEntry(m_szMessage,4);
		}
	}
	else
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);
		strMsg = (LPTSTR)lpMsgBuf;
		sprintf(m_szMessage,"\tCMDBMaintenance::BackupMDB(): failed because:\n\t%s\t%s",strMsg,m_strDestinationDb);
		m_Log->WriteLogEntry(m_szMessage,4);
		LocalFree(lpMsgBuf);
	}
}

void CMDBMaintenance::ProcessTransDump()
{
	CRegistry CReg;
	HKEY hComSrvKey;
	long lRes = 0;
	CString sDumpFile;
	FILE *DumpFile = NULL;
	LPTSTR szLine;
	CString strMsg;

	hComSrvKey = CReg.OpenRegKey(&lRes,
		"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\CommunicationServer",HKEY_LOCAL_MACHINE);
	if(lRes == ERROR_SUCCESS)
	{
		if(!GetRegValue(hComSrvKey,sDumpFile,"TransDump"))
			sDumpFile = "C:\\LogFiles\\TRANSDUMP.LOG";
		CReg.CloseRegKey(hComSrvKey);
	}

	if(!m_daodb->IsOpen()) vDbOpen();

	if(m_daodb->IsOpen())
	{
		szLine = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,512);
		DumpFile = fopen(sDumpFile,"rt");
		// open the transdump file and process sequentially
		if(DumpFile != NULL)
		{
			while(!feof(DumpFile))
			{
				fgets(szLine,512,DumpFile);
				// this file has carriage returns and line feeds. remove them.
				memset(szLine + (strlen(szLine) - 1),0,1);
				if(!feof(DumpFile))
				{
					CString strSql = "INSERT INTO Transactions (Trans) VALUES (\"";
					strSql += szLine;
					strSql += "\")";
					try
					{
						m_daodb->Execute(strSql,dbFailOnError | dbSeeChanges);
					}
					catch(CDaoException *e)
					{
						// acknowledge only errors that are not duplicate primary keys!
						if(DBDAOERR(e->m_pErrorInfo->m_lErrorCode) != E_DAO_KeyDuplicate)
						{
							strMsg  = "\tCMDBMaintenance::ProcessTransDump() Failed: " + e->m_pErrorInfo->m_strSource \
								+ "\t" + e->m_pErrorInfo->m_strDescription;
							sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
							m_Log->WriteLogEntry ( m_szMessage, 1 );
						}
						e->Delete();
					}
				}
			}
			fclose(DumpFile);
		}
		LocalFree(szLine);
		vCloseDb();
	}
	else
	{
		strMsg  = "\tCMDBMaintenance::ProcessTransDump() Failed: could not open";
		strMsg += m_strTempDb;
		sprintf(m_szMessage,"%s",strMsg);
		m_Log->WriteLogEntry ( m_szMessage, 1 );
	}
}

void CMDBMaintenance::vDbOpen()
{
	BOOL bDbOpen = FALSE;
	int iRetries = 0;
	CString sTempDb = m_strTempDbPath + m_strTempDb;
	CString strMsg;

	do
	{
		try
		{
			m_daodb->Open(sTempDb,false,false,"");
			bDbOpen = TRUE;
		}
		catch(CDaoException *e)
		{
			strMsg  = "\tCMDBMaintenance::bDbOpen()  Failed: " \
					+ e->m_pErrorInfo->m_strSource \
					+ "\t" + e->m_pErrorInfo->m_strDescription;
			sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
			m_Log->WriteLogEntry ( m_szMessage, 1 );

			iRetries++;
			e->Delete();
		}
		Sleep(100);
	} while(!bDbOpen && iRetries < 5);
}

void CMDBMaintenance::vCloseDb()
{
	CString strMsg;

	try
	{
		if(m_daodb->IsOpen())
			m_daodb->Close();
	}
	catch(CDaoException *e)
	{
		strMsg  = "\tCMDBMaintenance::bCloseDb()  Failed: " \
				+ e->m_pErrorInfo->m_strSource \
				+ "\t" + e->m_pErrorInfo->m_strDescription;
		sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
		m_Log->WriteLogEntry ( m_szMessage, 1 );
		e->Delete();
		return;
	}
}

void CMDBMaintenance::MoveMDB()
{
	CString sTempDb = m_strTempDbPath + m_strTempDb;
	CString strMsg;
	bool bMdbCopied = false;

	int iRetry = 0;

	// m_bRetval is used only here and in InitMdb since it is a global!
	if(m_bRetVal)
	{
		do
		{
			try
			{
				// Move the temporary mdb to the destination
				m_daodb->m_pWorkspace->CompactDatabase(sTempDb,m_strDestinationDb,dbLangGeneral);
				bMdbCopied = true;
			}
			catch(CDaoException* e)
			{
				strMsg  = "\tCMDBMaintenance::~CMDBMaintenance(): CompactDatabase Failed: " \
					+ e->m_pErrorInfo->m_strSource + "\t" + e->m_pErrorInfo->m_strDescription;
				sprintf(m_szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);

				if(DBDAOERR(e->m_pErrorInfo->m_lErrorCode) == E_DAO_DatabaseDuplicate && iRetry == 4)
					m_Log->WriteLogEntry ( m_szMessage, 1 );
				if(DBDAOERR(e->m_pErrorInfo->m_lErrorCode) != E_DAO_DatabaseDuplicate)
					m_Log->WriteLogEntry ( m_szMessage, 1 );

				// If the destination database exists then delete it and retry the compact
				if(DBDAOERR(e->m_pErrorInfo->m_lErrorCode) == E_DAO_DatabaseDuplicate)
					if(!DeleteFile(m_strDestinationDb))
					{
						LPVOID lpMsgBuf;

						FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
							FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
							(LPTSTR) &lpMsgBuf,0,NULL);
						strMsg = (LPTSTR)lpMsgBuf;
						sprintf(m_szMessage,"\tCMDBMaintenance::~CMDBMaintenance(): %s\tCould not delete %s",strMsg,sTempDb);
						m_Log->WriteLogEntry ( m_szMessage, 1 );
						LocalFree(lpMsgBuf);
					}
				e->Delete();
				iRetry++;
			}
			Sleep(100);
		} while(!bMdbCopied && iRetry < 5);
	}

	if(bMdbCopied) DeleteFile(sTempDb);

}
