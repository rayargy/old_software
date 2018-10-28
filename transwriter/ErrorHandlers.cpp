// ErrorHandlers.cpp: 
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

// int DumpTrans ( ); // EVS no longer needed with TransBuff.mdb

//////////////////////////////////////////////////////////////////////
// This is an error handler stub.  It can be freely modified.
//////////////////////////////////////////////////////////////////////

#include "Registry.h"
#include "EventLog.h"
#include "PARCSMSGS.h"

void ErrorHandler ( char *s, DWORD err )
{
	CString sMessage;
	FILE *ErrorFile;
	long lretVal = 0;
//	char *strPath = { "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\TransactionWriter" };
	char strPath[512] = { "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\TransactionWriter" };
	BOOL bPost = false;
	CString strDebugFile;
//	int iretVal = DumpTrans ( ); //EVS no longer needed with TransBuff.mdb
	CEventLog CLog( "TransWriter" );
	CRegistry CReg;

	// Get the Debug File Name from the System Registry.

	HKEY hKey   = CReg.OpenRegKey   ( &lretVal, strPath, HKEY_LOCAL_MACHINE );
	if ( lretVal )
	{
		bPost = CLog.PostError ( MSG_TWRT_REG_ERR, NULL, 0 );
		strDebugFile	= "C:\\TransWriterErr.txt";
	}
	else
	{
		strDebugFile = CReg.FetchStrValue ( &lretVal, hKey, "DebugFile" );
		if ( lretVal )
		{
			bPost = CLog.PostError ( MSG_TWRT_REG_NO_KEY, NULL, 0 );
			strDebugFile = "C:\\TransWriterErr.txt";
		}
	}
	CReg.CloseRegKey(hKey);

	ErrorFile = fopen ( strDebugFile, "a" );
	sMessage.Format ( "%s\nError Number: %d\n", s, err );
	fprintf ( ErrorFile, "%s" , sMessage );
	fclose  ( ErrorFile );

	ExitProcess ( err );
}
