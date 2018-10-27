// CBuffer.cpp
////////////////////////////////////////////////////////////////
// These class members functions defined the transaction buffer.
////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CBuffer.h"
#include "PM820Time.h"

#include "math.h"

// Added for Limit Alarms
// April 1, 1998 - mlv
#include "..\Include\LimitDefs.h"
// End April 1, 1998 modification

void ErrMsg ( CString &, long );

CBuffer::CBuffer ( int iLevel ) { iDebugLevel = iLevel; }

void CBuffer::EmptyStr ( )
{
//----------------------------------------------------------------------
// This function clears the strParam and strCredit arrays
// and releases any unused memory.
//----------------------------------------------------------------------

	register int i = 0;

	for ( i = 0; i < 30; i++ )
	{
		strParam[i].Empty ( );
		strParam[i].FreeExtra ( );
		if ( i < 14 )
		{
			strCredit[i].Empty ( );
			strCredit[i].FreeExtra ( );
		}
	}
}

CString CBuffer::strDateTime ( CString longhex, BOOL exit_time )
{
//----------------------------------------------------------------------
// This function converts a longhex value representing
// the seconds expired from January 1, 1989.
//----------------------------------------------------------------------
/*
	int		hrs,	min,	sec;
	long	days,	tdif;

	sscanf ( longhex, "%x", &tdif );
	days  = tdif/86400;
	tdif -= 86400*days;
	hrs   = tdif/3600;
	tdif -= 3600*hrs;
	min   = tdif/60;
	tdif -= 60*min;
	sec   = tdif;

//	CTime		TBase	( 1989, 1, 1, 0, 0, 0, 0 );
//	Changed DST value from 0 to 1 on 4/17/1998 to fix problem - JLE

//	CTime		TBase	( 1989, 1, 1, 0, 0, 0, 1 );
	// Change to accomodate DST - 10/27/98 - mlv
	CTime		TBase	( 1989, 1, 1, 0, 0, 0, -1 );

	CTimeSpan	TSec	( days, hrs, min, sec );
	CTime		datetime = TBase + TSec;
*/

	CPM820Time	TimeConv;
	CTime		datetime = TimeConv.ConvertSecs(longhex);

	if ( exit_time )
		CTexit = datetime;
	else
		CTentry = datetime;

	return ( "'" + datetime.Format ( "%Y-%m-%d %H:%M:%S" ) + "'" );
}

CString CBuffer::strTicketType ( CString strType )
{
//------------------------------------------------------------------------------
// This function converts the DP Type to ticket type.
//------------------------------------------------------------------------------

	char dummy[2] = { "" };
	int idp = atoi ( strType );

	if ( idp < -10 && idp > -19 )
		idp += 30;
	else if ( idp < 0 && idp > -9 )
		idp += 28;
	else
		return ( strType );

	CString strdp = (char *) itoa ( idp, dummy, 10 );
	return ( strdp );
}

CString CBuffer::strTicketNo ( CString strClient, CString strSub )
{
//------------------------------------------------------------------------------
// This calculates the ticket number for
// debit card and congress card ticket types.
//------------------------------------------------------------------------------

	char dummy[1] = { 0 };
	int ic = atoi ( strClient );
	int is = atoi ( strSub );

	CString strtn = (char *) itoa ( 1000 * ic + is, dummy, 10 );
	return ( strtn );
}

void CBuffer::Station ( CString strStation, int itype, int iaddr )
{
	strParam[1] = "'" + strStation.Left(1) + "'";
	strParam[iaddr] = strStation.Mid ( 1 );
}

