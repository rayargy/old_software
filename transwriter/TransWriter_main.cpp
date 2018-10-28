#include "stdafx.h"
#include <afxdb.h>
#include <fstream.h>
#include <odbcinst.h>

#include "CBuffer.h"
#include "CTables.h"
#include "Registry.h"
#include "EventLog.h"
#include "PARCSMSGS.h"
#include "ServiceMain.h"
#include "CreditCardXact.h"
#include "ProcessDumpInfo.h"

//#define QUERY_TIMEOUT 90	// 90 Seconds
#define QUERY_TIMEOUT 300	// 90 Seconds changed because queries were timing out

// Added for Limit Alarms
// April 1, 1998 - mlv
#include "LimitDefs.h"
// End April 1, 1998 modification

void	Exception	( CString & );
void	ErrorHandler( char *, DWORD );
//int		DumpTrans	( ); // EVS no longer needed with TransBuff.mdb
void	ErrMsg		( CString &, long );
int		GetNewXact	( CString &, int );
int		UpdateRegFlags ( int iDebugLevel );
long	MaxProtocol	( CDatabase *, CString & );
int		WriteTransactionToDb(int , CString , CDatabase *);
// void	ProcessDumpRecs ( int , CString ); //EVS no longer needed with TransBuff
int		ProcessTransaction(int , CString);
//UINT	ProcessWriteFailures( LPVOID pParam ); //EVS no longer needed with MDB
UINT    ProcessDBTransactions( LPVOID pParam );

// Added for using TransBuff.mdb
bool bOpenDBDao(CDaoDatabase *,CString,bool,bool);
void PostDBTrans(CString);
void UpdateTransStatus(CDaoDatabase *,CString);
void PurgeOldXacts(void);
CString CalcDeleteTime(CString,CString);

#define MAX_ITERATIONS 999

CString strXDataFile;
CString strConnect;

