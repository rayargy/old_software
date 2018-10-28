// CTables.cpp
////////////////////////////////////////////////////////////////
// These class members functions defined the database tables.
////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CTables.h"

// Added for Limit Alarms
// April 1, 1998 - mlv
#include "..\Include\LimitDefs.h"
// End April 1, 1998 modification

CTables::CTables ( int iLevel ) { iDebugLevel = iLevel; }

void CTables::CommonFields ( CString &strFields )
{
//---------------------------------------------------------------------
// This function constructs the comma delimited string
// of fields common to all tables.
//---------------------------------------------------------------------

	strFields += "TranID,StationType,StationAddress,DateTime,Protocol";
}

void CTables::CreditCard ( CString &strFields )
{
//---------------------------------------------------------------------
// This function constructs a comma delimites string
// of fields for credit card data.
// Company ID not used for entrance_tran.
//---------------------------------------------------------------------

	/*  Temporarily commented out for ignoring 
		Credit card info

	strFields += ",CreditCardNumber";	// Card number.
	strFields += ",StartDate";		// Start date.
	strFields += ",ExpirationDate";// Expiration date.
	strFields += ",AuthorizationCode";		// Authorization code.
	strFields += ",TranNumber";		// Transaction number.
	strFields += ",ServiceCode";	// Service code, if VISA.
	strFields += ",RequestFlag";	// Request flag.
	strFields += ",ResultCode";	// Result and response code.
	strFields += ",ErrorFlag";		// Inhibit error flag.
	strFields += ",CreditError";	// Credit error
	strFields += ",Cashier";		// Cashier number.
	strFields += ",Shift";			// Shift number.
	*/

}

void CTables::Alarm ( CString &strFields )
{
//---------------------------------------------------------------------
// This function constructs a comma delimites string
// of fields the shift_tran table.
//---------------------------------------------------------------------

	strFields += ",AlarmID";
	switch ( iTransType )
	{
		case 1:							// Cold boot
			break;
		case 53:						// Coinsafe replaced
		case 54:						// Notesafe replaced
			// Added 08/17/98 to capture amount removed
			strFields += ",CParam,LParam,VParam,GrossAmount";
			break;
		case 2:							// Warm boot
			strFields += ",VParam";		// Number of warmboots.
			break;
		case 3:							// Alarm (Param1 contains type)
		case 13:						// Warning  (Param1 contains type)
			// Param1 to be stored in CParam
			strFields += ",RejectCodeID,CParam,AcceptStatus";
			break;
		case 12:						// Parameter file missing
		case 19:						// Program file missing
			strFields += ",AcceptStatus";
			break;
		case 22:						// Alarm Ticket
			strFields += ",RejectCodeID,AcceptStatus,AcceptTime";
			// AcceptTime to contain ticket time???
			break;
		case 21:						// Unsuccessful entry
		case 31:						// Unsuccessful exit
		case 41:						// Unsuccessful payment
		case 71:						// Unsuccessful passage
			strFields += ",CParam,LParam";	// Rejection code, Card Number
			strFields += ",GrossAmount,NetAmount,RejectCodeID";
			break;
		case 50:						// MPS Error
			// Alarm ID dependent upon if value in CParam, LParam, or VParam
			//		CParam != 0 -> Error of Credit Card Reader.
			//		LParam != 0 -> Error of Bank Note Acceptor.
			//		VParam != 0 -> Error of Coin Acceptor.
			strFields += ",RejectCodeID,CParam,LParam,AcceptStatus,VParam";
			break;
		case 67:						// Hold up
			// CParam -> Shift number.
			// LParam -> Cashier number.
			strFields += ",RejectCodeID,CParam,LParam,AcceptStatus";
			break;
		case 79:						// NOCOMM -> Station does not answer poll.
			// CParam -> Number of comm attempts
			strFields += ",RejectCodeID,CParam,AcceptStatus";
			break;
		case LIMIT_TRANS:
			// CParam -> CounterID
			// LParam -> LimitID
			// VParam -> Counter Value
			strFields += ",CParam,LParam,VParam,AcceptStatus";
	}
	// All alarm transactions receive the parameter list.
	strFields += ",ParamList";
}

void CTables::Entrance ( CString &strFields )
{
//---------------------------------------------------------------------
// This function constructs a comma delimited string
// of fields the entrance_tran table.
//
// Only transaction type 20 applies to entrance_tran.
//
// Validation index not used for entrance_tran.
//---------------------------------------------------------------------

	// Successful entry.
	strFields += ",TicketTypeID,RejectCodeID,TicketNumber,ParkHouse,PayStatus,TicketTrace";
	strFields += ",RegionCode,PrevStationType,PrevStationAddress,EntryDateTime";
	strFields += ",CardNumber,SubNumber,GrossAmount,NetAmount,ValueCardValue";
}