void CBuffer::SetCommData ( CParse &CommData )
{
//----------------------------------------------------------------------
// This function extracts parameters common to all transactions.
//----------------------------------------------------------------------
	CString strCode;
	CString strMsg;

	// Empty the strParam and strCredit arrays.

	EmptyStr ( );

	//----------------------------------------------------------------------
	// Common Parameters
	//----------------------------------------------------------------------

	// The station address appears first in the buffer.

	Station ( CommData.GetField ( 1 ), 1, 2 );
	if ( iDebugLevel > 2 )
	{
		strMsg = "Station = " + strParam[1] + strParam[2];
		ErrMsg ( strMsg, 0 );
	}

	// The Response ID appears next (not in database).

	// The Transaction Type appears next.
	strParam[0]	= CommData.GetField ( 3 );
	iTransType  = atoi ( strParam[0] );

	// The Transaction Protocol Number appears next.
	strParam[4]	= CommData.GetField ( 4 );
	if ( iDebugLevel > 2 )
	{
		strMsg = "Transaction Number = " + strParam[0];
		ErrMsg ( strMsg, 0 );
	}

	// Finally the Transaction Date and Time (longhex).
	strParam[3]	= strDateTime ( CommData.GetField ( 5 ), true );
	if (strParam[3].GetLength() == 2)
	{
		CTime ct = CTime::GetCurrentTime();
		strParam[3] = "'" + ct.Format("%Y-%m-%d %H:%M:%S") + "'";
	}

	if ( iDebugLevel > 2 )
	{
		strMsg = "Transaction Time = " + strParam[3];
		ErrMsg ( strMsg, 0 );
	}


	// 8.Jan.98 Added this to get common fields needed
	// for credit card info.
	//	--Dar
	sCommonValues = strParam[1] + ", "	//Station Type
		+ strParam[2] +	", "			//Station Address
		+ strParam[4] + ", "			//Protocol
		+ strParam[3];					//Date/Time
}