int TransWriter_main (CServiceMain *pInstanceRef )
{
//--------------------------------------------------------------
// This function is the main Transaction Writer function.
//--------------------------------------------------------------

	int iDebugLevel = 0, iretVal = 0, iIterCtr = MAX_ITERATIONS + 1;
	int i = 0;
	BOOL bNoDumpFile = false, bPost = false, bDBConnected = false;
	long lretVal = 0, lPollDelay = 0;
	fstream fin, fout;
	CString strDataFile, strDebugFile, strBuffer, strMsg;
//	char *strPath			= { "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\TransactionWriter" };
	char strPath[512] = { "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\TransactionWriter" };
	CRegistry CReg;
	CEventLog CLog( "TransWriter" );

	// Get the Debug Level and Polling Delay from the System Registry.
	HKEY hKey   = CReg.OpenRegKey   ( &lretVal, strPath, HKEY_LOCAL_MACHINE );
	if ( lretVal )
	{
		bPost = CLog.PostError ( MSG_TWRT_REG_ERR, NULL, 0 );
		iDebugLevel = 0;
		lPollDelay = 500;
		strConnect = "DSN=PARCSDatabase;UID=parcs;PWD=parcs";
		strDataFile = "C:\\TransWriter.dat";
		strXDataFile = "C:\\TransWriterXdata.txt";
		strDebugFile = "C:\\TransWriterErr.txt";
	}
	else
	{
		iDebugLevel = (int)  CReg.FetchDWValue ( &lretVal, hKey, "DebugLevel" );
		lPollDelay  = (long) CReg.FetchDWValue ( &lretVal, hKey, "PollDelay" );
		if ( lretVal )
			lPollDelay = 500;
		strConnect = CReg.FetchStrValue ( &lretVal, hKey, "DBConnect" );
		if ( lretVal )
			strConnect = "DSN=PARCSDatabase;UID=parcs;PWD=parcs";
		strDataFile = CReg.FetchStrValue ( &lretVal, hKey, "DataFile" );
		if ( lretVal )
			strDataFile = "C:\\TransWriter.dat";
		strXDataFile = CReg.FetchStrValue ( &lretVal, hKey, "XDataFile" );
		if ( lretVal )
			strXDataFile = "C:\\TransWriterXdata.txt";
		strDebugFile = CReg.FetchStrValue ( &lretVal, hKey, "DebugFile" );
		if ( lretVal )
			strDebugFile = "C:\\TransWriterErr.txt";
	}
	CReg.CloseRegKey(hKey);

	if ( iDebugLevel )
	{
		strMsg.Format ( "Enter Transaction Writer, Debug Level = %d", iDebugLevel );
		ErrMsg ( strMsg, 0 );
	}

	// Enter the Main Pooling Loop
	if ( iDebugLevel > 2)
	{
		strMsg = "Enter Main Pooling Loop";
		ErrMsg ( strMsg, 3 );
	}

	while (pInstanceRef->m_bRunningService)
	{
/*		if (iIterCtr++ > MAX_ITERATIONS)  //EVS no longer needed with TransBuff
		{
//			ProcessDumpRecs( iDebugLevel, strConnect );
//			CProcessDumpInfo DumpInfo;
			CProcessDumpInfo *DumpInfo = new CProcessDumpInfo;
			DumpInfo->SetDebugLevel( iDebugLevel );
			DumpInfo->SetConnectString( strConnect );
			AfxBeginThread(ProcessWriteFailures, (LPVOID) DumpInfo);
			Sleep( 1000 );

			iIterCtr = 0;
		}  */

		// Necessary to clear out any remaining 
		// transactions from the previous iteration.
		strBuffer.Empty ( );

		// Get Transaction From The Buffer.
		if ( iDebugLevel > 2 )
		{
			strMsg = "Get Transaction From The Buffer";
			ErrMsg ( strMsg, 3 );

			CRegistry BeepReg;
			long RetVal = NO_ERROR;
			
			HKEY hOpenKey = BeepReg.OpenRegKey(&RetVal, 
				"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\TransactionWriter", HKEY_LOCAL_MACHINE);
			DWORD dwBeepVal = BeepReg.FetchDWValue(&RetVal, hOpenKey, "BeepValue");
			if (dwBeepVal >= 0x25 && dwBeepVal <= 0x7FFF)
				Beep(dwBeepVal,200);			// This is the 'heart beat'
			BeepReg.CloseRegKey(hOpenKey);
		}

		iretVal = GetNewXact ( strBuffer, iDebugLevel );
		if ( iretVal < 0 )
		{
			bPost = CLog.PostError ( MSG_TWRT_TRANSDLL, NULL, 0 );
			return  0;
		}
		if ( iretVal > 0 )
		{
			iretVal = 0;
			strBuffer.Empty();
		}
		if ( strBuffer.IsEmpty ( ) )
			Sleep ( lPollDelay );
		else
		{
			if ( iDebugLevel > 2 )
			{
				strMsg = "Input Buffer: " + strBuffer;
				ErrMsg ( strMsg, 3 );
			}

			// Process the transaction
//			ProcessTransaction(iDebugLevel, strBuffer);
//			CProcessDumpInfo TranstoDB;
			CProcessDumpInfo *TranstoDB = new CProcessDumpInfo;
			TranstoDB->SetDebugLevel( iDebugLevel );
			TranstoDB->SetConnectString( strBuffer );
			AfxBeginThread(ProcessDBTransactions, (LPVOID) TranstoDB);
			Sleep(400);


		}
		PurgeOldXacts();
	} // End Main While Loop
	if ( iDebugLevel > 0)
	{
		strMsg = "Exit Main Pooling Loop";
		ErrMsg ( strMsg, 0 );
	}

	return  1;
}

int ProcessTransaction(int pintDebugLevel, CString pstrBuffer)
{
	int iRetVal = -1, intDBConnectionTries = 0;
	BOOL bDBConnected = false;
	CDatabase *db = new CDatabase;
	CString strMsg;
	CEventLog CLog( "TransWriter" );

	// Open the PARCS Database.
	//  This routine will attempt to connect to the db.  If it fails,
	// it will attempt connection again after a 5 second delay up to
	// 5 times.  If the database can not be opened by then, the 
	// service will shut down.
	do
	{
		try
		{
			BOOL open = db->OpenEx ( strConnect, CDatabase::noOdbcDialog );
		}
		catch ( CDBException* e )
		{
			// The error code is in e->m_nRetCode
			// The error description is in e->m_strError

			strMsg  = "ODBC Database Connection Failed: " + e->m_strStateNativeOrigin + e->m_strError;
			ErrMsg ( strMsg, (long) e->m_nRetCode );
			BOOL bPost = CLog.PostError ( MSG_TWRT_DBOPEN, NULL, 0 );
//			iRetVal = DumpTrans ( ); //EVS no longer needed with TransBuff.mdb
			intDBConnectionTries++;
			e->Delete();
			return(-1);
		}

		if (db->IsOpen())
			bDBConnected = true;
		else if (intDBConnectionTries < 5)
			Sleep(5000);
	}
	while ((! bDBConnected) && intDBConnectionTries < 5);

	if (bDBConnected)
	{
		iRetVal = WriteTransactionToDb(pintDebugLevel, pstrBuffer, db);
		if(iRetVal >= 0)
			PostDBTrans(pstrBuffer);
//		UpdateRegFlags ( pintDebugLevel );
		db->Close ();
//		delete db; // EVS This should be deleted regardless of whether the DB
					// was opened or not!
	}
	delete db;
	return iRetVal;
}