void CTables::Parker ( CString &strFields )
{
//---------------------------------------------------------------------
// This function constructs a comma delimites string
// of fields the parker_tran table.
//---------------------------------------------------------------------
//
// ticket_number = client number (int_nr).
// ticket_number = 1000 * client number (int_nr) + sub-number (sub_nr).
//---------------------------------------------------------------------

//This table is no longer "parker_tran" but "RevenueTrans".

	// These fields are common to all revenue transactions.
	strFields += ",TicketTypeID,RejectCodeID,TicketNumber,ValidationID,GrossAmount,NetAmount";
	strFields += ",EntryDateTime,EntryStationType,EntryStationAddress,RateTable,ParkHouse";
	switch ( iTransType )
	{
		case 30:						// Successful exit
		case 32:						// Exit w/swallow cmd & open barrier
		case 40:						// Successful payment
			strFields += ",PayMethodID,PayStatus,CardNumber,SubNumber,TicketTrace,ValueCardValue";	// KP status.
			if ( bCreditCard )
				CreditCard ( strFields );
			break;
		case 42:						// Damaged ticket
		case 70:						// Successful passage
			strFields += ",PayStatus";				// KP status.
			if (iTransType != 42)
				strFields += ",TicketTrace";
	}
}

void CTables::Status ( CString &strFields )
{
//---------------------------------------------------------------------
// This function constructs a comma delimited string
// of fields the status_tran table.
// Table: StatusTrans.
//---------------------------------------------------------------------

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
			// ParamInt2 -> Rejection code.
			// ParamInt1 -> Type of Alarm.
			// ParamInt3 -> Default value of zero.
			strFields += ",ParamInt2,ParamInt1,ParamInt3";
			break;
		case 10:						// RAM-parameters changed by service terminal
		case 11:						// RAM-parameters changed by MS
		case 14:						// Input made
		case 15:						// Local barrier control
		case 16:						// Remote barrier control
		case 17:						// Car passed open barrier
		case 18:						// ROM-parameters changed by MS
		case 23:						// Request pool-information
			// For 10 & 11 -> Number of parameter block.
			// For 14 -> Number of input.
			// For 15 & 16 -> Control of barrier.
			// For 17 -> Kind of barrier.
			// For 18 -> Number of parameter block.
			// For 23 -> Client number.
			strFields += ",ParamInt1";
			break;
		case 48:						// CCC period started
		case 49:						// CCC period ended
			// ParamInt1 -> Result code.
			// ParamLong1 -> Credit error.
			strFields += ",ParamInt1,ParamLong1";
			break;
		case 52:						// Door of cash station opened
		case 57:						// Actual statistics printed
		case 58:						// Midnight statistics printed
			strFields += ",ParamLong1";	// Protocol number for printing.
			break;
		case 55:						// Drum filled
		case 59:						// Overpaid
			// ParamLong1 -> Protocol number for printing.
			// For 55: ParamLong2 -> Total fill amount in units.
			// For 59: ParamLong2 -> Amount not returned.
			strFields += ",ParamLong1,ParamLong2";
			break;
		case 56:						// Drum emptied
		case 68:						// Buco 1 replaced
		case 69:						// Buco 2 replaced
			// ParamLong1 -> Protocol number for printing.
			// For 56: ParamLong2 -> Number of coins dispensed.
			// For 68 & 69: ParamLong2 -> Contents of buco.
			// For 56: ParamLong3 -> Coin value of drum.
			// For 68 & 69: ParamLong3 -> ID of buco.
			strFields += ",ParamLong1,ParamLong2,ParamLong3";
			break;
		case 65:						// One datablock read on DP-trace
		case 66:						// One datablock after writing DP-trace
			// ParamInt1 -> Type of DP.
			// ParamLong1 -> Client number.
			// ParamLong2 -> Client sub-number.
			strFields += ",ParamInt1,ParamLong1,ParamLong2";
	}
}

void CTables::Shift ( CString &strFields )
{
//---------------------------------------------------------------------
// This function constructs a comma delimited string
// of fields the ShiftTrans table.
//---------------------------------------------------------------------

	strFields += ",Shift,Cashier";
}

void CTables::CardSale ( CString &strFields )
{
//---------------------------------------------------------------------
// This function constructs a comma delimited string
// of fields the CardSaleTrans table.
//---------------------------------------------------------------------

	switch ( iTransType )
	{
		case 63:						// DP-card sold
		case 64:						// DP-card programmed
			strFields += ",CardNumber";
			if (iTransType == 63)
			{
				strFields += ",PaidAmount,PayMethodID";
				if ( bCreditCard )
					CreditCard ( strFields );
			}
			strFields += ",TicketTraceKP,TicketTraceDP,TicketTypeID,CardValue";
		
			if (iTransType == 64)
				strFields += ",TicketCount";

			break;
		case 75:						// HICO-card sold
		case 76:						// HICO-card programmed
			strFields += ",CardNumber,Cashier,Shift";
			if (iTransType == 75)
			{
				strFields += ",PaidAmount,PayMethodID";
				if ( bCreditCard )
					CreditCard ( strFields );
			}
	}
}