void CBuffer::Alarm ( CParse &CommData )
{
//----------------------------------------------------------------------
// This function extracts option parameters for transactions
// that will be inserted into the alarm_tran table.
//----------------------------------------------------------------------
	int index = 0;

	switch ( iTransType )
	{
		case 1:							// Cold boot
			strParam[5]	= "30";			// Maps to AlarmId
			break;
		case 2:							// Warm boot
			strParam[5]	= "31";			// Maps to AlarmId
			strParam[7]	= "'" + CommData.GetField ( 6 ) + "'";	// VParam
			break;
		case 3:							// Alarm (Param1 contains type)
		case 13:						// Warning  (Param1 contains type)
			strParam[5]	= CommData.GetField ( 6 );	// Maps to AlarmId
			if (iTransType == 13)
			{
				switch (atoi(strParam[5]))
				{
					case 0:
						strParam[5] = "10";
						break;
					case 1:
						strParam[5] = "11";
						break;
					case 2:
						strParam[5] = "12";
						break;
					case 3:
						strParam[5] = "13";
						break;
					case 4:
						strParam[5] = "14";
						break;
					case 5:
						strParam[5] = "15";
						break;
					case 6:
						strParam[5] = "16";
						break;
					case 7:
						strParam[5] = "17";
						break;
					case 8:
						strParam[5] = "18";
						break;
					case 9:
						strParam[5] = "19";
						break;
					case 51:
						strParam[5] = "41";
						break;
					case 52:
						strParam[5] = "42";
						break;
					case 53:
						strParam[5] = "43";
						break;
					case 54:
						strParam[5] = "44";
						break;
					case 55:
						strParam[5] = "45";
				}
			}
			strParam[6]	= "0";			// Maps to RejectCodeID
			strParam[7]	= "'" + strParam[5] + "'";	// Maps to CParam
			strParam[10]= "0";			// Maps to AcceptStatus
			break;
		case 12:						// Parameter file missing
		case 19:						// Program file missing
			if (iTransType == 12)
				strParam[5]	= "32";		// Maps to AlarmID
			else
				strParam[5] = "33";		// Maps to AlarmID
			strParam[10]= "0";			// Maps to AcceptStatus
			break;
		case 22:						// Alarm Ticket
			strParam[5]	= "29";			// Maps to AlarmID
			strParam[6]	= "0";			// Maps to RejectCodeID
			strParam[10]= "0";			// Maps to AcceptStatus
			// Maps to AcceptTime
			strParam[13]= strDateTime ( CommData.GetField ( 6 ), false );
			break;
		case 21:						// Unsuccessful entry
		case 31:						// Unsuccessful exit
		case 41:						// Unsuccessful payment
		case 71:						// Unsuccessful passage
			// AlarmID
			switch (iTransType)
			{
				case 21:
					strParam[5] = "24";
					break;
				case 31:
					strParam[5] = "25";
					break;
				case 41:
					strParam[5] = "26";
					break;
				case 71:
					strParam[5] = "27";
			}
							//RejectCodeID
			strParam[6]	= "'" + CommData.GetField ( 6 ) + "'";	// Maps to CParam
							//CardNumber,TicketNumber
			strParam[7] = "'" + CommData.GetField(12) + "'";	// Maps to LParam

			strParam[8] = CommData.GetField(8); // Maps to GrossAmount
			strParam[8] = ConvertToDollars(strParam[8]);	// Comes in as integer...
			strParam[9] = CommData.GetField(9); // Maps to NetAmount
			strParam[9] = ConvertToDollars(strParam[9]);	// Comes in as integer...
			strParam[10] = CommData.GetField ( 6 );	// Maps to RejectCodeID

			break;
		case 50:						// MPS Error
			// Maps to AlarmID
			if (atoi(CommData.GetField ( 6 )))
				strParam[5] = "34";		// Coin-Acceptor
			else if (atoi(CommData.GetField ( 7 )))
				strParam[5] = "35";		// Banknote-Acceptor
			else if (atoi(CommData.GetField ( 8 )))
				strParam[5] = "36";		// Creditcard-Reader
			else
				strParam[5]	= "99";
			strParam[6]	= "0";			// Maps to RejectCodeID
			strParam[7] = "'" + CommData.GetField ( 8 ) + "'";	// Maps to CParam
			strParam[8] = "'" + CommData.GetField ( 7 ) + "'";	// Maps to LParam
			strParam[10]= "0";			// Maps to AcceptStatus
			strParam[12]= "'" + CommData.GetField ( 6 ) + "'";	// Maps to VParam
			break;
		case 53:						// Coinsafe replaced
			// CParam -> Protocol Number for Printing
			// LParam -> Identification Number for safe
			// VParam -> Number of coins in safe
			// GrossAmount -> Amount of money in safe
			strParam[5]	= "6";			// Maps to AlarmId
			strParam[6] = "'" + CommData.GetField(6) + "'";	// Maps to CParam
			strParam[7] = "'" + CommData.GetField(7) + "'";	// Maps to LParam
			strParam[8] = "'" + CommData.GetField(9) + "'";	// Maps to VParam
			strParam[9] = ConvertToDollars(CommData.GetField(8)); // Maps to GrossAmount
			break;
		case 54:						// Notesafe replaced
			// CParam -> Protocol Number for Printing
			// LParam -> Identification Number for safe
			// VParam -> Number of notes in safe
			// GrossAmount -> Amount of money in safe
			strParam[5]	= "7";			// Maps to AlarmId
			strParam[6] = "'" + CommData.GetField(6) + "'";	// Maps to CParam
			strParam[7] = "'" + CommData.GetField(7) + "'";	// Maps to LParam
			strParam[8] = "'" + CommData.GetField(9) + "'";	// Maps to VParam
			strParam[9] = ConvertToDollars(CommData.GetField(8)); // Maps to GrossAmount
			break;
		case 67:						// Hold up
			strParam[5]	= "37";			// Maps to AlarmID
			strParam[6]	= "0";			// Maps to RejectCodeID
			strParam[7]	= "'" + CommData.GetField ( 6 ) + "'";	// Maps to CParam
			strParam[8] = "'" + CommData.GetField ( 7 ) + "'";	// Maps to LParam
			strParam[10]= "0";			// Maps to AcceptStatus
			break;
		case 79:						// NOCOMM -> Station does not answer poll.
			strParam[5]	= "20";			// Maps to AlarmID
			strParam[6]	= "0";			// Maps to RejectCodeID
			strParam[7]	= "'" + CommData.GetField ( 6 ) + "'";	// Maps to CParam
			strParam[10]= "0";			// Maps to AcceptStatus
			break;
		case LIMIT_TRANS:				// Counter exceeded limit
			strParam[5] = "22";			// Maps to AlarmID
			strParam[7]	= "'" + CommData.GetField ( 6 ) + "'";	// Maps to CParam
			strParam[8]	= "'" + CommData.GetField ( 7 ) + "'";	// Maps to LParam
			strParam[9]	= "'" + CommData.GetField ( 8 ) + "'";	// Maps to VParam
			strParam[10]= "0";			// Maps to AcceptStatus
			break;
	}
}

