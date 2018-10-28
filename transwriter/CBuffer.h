// CBuffer.h
////////////////////////////////////////////////////////////////
// This class structure defined the transaction buffer.
////////////////////////////////////////////////////////////////

#include "CParse.h"

class CBuffer : public CObject
{
private:
	CString ConvertToDollars( CString strValue );
	CString sCommonValues;

	int		iDebugLevel,	iTransType;
	CTime	CTentry,		CTexit;
	BOOL	bCreditCard;

	enum dbTables
	{
		alarm_tran,
		entrance_tran,
		parker_tran,
		status_tran,
		shift_tran,
		cardSale_tran
	} table;

	// Transaction Header

	CString strTransNum;

	// Transaction Parameters

	CString strParam[30];

	// Common Parameters
	//  0 - Trans ID (number)	shortinteger	(1 byte)
	//  1 - Station Type		station_types	(1 byte)
	//  2 - Station Address		shortinteger	(1 byte)
	//  3 - Transaction Time	longhex			(4 bytes)
	//  4 - Protocol (calc)		integer			(2 bytes)

	// Optional Parameters

	//  5 - dp_type			dp_types		(1 byte)
	//  6 - rej_code		integer			(2 bytes)
	//  7 - int_nr			integer			(2 bytes)
	//  8 - sub_nr			integer			(2 bytes)
	//  9 - kp_ticket_id	longinteger		(4 bytes)
	// 10 - val_index		shortinteger	(1 bytes)
	// 11 - calc_cost		longinteger		(4 bytes)
	// 12 - lint_nr			longinteger		(4 bytes)
	// 13 - ticket_time		longhex			(4 bytes)
	// 14 - prev_statsort	station_types	(1 byte)
	// 15 - prev_stataddr	integer			(2 bytes)
	// 16 - tariff_nr		shortinteger	(1 byte)
	// 17 - KP_phnr			shortinteger	(1 byte)
	// 18 - pay_method		shortinteger	(1 byte)
	// 19 - KP_status		shortinteger	(1 byte)

	// Credit Card Parameters

	CString strCredit[14];

	//  0 - card company ID.
	//  1 - card number.
	//  2 - start date.
	//  3 - expiration date.
	//  4 - authorization code.
	//  5 - transaction number.
	//  6 - service code, if VISA.
	//  7 - request flag.
	//  8 - result and response code.
	//  9 - inhibit error flag.
	// 10 - credit error.
	// 11 - cashier number.
	// 12 - shift number.
	// 13 - KP Trace

public:
	CString GetCommonValues();
	CString GetCardValueFromTrace(CString traceString);
	CString GetTicketTypeIDFromTrace(CString traceString);
	CString GetCreditCardValues();

	CBuffer					( int );
	void	EmptyStr		( );
	CString strDateTime		( CString, BOOL );
	CString strTicketType	( CString );
	CString strTicketNo		( CString, CString );
	void	Station			( CString, int, int );
	void	SetCommData		( CParse & );
	void	Alarm			( CParse & );
	void	Entrance		( CParse & );
	void	Parker			( CParse & );
	void	Status			( CParse & );
	void	Shift			( CParse & );
	void	CardSale		( CParse & );
	void	CreditCard		( CParse &, int );
	void	SetData			( CParse & );
	void	SetTable		( CString & );
	CString GetValues		( );
	BOOL	GetCredit		( );
};
