// GetNewXact.cpp
///////////////////////////////////////////////////////////////
// This function calls the TransDll function CommDirGetNewXact
// that retreives the next transaction in the buffer.
///////////////////////////////////////////////////////////////

#define STATUS_READ_TRANS TRUE
#define STATUS_NEW_TRANS FALSE

#include "stdafx.h"
#include "Registry.h"
#include "Crack.h"

void ErrMsg ( CString &, long );

bool bOpenDBDao(CDaoDatabase *,CString,bool,bool);
CString GetNextXact(CDaoDatabase *);
/*  //  EVS - Removed old GetNewXact which is the registry for the Transaction Buffer
int GetNewXact ( CString &strBuffer, int iDebugLevel )
{
	CRegistry *Reg;
	long lResults;
	DWORD dwNextToRead, dwNumTrans;
	HKEY hBaseKey, hTransKey;
	char szKeyName[20] = "Tran";
	char szBuffer[10];

	Reg = new CRegistry;

	hBaseKey = Reg->OpenRegKey(&lResults, 
		"SOFTWARE\\CTRSystems\\PARCS-CS\\Transactions", 
		HKEY_LOCAL_MACHINE);

	dwNextToRead = Reg->FetchDWValue(&lResults, hBaseKey, "NextToRead");
	dwNumTrans = Reg->FetchDWValue(&lResults, hBaseKey, "NumTrans");

	_ultoa(dwNextToRead, szBuffer, 10);
	strcat(szKeyName, szBuffer);

	hTransKey = Reg->OpenRegKey(&lResults, szKeyName, hBaseKey);

	if (lResults != ERROR_SUCCESS)
	{
		Reg->CloseRegKey(hTransKey);
		Reg->CloseRegKey(hBaseKey);
		delete Reg;
		return 1;
	}

	if (Reg->FetchBoolValue(&lResults, hTransKey, "Status") != STATUS_NEW_TRANS)
	{
		Reg->CloseRegKey(hTransKey);
		Reg->CloseRegKey(hBaseKey);
		delete Reg;
		return 2;
	}


	strBuffer = Reg->FetchStrValue(&lResults, hTransKey, "Trans");

	BOOL bStatus = STATUS_READ_TRANS;
	Reg->SetRegValue(hTransKey, "Status", REG_BINARY, 
		(CONST BYTE *) &bStatus, sizeof(BOOL));
	Reg->CloseRegKey(hTransKey);

	dwNextToRead++;
	if (dwNextToRead == dwNumTrans)
		dwNextToRead = 0;
	Reg->SetRegValue(hBaseKey, "NextToRead", REG_DWORD, 
		(CONST BYTE *) &dwNextToRead, sizeof(DWORD));
	Reg->CloseRegKey(hBaseKey);
	Sleep( 100 );

	delete Reg;
	return 0;
}

int UpdateRegFlags ( int iDebugLevel )
{
	CRegistry *Reg;
	long lResults;
	BOOL bStatus = STATUS_READ_TRANS;
	DWORD dwNextToRead, dwNumTrans;
	HKEY hBaseKey, hTransKey;
	char szKeyName[20] = "Tran";
	char szBuffer[10];

	Reg = new CRegistry;

	hBaseKey = Reg->OpenRegKey(&lResults, 
		"SOFTWARE\\CTRSystems\\PARCS-CS\\Transactions", 
		HKEY_LOCAL_MACHINE);

	dwNextToRead = Reg->FetchDWValue(&lResults, hBaseKey, "NextToRead");
	dwNumTrans = Reg->FetchDWValue(&lResults, hBaseKey, "NumTrans");

	_ultoa(dwNextToRead, szBuffer, 10);
	strcat(szKeyName, szBuffer);

	hTransKey = Reg->OpenRegKey(&lResults, szKeyName, hBaseKey);

	if (lResults != ERROR_SUCCESS)
	{
		Reg->CloseRegKey(hTransKey);
		Reg->CloseRegKey(hBaseKey);
		delete Reg;
		return 1;
	}

	if (Reg->FetchBoolValue(&lResults, hTransKey, "Status") != STATUS_NEW_TRANS)
	{
		Reg->CloseRegKey(hTransKey);
		Reg->CloseRegKey(hBaseKey);
		delete Reg;
		return 2;
	}


	Reg->SetRegValue(hTransKey, "Status", REG_BINARY, 
		(CONST BYTE *) &bStatus, sizeof(BOOL));

	Reg->CloseRegKey(hTransKey);

	dwNextToRead++;

	if (dwNextToRead == dwNumTrans)
		dwNextToRead = 0;

	Reg->SetRegValue(hBaseKey, "NextToRead", REG_DWORD, 
		(CONST BYTE *) &dwNextToRead, sizeof(DWORD));

	Reg->CloseRegKey(hBaseKey);
	delete Reg;

	return 0;
}
*/