void CBuffer::Entrance ( CParse &CommData )
{
//	strParam[5]	= strTicketType ( CommData.GetField ( 11 ) );	// Maps to TicketType
	strParam[5]	= CommData.GetField ( 11 );	// Maps to TicketType
	strParam[6]	= CommData.GetField ( 6 );	// Maps to RejectCodeID

	if ((strParam[5] == "0")) // KP Ticket
	{
		strParam[12] = CommData.GetField (12); // TicketNumber
		strParam[25] = "NULL";  // Maps to CardNumber
		strParam[28] = "NULL";	// Maps to NetAmount
	}
	else	// DP Ticket
	{
		strParam[12] = "NULL";   // TicketNumber
		strParam[25] = CommData.GetField (12); // Maps to CardNumber
		strParam[28] = CommData.GetField (9);	// Maps to NetAmount
		strParam[28] = ConvertToDollars(strParam[28]);	// Comes in as integer...
	}

	strParam[17]= CommData.GetField ( 16 );	// Maps to ParkHouse
	strParam[19]= CommData.GetField ( 17 );	// Maps to TranStatus
	strParam[20]= "'" + CommData.GetField ( 18 ) + "'";	// TicketTrace
	strParam[21]= CommData.GetField (15); // Maps to RegionCode

	CString sTemp = CommData.GetField(14);
	strParam[22]= sTemp.Left(1);			// Maps to PrevStationType
	strParam[23]= sTemp.Mid(1);				// Maps to PrevStationAddress

	strParam[24] = strDateTime(CommData.GetField (7), false);	// Maps to EntryDateTime
	strParam[26] = CommData.GetField (13);  // Maps to SubNumber
	strParam[27] = CommData.GetField (8);  // Maps to GrossAmount
	strParam[27] = ConvertToDollars(strParam[27]);	// Comes in as integer...
	// If the ticket type is valuecard extract
	// ValueCardValue from KP Trace

	// 24.August.1998 -- Changed the following strParam[]
	// index from 28 to 29 because entry transactions were
	// not getting inserted as more columns than values were
	// in the SQL statement, and it appeared that NetAmount
	// was getting overwritten.
	if (strParam[5] == "1")
		strParam[29] = GetCardValueFromTrace(CommData.GetField(18));
	else
		strParam[29] = "NULL";
}

void CBuffer::Parker ( CParse &CommData )
{
//----------------------------------------------------------------------
// This function extracts option parameters for transactions
// that will be inserted into the parker_tran table.
//----------------------------------------------------------------------

//The table used is no longer "parker_tran" but "RevenueTrans"

	int ipay = 0;
	CString sErr;

	strParam[5]	= CommData.GetField(11);	// Maps to TicketType in the parker_tran table.
	strParam[6]	= CommData.GetField(6);					// Maps to RejectCodeID
	// Maps to TicketNumber
	if ((strParam[5] == "0"))
	{
		strParam[9]	= CommData.GetField (12);
		strParam[20] = "NULL";
		strParam[21] = "NULL";
	}
	else
	{
		strParam[9] = "NULL";
		strParam[20] = CommData.GetField (12);
		strParam[21] = CommData.GetField (13);
	}
	strParam[10] = CommData.GetField (10);		// Maps to ValidationID in the parker_tran table.
	strParam[11] = CommData.GetField (8);		// Maps to GrossAmt in the parker_tran table.
	strParam[11] = ConvertToDollars(strParam[11]);		// Comes in as integer...
	strParam[12] = CommData.GetField (9);		// Maps to NetAmt
	strParam[12] = ConvertToDollars(strParam[12]);		// Comes in as integer...
	strParam[13] = strDateTime(CommData.GetField (7), false);	// Maps to EntryDateTime
	CString sTemp = CommData.GetField(14);
	strParam[14]= sTemp.Left(1);			// Maps to EntryStationType
	strParam[15]= sTemp.Mid(1);				// Maps to EntryStationAddress
	strParam[16]= CommData.GetField(15);	// Maps to RateTable
	strParam[17]= CommData.GetField(16);	// Maps to ParkHouse

	switch ( iTransType )
	{
		case 30:						// Successful exit
		case 32:						// Exit w/swallow cmd & open barrier
		case 40:						// Successful payment
			strParam[18]= CommData.GetField(18);	// Maps to PayMethodID
			strParam[19]= CommData.GetField(17);	// Maps to PayStatus
			ipay = atoi(strParam[18]);
			strParam[22]= "'" + CommData.GetField(CommData.Count()) + "'";	// TicketTrace

			// If the ticket type is valuecard extract
			// ValueCardValue from KP Trace
			if (strParam[5] == "1")
				strParam[23] = GetCardValueFromTrace(CommData.GetField(CommData.Count()));
			else
				strParam[23] = "NULL";

			break;
		case 42:						// Damaged ticket
		case 70:						// Successful passage
			strParam[19]= CommData.GetField (17);	// Maps to PayStatus
			strParam[20] = "";			// Clear out 20 & 21 because...
			strParam[21] = "";			// We aren't sure about info
			if (iTransType == 70)
				strParam[22]= "'" + CommData.GetField(CommData.Count()) + "'";	// TicketTrace
	}
}