int WriteTransactionToDb(int pintDebugLevel, CString pstrBuffer, CDatabase *db)
{
	CTables CTbls (pintDebugLevel);
	CBuffer CBuff (pintDebugLevel);
	BOOL bPost, bCreditCard = false;
	CParse CommData (pstrBuffer, ',', ';');
	CString strMsg, strTable, strValues, strFields, strSQL;
	int intResponseCode, intTransType;
	CEventLog CLog("TransWriter");
	int iCreditCardIndex = 0;
	CString sTempDate;
	int iYear;
	int iMonth;
	int iIndex;

	if (CommData.Count() < 5)
	{
		Exception (pstrBuffer);
		return -1;
	}
	if (pintDebugLevel > 2)
	{
		strMsg = "Input Buffer: " + pstrBuffer;
		ErrMsg( strMsg, 3);
	}
	intResponseCode = atoi(CommData.GetField(2));
	if (pintDebugLevel > 3)
	{
		strMsg.Format ("Response Code = %d", intResponseCode);
		ErrMsg(strMsg, 3);
	}
	intTransType = atoi(CommData.GetField(3));
	if (pintDebugLevel > 3)
	{
		strMsg.Format("Transaction Type = %d", intTransType);
		ErrMsg(strMsg, 3);
	}

	// Since credit card xactions use 81 and 82, commenting this
	// out and adjusting boundaries.  1.July.98 -Dar
	// if ((intTransType < 1 || intTransType > 79) && (intTransType != LIMIT_TRANS)) 
	if ((intTransType < 1 || intTransType > 82) && (intTransType != LIMIT_TRANS)) 
	{
		intTransType = 0; 
		return -1;
	}

	if (! intTransType && pintDebugLevel > 0)
	{
		strMsg = "Invalid Transaction Type: ";
		strMsg += pstrBuffer;
		ErrMsg(strMsg, 1);
		return -1;
	}


	if (intTransType == 81 || intTransType == 82)
	{
		// These two credit card transactions that will be
		// handled in this later, separate phase of development,
		// beginning 1.July.1998, will be handled entirely by
		// the following instance of CCreditCardXact.  This is to
		// circumvent the (in my opinion) confusing and inflexible
		// use of the CBuffer and CTables classes. -Dar
		CCreditCardXact *lpCreditCard;

		lpCreditCard = new CCreditCardXact;
		
		strSQL = lpCreditCard->GetSaveXactString(&CommData);

		delete lpCreditCard;

		try
		{
			db->ExecuteSQL(strSQL);
		}
		catch (CDBException* e)
		{
			int iRetVal = e->m_strError.Find("Attempt to insert duplicate key in object");
			strMsg  = "Error saving CreditCardXact: " + e->m_strStateNativeOrigin + e->m_strError + "  SQL: " + strSQL;
			Exception(pstrBuffer);
			ErrMsg(strMsg, (long)e->m_nRetCode);
			bPost = CLog.PostError(MSG_TWRT_DB_TBLOPEN, NULL, 0);
			e->Delete();
//			return -1; //EVS since the possibility of duplicates exists,
						// and CDBException does not return anything but
						// strings to indicate a duplicate. m_nRetCode
						// is returned as a -1 when a duplicate occur
						// but it could also return a -1 for other errors.
			// Hence, the error strings must be searched for the following text.
			return(iRetVal);
		}

		return 0;
	}

	CBuff.SetCommData(CommData);
	strTable = CTbls.GetTable(intTransType);
	if (pintDebugLevel > 3)
	{
		strMsg = "Database Table = " + strTable;
		ErrMsg (strMsg, 4);
	}
	if (strTable.IsEmpty())
	{
		if (pintDebugLevel > 0)
		{
			strMsg = "Invalid Database Table: " + pstrBuffer;
			ErrMsg(strMsg, 1);
		}
		return -1;
	}
	CBuff.SetTable(strTable);
	CBuff.SetData(CommData);
	bCreditCard = CBuff.GetCredit();
	if (pintDebugLevel > 3)
	{
		if (bCreditCard) strMsg = "Credit Card info";
		else	strMsg = "No Credit Card info";
		ErrMsg(strMsg, 4);
	}
	CTbls.SetCredit(bCreditCard);
	strFields = CTbls.GetFields();
	strValues = CBuff.GetValues();
	if (strTable == "AlarmTrans")
	{
		strValues = strValues.Left(strValues.GetLength() - 1);
		strValues += ",'" + pstrBuffer + "')";
	}
	strSQL.Empty();
	strSQL.FreeExtra();
	strSQL = "INSERT INTO " + strTable + strFields + "VALUES" + strValues;
	if (pintDebugLevel > 2)
	{
		strMsg = strSQL;
		ErrMsg(strMsg, 3);
	}

	db->SetQueryTimeout(QUERY_TIMEOUT); //Set New Timeout for Query

	try
	{
		db->ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		int iRetVal = e->m_strError.Find("Attempt to insert duplicate key in object");
		strMsg  = "ODBC ERROR: " + e->m_strStateNativeOrigin + e->m_strError + "  SQL: " + strSQL;
		Exception(pstrBuffer);
		ErrMsg(strMsg, (long)e->m_nRetCode);
		bPost = CLog.PostError(MSG_TWRT_DB_TBLOPEN, NULL, 0);
		e->Delete();
		return(iRetVal);
	}
	
	/*  
		8.Jan.98 Added to save credit card info
		in CreditCardInfo table (since it is now
		stored in a separate table from RevenueTrans,
		formerly parker_trans).
	*/
	/*
	if (CTbls.bCreditCard)
	{
		strFields = CTbls.GetCreditCardFields();
		strValues = CBuff.GetCreditCardValues();
		strSQL = "INSERT INTO CreditCardInfo " + strFields + " VALUES " + strValues;

		if (pintDebugLevel > 2)
		{
			strMsg = strSQL;
			ErrMsg(strMsg, 3);
		}

		try
		{
			db.ExecuteSQL(strSQL);
		}
		catch (CDBException* e)
		{
			strMsg  = "ODBC ERROR: " + e->m_strStateNativeOrigin + e->m_strError + "  SQL: " + strSQL;
			Exception(pstrBuffer);
			ErrMsg(strMsg, (long)e->m_nRetCode);
			bPost = CLog.PostError(MSG_TWRT_DB_TBLOPEN, NULL, 0);
			e->Delete();
		}
	}
	*/

	bCreditCard = false;

	for (iIndex = 0; iIndex < CommData.Count(); iIndex++)
	{
		/*
		strMsg.Format("%d \t %s", iIndex, CommData.GetField(iIndex));
		ErrMsg(strMsg, 0);
		*/

		if (CommData.GetField(iIndex) == "!")
		{
			bCreditCard = true;
			iCreditCardIndex = iIndex;
			strMsg = "This is a credit card transaction!";
			ErrMsg(strMsg, 0);
		}
	}

	if (bCreditCard)
	{
		iCreditCardIndex++;

		strFields.Empty();
		strValues.Empty();

		CTbls.CommonFields(strFields);

		strFields = "StationType, StationAddress, Protocol, DateTime";
		strValues = CBuff.GetCommonValues();

		strFields += ", CreditCardNumber";
		strValues += ", '" + CommData.GetField(iCreditCardIndex++).Left(25) + "'";

		// The "StartDate" is next, but it is typically sent 
		// as "0000", which is not a valid date of course.
		sTempDate = CommData.GetField(iCreditCardIndex++);

		if (sTempDate == "0000")
			sTempDate.Empty();
		else
		{
			iYear = atoi(sTempDate.Left(2)) + 1900;
			if (iYear < 1935)
				iYear += 100;
	
			iMonth = atoi(sTempDate.Mid(2));

			if (iMonth == 0)
				iMonth = 1;

			sTempDate.Format("%d-%d-1 12:00:00", iYear, iMonth);
		}

		strFields += ", StartDate";
		strValues += ", '" + sTempDate + "'";

		// ExpirationDate can also come across as a 
		// "0000" date in transactions like 41.
		sTempDate = CommData.GetField(iCreditCardIndex++);

		if (sTempDate == "0000")
			sTempDate.Empty();
		else
		{
			iYear = atoi(sTempDate.Left(2)) + 1900;
			if (iYear < 1935)
				iYear += 100;

			iMonth = atoi(sTempDate.Mid(2));
			iMonth++;

			if (iMonth > 12)
			{
				iMonth = 1;
				iYear++;
			}

			sTempDate.Format("%d-%d-1 12:00:00", iYear, iMonth);
		}

		strFields += ", ExpirationDate";
		strValues += ", '" + sTempDate + "'";

		strFields += ", AuthorizationCode";
		strValues += ", '" + CommData.GetField(iCreditCardIndex++) + "'";

		strFields += ", TranNumber";
		strValues += ", '" + CommData.GetField(iCreditCardIndex++) + "'";

		strFields += ", ServiceCode";
		strValues += ", '" + CommData.GetField(iCreditCardIndex++) + "'";

		strFields += ", RequestFlag";
		strValues += ", '" + CommData.GetField(iCreditCardIndex++) + "'";

		strFields += ", ResultCode";
		strValues += ", '" + CommData.GetField(iCreditCardIndex++).Left(3) + "'";

		strFields += ", ErrorFlag";
		strValues += ", 0" + CommData.GetField(iCreditCardIndex++);

		strFields += ", CreditError";
		strValues += ", 0" + CommData.GetField(iCreditCardIndex++);

		strFields += ", Cashier";
		strValues += ", 0" + CommData.GetField(iCreditCardIndex++);

		strFields += ", Shift";
		strValues += ", 0" + CommData.GetField(iCreditCardIndex++);

		strFields += ", KPTrace";
		strValues += ", '" + CommData.GetField(iCreditCardIndex) + "'";

		/* *******Uncomment if we want to add TranID to CreditCardInfo
		strFields += ", TranID";
		strValues += ", " + CommData.GetField ( 3 );
		*/

		strSQL = "INSERT INTO CreditCardInfo (" + strFields + ") VALUES (" + strValues + ")";

		if (pintDebugLevel > 2)
		{
			strMsg = strSQL;
			ErrMsg(strMsg, 3);
		}

		try
		{
			db->ExecuteSQL(strSQL);
		}
		catch (CDBException* e)
		{
			int iRetVal = e->m_strError.Find("Attempt to insert duplicate key in object");
			strMsg  = "ODBC ERROR: " + e->m_strStateNativeOrigin + e->m_strError + "  SQL: " + strSQL;
			Exception(pstrBuffer);
			ErrMsg(strMsg, (long)e->m_nRetCode);
			bPost = CLog.PostError(MSG_TWRT_DB_TBLOPEN, NULL, 0);
			e->Delete();
			return(iRetVal);
		}

	}
	return 0;
}

