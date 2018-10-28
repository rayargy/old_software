// DumpTrans.cpp
///////////////////////////////////////////////////////////////
// This function clears the transaction buffer and dumps the
// transactions to a text file. This event usually occurs when
// the Comm Server is shutdown.
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <fstream.h>

#include "Registry.h"
#include "EventLog.h"
#include "PARCSMSGS.h"

int		GetNewXact	( CString &, int );
void	ErrMsg		( CString &, long );

int DumpTrans ( )
{
	int iretVal = 0, iDebugLevel = 0;
	long lretVal = 0;
//	char *strPath = { "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\TransactionWriter" };
	char strPath[512] = { "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\TransactionWriter" };
	BOOL bPost = false;
	CString strDataFile, strDebugFile, strBuffer, strMsg;
	fstream fout;
	CEventLog CLog( "TransWriter" );
	CRegistry CReg;

	// Get the Data File Name, Debug File Name and Debug Level from the System Registry.

	HKEY hKey   = CReg.OpenRegKey   ( &lretVal, strPath, HKEY_LOCAL_MACHINE );
	if ( lretVal )
	{
		bPost = CLog.PostError ( MSG_TWRT_REG_ERR, NULL, 0 );
		iDebugLevel		= 0;
		strDataFile		= "C:\\TransWriter.dat";
		strDebugFile	= "C:\\TransWriterErr.txt";
	}
	else
	{
		iDebugLevel = (int)  CReg.FetchDWValue ( &lretVal, hKey, "DebugLevel" );
		if ( lretVal )
		{
			bPost = CLog.PostError ( MSG_TWRT_REG_NO_KEY, NULL, 0 );
			iDebugLevel = 0;
		}

		strDataFile = CReg.FetchStrValue ( &lretVal, hKey, "DataFile" );
		if ( lretVal )
		{
			bPost = CLog.PostError ( MSG_TWRT_REG_NO_KEY, NULL, 0 );
			strDataFile = "C:\\TransWriter.dat";
		}

		strDebugFile = CReg.FetchStrValue ( &lretVal, hKey, "DebugFile" );
		if ( lretVal )
		{
			bPost = CLog.PostError ( MSG_TWRT_REG_NO_KEY, NULL, 0 );
			strDebugFile = "C:\\TransWriterErr.txt";
		}
	}
	CReg.CloseRegKey(hKey);

	if ( iDebugLevel > 0 )
	{
		strMsg.Format ( "Entering DumpTrans Function, Debug Level = %d", iDebugLevel );
		ErrMsg ( strMsg, 0 );
	}

	// Open Output File.

	fout.open	( strDataFile, ios::app );

	if ( fout.fail ( ) )
	{
		strMsg = "Can NOT Open " + strDataFile + " File For Output";
		ErrMsg ( strMsg, 1 );
		return ( -1 );
	}
	else fout.setmode ( filebuf::text );

	// Main Loop to Retreive Transactions from the Buffer.

	while ( GetNewXact ( strBuffer, iDebugLevel ) == 0)
	{
		// Write Transaction To A Text File.

		if ( iDebugLevel > 1 )
		{
			strMsg = "Dumping trans: " + strBuffer;
			ErrMsg ( strMsg, 0 );
		}

		fout << strBuffer << '\n';

	} // End Loop to Dump Buffer.

	strBuffer.Empty ( );
	strBuffer.FreeExtra ( );
	strMsg.Empty ( );
	strMsg.FreeExtra ( );

	// Close Output File.

	fout.close ( );

	if ( iDebugLevel > 0 )
	{
		strMsg = "Leaving DumpTrans Function";
		ErrMsg ( strMsg, 0 );
	}
	return ( 1 );
}