void CBuffer::Status ( CParse &CommData )
{
//----------------------------------------------------------------------
// This function extracts option parameters for transactions
// that will be inserted into the status_tran table.
//----------------------------------------------------------------------
	int index = 0, imsb = 0, ilsb = 0;
	long longhex = 0;
	char strdummy[50];

	switch ( iTransType )
	{
		case 4:							// Reset out of order
		case 6:							// Start of test mode
		case 7:							// End of test mode
		case 8:							// ROM-module copied to RAM
		case 9:							// RAM-module copied to ROM
		case 51:						// No change
			// None of the above have any parameters...
			break;
		case 5:							// SLG error
			strParam[6]	= "0";			// ParamInt2
			sscanf ( CommData.GetField ( 6 ), "%x", &longhex );
			strParam[7]= ltoa ( longhex, strdummy, 10 );	// ParamInt1
			strParam[10]= "0";			// ParamInt3
			break;
		case 10:						// RAM-parameters changed by service terminal
		case 11:						// RAM-parameters changed by MS
		case 14:						// Input made
		case 15:						// Local barrier control
		case 16:						// Remote barrier control
		case 17:						// Car passed open barrier
		case 18:						// ROM-parameters changed by MS
		case 23:						// Request pool-information
		case 57:						// Actual statistics printed
		case 58:						// Midnight statistics printed
			// For 10,11,14,15,16,17,18,23 -> Maps to ParamInt1
			// For 57, 58 -> Maps to ParamLong1
			strParam[7]	= CommData.GetField ( 6 );
			break;
		case 48:						// CCC period started
		case 49:						// CCC period ended
			strParam[7]	= CommData.GetField ( 6 );	// Maps to ParamInt1
			strCredit[10]=CommData.GetField ( 7 );	// Maps to ParamLong1
			break;
		case 52:						// Door of cash station opened
			strParam[12]= CommData.GetField ( 6 );	// Maps to ParamLong1
			break;
		case 55:						// Drum filled
		case 56:						// Drum emptied
		case 59:						// Overpaid
			strParam[7]	= CommData.GetField ( 6 );	// Maps to ParamLong1
			strParam[12]= CommData.GetField ( 7 );	// Maps to ParamLong2
			if ( iTransType == 56 )
				strParam[8]= CommData.GetField ( 8 );	// Maps to ParamLong3
			break;
		case 65:						// One datablock read on DP-trace
		case 66:						// One datablock after writing DP-trace
			strParam[6]	= CommData.GetField ( 6 );	// Maps to ParamInt1
			strParam[7]	= CommData.GetField ( 7 );	// Maps to ParamLong1
			strParam[8]= CommData.GetField ( 8 );	// Maps to ParamLong2
			break;
		case 68:						// Buco 1 replaced
		case 69:						// Buco 2 replaced
			strParam[7]	= CommData.GetField ( 6 );	// Maps to ParamLong1
			sscanf ( CommData.GetField ( 7 ), "%x", &longhex );
			strParam[13]= ltoa ( longhex, strdummy, 10 );	// Maps to ParamLong3
			strParam[12]= CommData.GetField ( 8 );	// Maps to ParamLong2
	}
}

void CBuffer::Shift ( CParse &CommData )
{
	strParam[6]	= CommData.GetField ( 6 );	// Maps to Shift
	strParam[7] = CommData.GetField ( 7 );	// Maps to Cashier
}
	
