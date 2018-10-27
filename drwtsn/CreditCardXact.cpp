// CreditCardXact.cpp: implementation of the CCreditCardXact class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <afxdb.h>
#include "CParse.h"	// Added by ClassView
#include "EventLog.h"
#include "PARCSMSGS.h"
#include "TransWriter.h"
#include "CreditCardXact.h"
#include "PM820Time.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCreditCardXact::CCreditCardXact()
{

}

CCreditCardXact::~CCreditCardXact()
{

}

// Returns an SQL Insert string appropriate to the particular
// transaction that was passed.  Currently only two such
// transactions are handled:
//
//	81 - Request Credit Card Inserted
//  82 - Virtual Ticket for Credipark
//
CString CCreditCardXact::GetSaveXactString(CParse *oParse)
{
	CString strSQL;

	m_intTransType = atoi(oParse->GetField(3));

	// m_lpParse will reference all the fields from the 
	// original transaction string that was parsed.
	m_lpParse = oParse;


	switch (m_intTransType)
	{
	case 81:
		strSQL = Process81();
		break;

	case 82:
		strSQL = Process82();
		break;

	default:
		strSQL = "";
	}

	return strSQL;
}

// Assembles an SQL Insert string for an 81 - Request Credit
// Card Inserted transaction.
CString CCreditCardXact::Process81()
{
	CString strSQL;
	CString strTable;
	CString strFields;
	CString strValues;
	CString strTemp;
	int intIndex;
	
	strTable = "CreditCardRequestTrans";

	GetCommonFieldsAndValues(strFields, strValues);
	// These remaining fields must be verified for the order 
	// they are received--This order is based purely on 
	// documentation P900 v.1.3 of 13.Feb.98.

	// Ignore the above comment--The documentation does not
	// list the parameters in the right order.

	//RFCNumber (6)
	strFields += ", RFCNumber";
	strValues += ", " + m_lpParse->GetField(7);

	// If the RFCNumber == 1, then another integer
	// value gets passed in the next field.  However,
	// it is something we want to ignore, so we must
	// check for it and adjust our index from here on out.
	/*
	if ((atoi(m_lpParse->GetField(6))) == 1)
		intIndex = 8;
	else
		intIndex = 7;
	*/

	intIndex = 8;

	// "(#/#)" indicates index when RFCNumber != 1 and
	// index when RFCNumber == 1 respectively.

	//CommandNumber (7/8)
	strTemp = m_lpParse->GetField(intIndex++);
	if (strTemp.GetLength() == 0)
		strTemp = "0";

	strFields += ", CommandNumber";
	strValues += ", " + strTemp;

	//CreditCardNumber (8/9)
	// This is not really literally the credit card 
	// number, but all info from Track 2 of the card.
	strFields += ", CreditCardNumber";
	strValues += ", '" + m_lpParse->GetField(intIndex++) + "'";

	//Amount (9/10)
	strFields += ", Amount";
	strValues += ", " + ConvertToMoney(m_lpParse->GetField(intIndex++));

	//ClearingMethod (10/11)
	strFields += ", ClearingMethod";
	strValues += ", '" + m_lpParse->GetField(intIndex++) + "'";

	//AuthorizationCode (11/12)
	strFields += ", AuthorizationCode";
	strValues += ", '" + m_lpParse->GetField(intIndex++) + "'";

	//CompanyID (12/13)
	strFields += ", CompanyID";
	strValues += ", '" + m_lpParse->GetField(intIndex++) + "'";

	strSQL = "Insert into " + strTable + " (" + strFields + ") Values (" +
		strValues + ")";
	return strSQL;

}