CString CTables::GetFields ( )
{
//---------------------------------------------------------------------
// This function returns a comma delimited list of fields
// enclosed in parentheses for use in the SQL script.
//---------------------------------------------------------------------

	CString strFields = " (";
	CommonFields ( strFields );
		
	switch ( table )
	{
		case alarm_tran:
			Alarm ( strFields );
			break;
		case entrance_tran:
			Entrance ( strFields );
			break;
		case parker_tran:
			Parker ( strFields );
			break;
		case status_tran:
			Status ( strFields );
			break;
		case shift_tran:
			Shift ( strFields );
			break;
		case cardSale_tran:
			CardSale ( strFields );
	}
	strFields += ") ";

	return strFields;
}

CString CTables::GetTable ( int TranType )
{
//---------------------------------------------------------------------
// This function returns the name of the table associated with
// the transaction type and sets the enum value for "table".
//
// Types 3, 5, 12, 13, 19, 22, 50, 67 and 79 are alarms (not included)
//              ^-- They're "included".  9.Oct.97 dar
//---------------------------------------------------------------------

	CString strTable;

	strTable.Empty ( );
	strTable.FreeExtra ( );

	iTransType = TranType;
	switch ( iTransType )
	{
		case 1:							// Cold boot
		case 2:							// Warm boot
		case 3:							// Alarm (Param1 contains type)
		case 12:						// Parameter file missing
		case 13:						// Warning  (Param1 contains type)
		case 19:						// Program file missing
		case 21:						// Unsuccessful entry
		case 22:						// Alarm Ticket
		case 31:						// Unsuccessful exit
		case 41:						// Unsuccessful payment
		case 50:						// MPS Error
		case 53:						// Coinsafe replaced
		case 54:						// Notesafe replaced
		case 67:						// Hold up
		case 71:						// Unsuccessful passage
		case 79:						// NOCOMM -> Station does not answer poll.
		case LIMIT_TRANS:				// Counter exceeded limit
			table = alarm_tran;
			strTable = "AlarmTrans";
			break;
		case 20:						// Successful entry
			table = entrance_tran;
			strTable = "EntryTrans";
			break;
		case 30:						// Successful exit
		case 32:						// Exit w/swallow cmd & open barrier
		case 40:						// Successful payment
		case 42:						// Damaged ticket
		case 70:						// Successful passage
			table = parker_tran;
			strTable = "RevenueTrans";
			break;
		case 4:							// Reset out of order
		case 5:							// SLG error
		case 6:							// Start of test mode
		case 7:							// End of test mode
		case 8:							// ROM-module copied to RAM
		case 9:							// RAM-module copied to ROM
		case 10:						// RAM-parameters changed by service terminal
		case 11:						// RAM-parameters changed by MS
		case 14:						// Input made
		case 15:						// Local barrier control
		case 16:						// Remote barrier control
		case 17:						// Car passed open barrier
		case 18:						// ROM-parameters changed by MS
		case 23:						// Request pool-information
		case 48:						// CCC period started
		case 49:						// CCC period ended
		case 51:						// No change
		case 52:						// Door of cash station opened
		case 55:						// Drum filled
		case 56:						// Drum emptied
		case 57:						// Actual statistics printed
		case 58:						// Midnight statistics printed
		case 59:						// Overpaid
		case 65:						// One datablock read on DP-trace
		case 66:						// One datablock after writing DP-trace
		case 68:						// Buco 1 replaced
		case 69:						// Buco 2 replaced
			table = status_tran;
			strTable = "StatusTrans";
			break;
		case 60:						// Shift started
		case 61:						// Shift paused
		case 62:						// Shift ended
			table = shift_tran;
			strTable = "ShiftTrans";
			break;
		case 63:						// DP-card sold
		case 64:						// DP-card programmed
		case 75:						// HICO-card sold
		case 76:						// HICO-card programmed
			table = cardSale_tran;
			strTable = "CardSaleTrans";
	}
	return ( strTable );
}

void CTables::SetCredit ( BOOL bCredit )
{
//---------------------------------------------------------------------
// This function sets the credit card flag.
//---------------------------------------------------------------------
	if ( bCredit )
		bCreditCard = true;
	else
		bCreditCard = false;
}

void CTables::AlarmFields(CString &strFields)
{
	strFields += "AlarmID, TranID, AlarmIndex, StationType, StationAddress, Protocol, DateTime, Cashier, AcceptStatus, AcceptTime, CParam";
}


/*  8.Jan.98 
	Added this function to extract only the fields needed
	to store in table CreditCardInfo.
	--Dar
*/
CString CTables::GetCreditCardFields()
{
	CString sFields;

	// These four columns map to sCommonValues in CBuffer.
	sFields = "(StationType, StationAddress, Protocol, DateTime";
	// !!!!  Need to get CreditCardCompany in here (and value in CBuffer)
	sFields += ",CreditCardNumber,StartDate,ExpirationDate,AuthorizationCode,TranNumber";		// Transaction number.
	// Service code, if VISA.
	sFields += ",ServiceCode,RequestFlag,ResultCode,ErrorFlag,CreditError,Cashier,Shift)";			// Shift number.

	return sFields;
}