void CBuffer::CardSale ( CParse &CommData )
{
//----------------------------------------------------------------------
// This function extracts option parameters for transactions
// that will be inserted into the cardSale_tran table.
//----------------------------------------------------------------------
//

	int ipay = 0;
	long longhex = 0;
	char strdummy[50];
	CString sErr;

	switch ( iTransType )
	{
		case 63:					// DP-card sold
		case 64:					// DP-card programmed
			strParam[7]	= CommData.GetField ( 6 );	// Maps to CardNumber
			if (iTransType == 63)
			{
				strParam[8]= CommData.GetField ( 7 );		// Maps to PaidAmt
				strParam[8] = ConvertToDollars( strParam[8] );	// Comes in as integer...
				strParam[9] = CommData.GetField ( 10 );		// Maps to PayMethodID
				ipay = atoi ( strParam[18] );
				if ( ipay == 1 || ipay > 9 )
				{
					CreditCard ( CommData, 9 );
					bCreditCard = true;
					sErr = "Creditcard info found...";
					ErrMsg (sErr, 0 );
				}
			}
			strParam[10] = "'" + CommData.GetField(8) + "'";	// Maps to TicketTraceKP
			strParam[11] = "'" + CommData.GetField(9) + "'";	// Maps to TicketTraceDP

			// Extract TicketTypeID from DP Trace
			strParam[12] = GetTicketTypeIDFromTrace(CommData.GetField(9));	// Maps to TicketTypeID
			// Extract CardValue from KP Trace
			strParam[13] = GetCardValueFromTrace(CommData.GetField(8));		// Maps to CardValue

			if (iTransType == 64)
				strParam[14] = CommData.GetField(7); // Maps to TicketCount

			break;
		case 75:					// HICO-card sold
		case 76:					// HICO-card programmed
			sscanf ( CommData.GetField ( 8 ), "%x", &longhex );
			strParam[7]= ltoa ( longhex, strdummy, 10 );	// Maps to CardNumber
			if (iTransType == 75)
			{
				strParam[8] = CommData.GetField ( 10 );	// Maps to PaidAmt
				strParam[9] = CommData.GetField ( 9 );	// Maps to PayMethodID
				ipay = atoi ( strParam[9] );
				if ( ipay == 1 || ipay > 9 )
				{
					CreditCard ( CommData, 9 );
					bCreditCard = true;
					sErr = "Creditcard info found...";
					ErrMsg ( sErr, 0 );
				}
			}
	}
}

void CBuffer::CreditCard ( CParse &CommData, int index )
{
//----------------------------------------------------------------------
// This function sets the credit card data.
// Note: the first character, which should be an '!', is ignored.
//----------------------------------------------------------------------
	int i = 0;
	CString sErr;

	sErr = "**********Creditcard info:";
	ErrMsg ( sErr, 0 );
	strCredit[0]= CommData.GetField ( index++ );
	strCredit[0].Empty ( ); // Not used in data dictionary for parker_tran.
	
	for ( i = 1; i < 13; i++ )
		strCredit[i]= CommData.GetField ( index++ );
	
	// Eliminate the space of 'K' at the end.
	strCredit[3] = strCredit[3].Mid (0, strCredit[3].GetLength() - 1);
	strCredit[4] = strCredit[4].Mid (0, strCredit[4].GetLength() - 1);
	
	// Change the date format from YYMMDD to YY-MM-DD.
	
	strCredit[2]= strCredit[2].Left (2) + '-' + strCredit[2].Mid (2, 2) + '-' + strCredit[2].Right (2);
	strCredit[3]= strCredit[3].Left (2) + '-' + strCredit[3].Mid (2, 2) + '-' + strCredit[3].Right (2);

	// Add quotes for the first 9 items which were identified as strings.
	for ( i = 0; i < 9; i++ )
		strCredit[i]= "'" + strCredit[i] + "'";

	for (i = 0; i < 14; i++)
	{
		sErr.Format("%d: %s", i, strCredit[i]);
		ErrMsg ( sErr, 0 );
	}
	sErr = "***********END creditcard info*********\n";
	ErrMsg(sErr, 0);
}