void Exception ( CString &strRecord )
{
//--------------------------------------------------------------------
// This function writes bad transaction records to an exception file.
//--------------------------------------------------------------------

	FILE *ExceptFile;

	ExceptFile = fopen ( strXDataFile, "a" );
	if ( ExceptFile != NULL )
	{
		fprintf ( ExceptFile, "%s\n", strRecord );
		fclose  ( ExceptFile );
	}
}

void ErrMsg ( CString &msg, long lErrNo )
{
//--------------------------------------------------------------
// This function writes debug messages to the error log.
// The current date/time is included in the message.
//--------------------------------------------------------------

	FILE *ErrorFile;
	long lretVal = 0;
//	char *strPath = { "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\TransactionWriter" };
	char strPath[512] = { "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\TransactionWriter" };
	CString strDebugFile;
	CRegistry CReg;

	// Get the Debug File Name from the System Registry.
	HKEY hKey   = CReg.OpenRegKey   ( &lretVal, strPath, HKEY_LOCAL_MACHINE );
	if ( lretVal )
		strDebugFile	= "C:\\TransWriterErr.txt";
	else
	{
		strDebugFile = CReg.FetchStrValue ( &lretVal, hKey, "DebugFile" );
		if ( lretVal )
			strDebugFile = "C:\\TransWriterErr.txt";
	}
	CReg.CloseRegKey(hKey);

	CTime t = CTime::GetCurrentTime();
	CString strMsg = t.Format ( "%Y-%m-%d %H:%M:%S" );

	DWORD dwThreadID = AfxGetThread()->m_nThreadID;
	ErrorFile = fopen ( strDebugFile, "a" );
	if ( ErrorFile != NULL )
	{
		fprintf ( ErrorFile, "%d:\t%s\n%s\nError Number: %d\n" , dwThreadID, strMsg, msg, lErrNo );
		fclose  ( ErrorFile );
	}
}
/*  //EVS no longer needed with TransBuff.mdb
void ProcessDumpRecs ( int iDebugLevel, CString strConnect )
{
	// ProcessDumpRecs is used to run through the XData file
	// and attempt to process any transactions found there.
	HANDLE hDumpFile;
	OFSTRUCT udtFileParams;
	char szTempFile[MAX_PATH + 2], cCharacter;
	UINT uResult = 0;
	bool bEndOfFile = false, bEndOfTrans, bDBConnected = false;
	BOOL bPost = false;
	CString sTransaction, sMsg, strMsg;
	DWORD dwBytesRead;
	int intDBConnectionTries = 0;
	CEventLog CLog( "TransWriter" );
	CDatabase *db = new CDatabase;

	if ( iDebugLevel )
	{
		strMsg = "Entering ProcessDumpRecs";
		ErrMsg ( strMsg, iDebugLevel );
	}

	if ( iDebugLevel > 2)
	{
		strMsg = "Attempting to open Database";
		ErrMsg ( strMsg, 3 );
	}

	// Open the PARCS Database.
	//  This routine will attempt to connect to the db.  If it fails,
	// it will attempt connection again after a 5 second delay up to
	// 5 times.  If the database can not be opened by then, the 
	// service will shut down.
	do
	{
		try
		{
			BOOL open = db->OpenEx ( strConnect, CDatabase::noOdbcDialog );
		}
		catch ( CDBException* e )
		{
			// The error code is in e->m_nRetCode
			// The error description is in e->m_strError

			strMsg  = "ODBC Database Connection Failed: " + e->m_strStateNativeOrigin + e->m_strError;
			ErrMsg ( sMsg, (long) e->m_nRetCode );
			bPost = CLog.PostError ( MSG_TWRT_DBOPEN, NULL, 0 );
			intDBConnectionTries++;
			e->Delete();
		}

		if (db->IsOpen())
			bDBConnected = true;
		else if (intDBConnectionTries < 5)
			Sleep(5000);
	}
	while ((!bDBConnected) && intDBConnectionTries < 5);

	if (!bDBConnected)
	{
		delete db;
		return;
	}

	udtFileParams.cBytes = sizeof(udtFileParams);

	if ( iDebugLevel > 2)
	{
		strMsg = "Checking existence of XDataFile";
		ErrMsg ( strMsg, 3 );
	}
	// Check to see if XData file exists
	HFILE hChkFile = OpenFile( strXDataFile, &udtFileParams, OF_EXIST );

	// If it does, continue
	if ( hChkFile == HFILE_ERROR )
	{
		db->Close ();					// Close the database
		delete db;
		return;
	}

	_lclose(hChkFile);

	if ( iDebugLevel > 2)
	{
		strMsg = "Getting Temporary File Name";
		ErrMsg ( strMsg, 3 );
	}

	// It exists, so get a temporary file name
	uResult = GetTempFileName( "c:\\", "twd", 0, szTempFile );

	// If GetTempFileName fails, quit
	if (uResult == 0)
	{
		db->Close ();					// Close the database
		delete db;
		return;
	}

	// Since we specified 0, the file was created.  Now delete it.
	DeleteFile ( szTempFile );

	if ( iDebugLevel > 2)
	{
		strMsg = "Renaming XData File";
		ErrMsg ( strMsg, 3 );
	}

	// Now, rename the XData file to the Temp File.
	MoveFile( strXDataFile, szTempFile );

	if ( iDebugLevel > 2)
	{
		strMsg = "Opening renamed dump file";
		ErrMsg ( strMsg, 3 );
	}

	// Open the Temp File for reading the transactions
	hDumpFile = CreateFile(
		szTempFile,											// File to open
		GENERIC_READ,										// We are reading
		FILE_SHARE_READ,									// Others can read too
		NULL,												// We will be us
		OPEN_EXISTING,										// Open only if exists
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,	// We are reading sequentially
		0);												// No template needed

	// Loop to read the file.
	while ( bEndOfFile == false)
	{
		sTransaction = "";
		bEndOfTrans = false;
		while ( bEndOfTrans == false)
		{
			ReadFile(hDumpFile, &cCharacter, 1, &dwBytesRead, NULL);
			if (dwBytesRead)
			{
				switch ( cCharacter )
				{
					case 0xD:				// Return!
					case 0xA:				// Line Feed!
						bEndOfTrans = true;
						break;
					default:
						sTransaction += cCharacter;
				}
			}
			else
			{
				bEndOfFile = true;
				bEndOfTrans = true;
			}
		}
		if ( sTransaction.GetLength() > 0 && bEndOfFile == false)
		{
			if (bDBConnected)
				WriteTransactionToDb(iDebugLevel, sTransaction, db);
		}
	}
	// We are done, so close the file
	CloseHandle ( hDumpFile );
	// Get rid of Temp File.
	DeleteFile ( szTempFile );
	db->Close ();					// Close the database
	delete db;

	if ( iDebugLevel )
	{
		strMsg = "ProcessDumpRecs Done.";
		ErrMsg ( strMsg, iDebugLevel );
	}
}  */