// Assembles an SQL Insert string for an 82 - Virtual 
// Ticket for Credipark transaction.
CString CCreditCardXact::Process82()
{
	CString strSQL;
	CString strTable;
	CString strFields;
	CString strValues;
	CString sTemp;

	int intIndex;

	strTable = "VTicketTrans";

	GetCommonFieldsAndValues(strFields, strValues);
	
	//RFCNumber (6)
	strFields += ", RFCNumber";
	strValues += ", " + m_lpParse->GetField(6);

	// If the RFCNumber == 1, then another integer
	// value gets passed in the next field.  However,
	// it is something we want to ignore, so we must
	// check for it and adjust our index from here on out.
	/*
	if ((atoi(m_lpParse->GetField(6))) == 1)
		intIndex = 8;
	else
		intIndex = 7;
	*/

	intIndex = 8;

	//CommandNumber (7 or 8)

	sTemp = m_lpParse->GetField(intIndex++);

	if (sTemp.GetLength() == 0)
		sTemp = "0";

	strFields += ", CommandNumber";
	strValues += ", " + sTemp;

	//CreditCardNumber (8/9)  
	// This is not really literally the credit card 
	// number, but all info from Track 2 of the card.
	strFields += ", CreditCardNumber";
	strValues += ", '" + m_lpParse->GetField(intIndex++) + "'";

	//Attribute (9/10)
	sTemp = m_lpParse->GetField(intIndex++);	// Added 10/7/98 to prevent db write errors
	if ( sTemp.GetLength() == 0 )
		sTemp = "NULL";

	strFields += ", Attribute";
//	strValues += ", " + m_lpParse->GetField(intIndex++);
	strValues += ", " + sTemp;

	//KP-Trace (10/11)
	strFields += ", KPTrace";
	strValues += ", '" + m_lpParse->GetField(intIndex++) + "'";

	//DP-Trace (11/12)
	strFields += ", DPTrace";
	strValues += ", '" + m_lpParse->GetField(intIndex++) + "'";

	strSQL = "Insert into " + strTable + " (" + strFields + 
		") Values (" + strValues + ")";
	return strSQL;
}

CString CCreditCardXact::ConvertDateTime(CString longhex)
{
/*nt		hrs,	min,	sec;
	long	days,	tdif;

	sscanf ( longhex, "%x", &tdif );
	days  = tdif/86400;
	tdif -= 86400*days;
	hrs   = tdif/3600;
	tdif -= 3600*hrs;
	min   = tdif/60;
	tdif -= 60*min;
	sec   = tdif;

//	CTime		TBase	( 1989, 1, 1, 0, 0, 0, 1 );
	// Change to accomodate DST - 10/27/98 - mlv
	CTime		TBase	( 1989, 1, 1, 0, 0, 0, -1 );

	CTimeSpan	TSec	( days, hrs, min, sec );
	CTime		datetime = TBase + TSec;
*/

	CPM820Time	TimeConv;
	CTime		datetime = TimeConv.ConvertSecs(longhex);

	return ( "'" + datetime.Format ( "%Y-%m-%d %H:%M:%S" ) + "'" );

}

// Converts a string value representing pennies to a 
// string representing an actual money value with decimal, 
// e.g., "1025" becomes "10.25"
CString CCreditCardXact::ConvertToMoney(CString strValue)
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

// As the first five fields of data are common to both transactions
// 81 and 82, this function will save repeating the code in both 
// ProcessXX() functions.  Note that while the tables are different
// in both transactions, the column names are still the same.
BOOL CCreditCardXact::GetCommonFieldsAndValues( CString &strFields, CString &strValues)
{
	CString strTemp;

	// StationType (first char of 1st field)
	strFields = "StationType";
	strTemp = m_lpParse->GetField(1);
	strValues = "'" + strTemp.Left(1) + "'";

	//StationAddress (remaining char's of 1st field)
	strFields += ", StationAddress";
	strValues +=", " + strTemp.Mid(1);

	//ResponseID is field 2, but is not needed for
	//the table we're inserting into.

	//TranID (3)
	strFields += ", TranID";
	strValues += ", " + m_lpParse->GetField(3);

	//Protocol (4)
	strFields += ", Protocol";
	strValues += ", " + m_lpParse->GetField(4);

	//DateTime (5)
	strTemp = ConvertDateTime(m_lpParse->GetField ( 5 ));
	if (strTemp.GetLength() == 2)
	{
		CTime ct = CTime::GetCurrentTime();
		strTemp = "'" + ct.Format("%Y-%m-%d %H:%M:%S") + "'";
	}

	strFields += ", DateTime";
	strValues += ", " + strTemp;


	return true;
}