void CBuffer::SetData ( CParse &CommData )
{
//----------------------------------------------------------------------
// This function is the main function for setting values
// for optional transaction parameters.
//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	// Optional Parameters
	//----------------------------------------------------------------------
	//
	// Types 3, 5, 12, 13, 19, 22, 50, 67 and 79 are alarms (not included)
	//
	// Types 1, 4, 6, 7, 8, 9, 51, 66 have no optional parameters.
	//
	//----------------------------------------------------------------------

	bCreditCard = false;

	switch ( table )
	{
		case alarm_tran:
			Alarm ( CommData );
			break;
		case entrance_tran:
			Entrance ( CommData );
			break;
		case parker_tran:
			Parker ( CommData );
			break;
		case status_tran:
			Status ( CommData );
			break;
		case shift_tran:
			Shift ( CommData );
			break;
		case cardSale_tran:
			CardSale ( CommData );
	}
}

void CBuffer::SetTable ( CString &strTable )
{
//----------------------------------------------------------------------
// This function sets the enum table value.
//----------------------------------------------------------------------
	if ( ! strTable.Compare ( "AlarmTrans"    ) ) table = alarm_tran;
	if ( ! strTable.Compare ( "EntryTrans"	  ) ) table = entrance_tran;
	if ( ! strTable.Compare ( "RevenueTrans"  ) ) table = parker_tran;
	if ( ! strTable.Compare ( "StatusTrans"   ) ) table = status_tran;
	if ( ! strTable.Compare ( "ShiftTrans"	  ) ) table = shift_tran;
	if ( ! strTable.Compare ( "CardSaleTrans" ) ) table = cardSale_tran;
}

CString CBuffer::GetValues ( )
{
//----------------------------------------------------------------------
// This function returns the values as a comma delimited
// string enclosed in parentheses for the SQL script.
//----------------------------------------------------------------------
	int i = 0;

	CString strValues = " (" + strParam[i];

	for (i = 1; i < 30; i++)
		if ( !strParam[i].IsEmpty ( ) )
			strValues += "," + strParam[i];

	/*  Temporarily commented out for ignoring 
		Credit card info

	for (i = 1; i < 13; i++)
		if ( !strCredit[i].IsEmpty ( ) )
			strValues += "," + strCredit[i];
	*/

	strValues += ")";
	return strValues;
}

BOOL CBuffer::GetCredit ( )
{
//----------------------------------------------------------------------
// This function returns a value indicating credit card
// information is available for the transaction.
//----------------------------------------------------------------------
	return ( bCreditCard );
}

/* 8.Jan.98
	Added function for extracting separate credit card-only
	information for saving to separate table CreditCardInfo.
	--Dar
*/
CString CBuffer::GetCreditCardValues()
{
	CString strValues;
	int i;

	strValues = "(" + sCommonValues;

	for (i = 1; i < 13; i++)
		if ( !strCredit[i].IsEmpty ( ) )
			strValues += "," + strCredit[i];

	strValues += ")";
	return strValues;
}

CString CBuffer::ConvertToDollars(CString strValue)
{
	CString strTemp0 = strValue;
	CString strTemp1 = "";

	if ( strValue.GetLength() < 3 )
		strTemp0 = "000" + strValue;

	strTemp1 = strTemp0.Left( strTemp0.GetLength() - 2 );
	strTemp1 += ".";
	strTemp1 += strTemp0.Right(2);
	return strTemp1;
}

CString CBuffer::GetTicketTypeIDFromTrace(CString traceString)
{
	CString strMsg, strFields, formatString;
	unsigned long lValue = 0L;

	if ( iDebugLevel > 2 )
	{
		strMsg = "traceString = " + traceString;
		ErrMsg ( strMsg, 0 );
	}

	strFields = traceString.Mid(10,8);

	if ( iDebugLevel > 2 )
	{
		strMsg = "strFields = " + strFields;
		ErrMsg ( strMsg, 0 );
	}

	sscanf ( strFields, "%x", &lValue );

	if ( iDebugLevel > 2 )
	{
		strMsg.Format("lValue = %lu",lValue);
		ErrMsg ( strMsg, 0 );
	}
	formatString.Format("%lu",((lValue & 0x0001F000L)>>12)); // Ticket Type ID

	return formatString;
}


CString CBuffer::GetCardValueFromTrace(CString traceString)
{
	char *szStopString;
	CString sCardValue = "0x" + traceString.Mid(14, 7);
	long lTempCardValue = strtol(sCardValue, &szStopString, 16);
	sCardValue.Format("%d", lTempCardValue);
	sCardValue = ConvertToDollars(sCardValue);
	return sCardValue;
}

CString CBuffer::GetCommonValues()
{

	return sCommonValues;
}