/*  //EVS no longer needed with TransBuff.mdb
UINT ProcessWriteFailures( LPVOID pParam )
{
	CProcessDumpInfo *DumpInfo;
	DumpInfo = (CProcessDumpInfo *) pParam;
	int iDebugLevel = DumpInfo->GetDebugLevel();
	CString sBuffer = DumpInfo->GetConnectString();
	ProcessDumpRecs ( iDebugLevel, sBuffer );
	return 0;
}
*/
UINT ProcessDBTransactions( LPVOID pParam )
{
	CProcessDumpInfo *TranstoDB;
	TranstoDB = (CProcessDumpInfo *) pParam;
	int iDebugLevel = TranstoDB->GetDebugLevel();
	CString sBuffer = TranstoDB->GetConnectString();
	ProcessTransaction ( iDebugLevel, sBuffer );
	return 0;
}


bool bOpenDBDao(CDaoDatabase *dbTransBuf,CString strMdb,bool bExclusive,bool bReadonly)
{
	int intDBConnectionTries = 0;
	bool bDBConnected = false;

	do
	{
		try
		{
			dbTransBuf->Open(strMdb,bExclusive,bReadonly,_T(""));
		}
		catch ( CDaoException* e )
		{
			CString strMsg;
			char szMessage[512];

			strMsg  = "bOpenDBDao open failed: " + e->m_pErrorInfo->m_strSource \
						+ "\t" + e->m_pErrorInfo->m_strDescription;
			sprintf(szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
			ErrMsg(strMsg,4);
			intDBConnectionTries++;  //errMsg EVS
			e->Delete();
			return(bDBConnected);
		}

		if (dbTransBuf->IsOpen())
		{
			bDBConnected = true;
		}
		else if (intDBConnectionTries < 5)
		{
			Sleep(5000);
		}
	}
	while ((! bDBConnected) && intDBConnectionTries < 5);
	return(bDBConnected);
}


void PostDBTrans(CString strTrans)
{
	CString strMdb,strMsg;
	CRegistry *Reg;
	long lRetVal;
	HKEY hBaseKey;
	char *szKeyName = "BufferLoc";
	char szMessage[512];

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
//		sprintf(szMessage,"Successfully retrieved the MDB Path: %s",strMdb);
//		strMsg = szMessage;
//		ErrMsg(strMsg,4);
	}

	Reg->CloseRegKey(hBaseKey);
	delete Reg;

	CDaoDatabase *dbTransBuf = new CDaoDatabase;
	if(bOpenDBDao(dbTransBuf,strMdb,false,false))
	{
		UpdateTransStatus(dbTransBuf,strTrans);
		if(dbTransBuf->IsOpen())
			dbTransBuf->Close();
	}
	delete dbTransBuf;
	AfxDaoTerm();

}