int GetNewXact ( CString &strBuffer, int iDebugLevel )
{
	CString strMdb;
	CRegistry *Reg;
	long lRetVal;
	HKEY hBaseKey;
	char *szKeyName = "BufferLoc";
	char szMessage[512];
	CString strMsg;

	Reg = new CRegistry;

	hBaseKey = Reg->OpenRegKey(&lRetVal, 
		"SOFTWARE\\CTRSystems\\PARCS-CS\\Transactions", 
		HKEY_LOCAL_MACHINE);
	if ( lRetVal != ERROR_SUCCESS )
	{
		strMdb = "C:\\LogFiles\\TransBuff.mdb";
		sprintf(szMessage," \tCould not open hBaseKey: Using program default: %s",strMdb);
		strMsg = szMessage;
		ErrMsg(strMsg,4);
	}  //errMsg EVS

	strMdb = Reg->FetchStrValue(&lRetVal, hBaseKey, szKeyName);
	if( lRetVal != ERROR_SUCCESS )
	{
		strMdb = "C:\\LogFiles\\TransBuff.mdb";
		sprintf(szMessage,"Could not Fetch Registry Value: Using program default: %s",strMdb);
		strMsg = szMessage;
		ErrMsg(strMsg,4);
	}  //errMsg EVS

	if(lRetVal == ERROR_SUCCESS)
	{
//		sprintf(szMessage,"Successfully retrieved MDB Path: %s",strMdb);
//		strMsg = szMessage;
//		ErrMsg(strMsg,4);
	}

	Reg->CloseRegKey(hBaseKey);
	delete Reg;

	CDaoDatabase *dbTransBuf = new CDaoDatabase;
	if(bOpenDBDao(dbTransBuf,strMdb,false,true))
	{
		// GetNextXact read TransBuff.mdb and return the new transactions.
		strBuffer = GetNextXact(dbTransBuf);
		sprintf(szMessage,"GetNextXact returned transaction: ");
		strMsg = szMessage;
		strMsg += strBuffer;
		dbTransBuf->Close();
	}
	delete dbTransBuf;
	AfxDaoTerm();

	return(0); //check EVS
}


CString GetNextXact(CDaoDatabase *dbTransBuf)
{
	CDaoRecordset dbTransRec(dbTransBuf);
	CString strSql,strResult;
	long lNumRecs = 0;

	// SELECT all records where the DateInserted is NULL!
	// This could be changed to read a chunk of records into a CStringArray.
	// and probably eliminate the need for a multi-thread Transaction Writer.
	strSql = "SELECT * FROM Transactions WHERE (DateInserted = NULL) ORDER BY DateRecvd";

	try
	{
		dbTransRec.Open(dbOpenSnapshot,strSql,dbReadOnly | dbSeeChanges);
	}
	catch(CDaoException *e)
	{
		CString strMsg;
		char szMessage[512];

		strMsg  = "GetNextXact.Open failed: " + e->m_pErrorInfo->m_strSource \
					+ "\n" + e->m_pErrorInfo->m_strDescription;
		sprintf(szMessage,"%s\n%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
		ErrMsg(strMsg,e->m_pErrorInfo->m_lErrorCode);
		e->Delete();
		return("");
	}

	try
	{
		lNumRecs = dbTransRec.GetRecordCount();
	}
	catch(CDaoException *e)
	{
		CString strMsg;
		char szMessage[512];
		lNumRecs = 0;
		strMsg  = "GetNextXact.GetRecordCount Failed: " + e->m_pErrorInfo->m_strSource \
					+ "\n" + e->m_pErrorInfo->m_strDescription;
		sprintf(szMessage,"%s\n%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
		ErrMsg(strMsg,e->m_pErrorInfo->m_lErrorCode);
		e->Delete();
		return("");
	}
		
	if(lNumRecs > 0)
	{
		COleVariant vTrans;
		dbTransRec.GetFieldValue("Trans",vTrans);
		// CCrack take the COleVariant and converts it to a CString.
		strResult = CCrack::strVARIANT(vTrans);
	}
	else
		strResult = "";

	if(dbTransRec.IsOpen())
	{
		dbTransRec.Close();
	}

	return(strResult);
}