void UpdateTransStatus(CDaoDatabase *dbTransBuf,CString strNewRec)
{

	CString strSQL;

	CTime tCurDate = CTime::GetCurrentTime();
	CString strTime = tCurDate.Format("%m/%d/%y %H:%M:%S %p");

	//update dateinserted query.
	// Update the Transactions buffer status of the record where
	// the transaction equals strNewRec and the DateInserted is NULL
	strSQL = "UPDATE Transactions SET DateInserted = ";
	strSQL += "\"" + strTime + "\"";
	strSQL += " WHERE ((Trans = ";
	strSQL += "\"" + strNewRec + "\") AND (DateInserted = NULL));";

	try
	{
		dbTransBuf->Execute(strSQL,dbSeeChanges);
	}
	catch(CDaoException *e)
	{
		CString strMsg;

		strMsg  = "Database Connection Failed: " + e->m_pErrorInfo->m_strSource \
					+ "\t" + e->m_pErrorInfo->m_strDescription;
		ErrMsg(strMsg,e->m_pErrorInfo->m_lErrorCode);
		e->Delete();
		return;
	}
}


void PurgeOldXacts(void)
{
	CString strSQL,strMdb,strMsg;
	char szMessage[512];
	long lRetVal;
	HKEY hBaseKey;
	char *szKeyName = "BufferLoc";
	CDaoFieldInfo RetTable;
	CRegistry *Reg;
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
	{   // Only for Debugging
//		sprintf(szMessage,"Successfully retrieved the MDB Path: %s",strMdb);
//		strMsg = szMessage;
//		ErrMsg(strMsg,4);
	}

	Reg->CloseRegKey(hBaseKey);
	delete Reg;

	CDaoDatabase *dbDelBuf = new CDaoDatabase;

	if(bOpenDBDao(dbDelBuf,strMdb,false,false))
	{
		CDaoTableDef dbTransTable(dbDelBuf);
		CDaoFieldInfo dbRetFields;

		try
		{
			dbTransTable.Open("Retention");
		}
		catch(CDaoException *e)
		{
			CString strMsg;
			char szMessage[512];

			strMsg  = "Database Connection Failed: " + e->m_pErrorInfo->m_strSource \
						+ "\t" + e->m_pErrorInfo->m_strDescription;
			sprintf(szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
			e->Delete();
			dbDelBuf->Close();
			// Make an attempt to handle the exception error!
			delete dbDelBuf;
			AfxDaoTerm();
			return;
		}

		try
		{
			dbTransTable.GetFieldInfo("Increment",RetTable,AFX_DAO_ALL_INFO);
		}
		catch(CDaoException *e)
		{
			CString strMsg;
			char szMessage[512];

			strMsg  = "Database Connection Failed: " + e->m_pErrorInfo->m_strSource \
						+ "\t" + e->m_pErrorInfo->m_strDescription;
			sprintf(szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
			e->Delete();
			dbDelBuf->Close();
			AfxDaoTerm();
			return;

		}
		CString strInc = RetTable.m_strDefaultValue;

		try
		{
			dbTransTable.GetFieldInfo("Duration",RetTable,AFX_DAO_ALL_INFO);
		}
		catch(CDaoException *e)
		{
			CString strMsg;
			char szMessage[512];

			strMsg  = "Database Connection Failed: " + e->m_pErrorInfo->m_strSource \
						+ "\t" + e->m_pErrorInfo->m_strDescription;
			sprintf(szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
			e->Delete();
			dbDelBuf->Close();
			AfxDaoTerm();
			return;

		}
		CString strDur = RetTable.m_strDefaultValue;

		if(dbTransTable.IsOpen())
			dbTransTable.Close();

		CString strDelTime = CalcDeleteTime(strInc,strDur);

		strSQL = "DELETE * FROM Transactions WHERE ((DateInserted <> NULL) AND ";
		strSQL += "(DateInserted < #";
		strSQL += strDelTime + "#))";

		try
		{
			dbDelBuf->Execute(strSQL,0);
		}
		catch(CDaoException *e)
		{
			CString strMsg;
			char szMessage[512];

			strMsg  = "Database Connection Failed: " + e->m_pErrorInfo->m_strSource \
						+ "\t" + e->m_pErrorInfo->m_strDescription;
			sprintf(szMessage,"%s\t%d",strMsg,e->m_pErrorInfo->m_lErrorCode);
			e->Delete();
			dbDelBuf->Close();
			AfxDaoTerm();
			return;
		}
	}
	if(dbDelBuf->IsOpen())
		dbDelBuf->Close();
	delete dbDelBuf;
	AfxDaoTerm();
}


CString CalcDeleteTime(CString strIncrement,CString strDuration)
{
	CTime tOldest;
	CString strDelTime;

	// strIncrement comes from the table definition in the ""s"",""m"",etc...
	// The next seven line of code removes the extra two quotes from the string.
	int i = strIncrement.Find('"');
	strIncrement.SetAt(i,' ');
	i = strIncrement.Find('"');
	strIncrement.SetAt(i,' ');
	strIncrement.TrimLeft();
	strIncrement.TrimRight();
	strIncrement.MakeUpper();

	CTime tCurDate = CTime::GetCurrentTime();
	char *szT;
	switch(strtol(strIncrement,&szT,36))
	{
		case 28:
		{   // This case is for time retention in seconds!
			tOldest = tCurDate - CTimeSpan( 0, 0, 0, strtol(strDuration,&szT,10));
			break;
		}
		case 22:
		{   // This case is for time retention in minutes!
			tOldest = tCurDate - CTimeSpan( 0, 0, strtol(strDuration,&szT,10), 0);
			break;
		}
		case 17:
		{   // This case is for time retention in hours!
			tOldest = tCurDate - CTimeSpan( 0, strtol(strDuration,&szT,10), 0, 0);
			break;
		}
		case 13:
		{    // This case is for time retention in days!
			tOldest = tCurDate - CTimeSpan( strtol(strDuration,&szT,10), 0, 0, 0);
			break;
		}
		default:
		{
			tOldest = tCurDate - CTimeSpan( 1, 0, 0, 0);
		}
	}
	strDelTime = tOldest.Format("%m/%d/%y %H:%M:%S %p");
	return(strDelTime);
}



