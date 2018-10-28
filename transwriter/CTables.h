// CTables.h
////////////////////////////////////////////////////////////////
// This class structure defined the database tables.
////////////////////////////////////////////////////////////////

void ErrMsg ( CString &, long );

class CTables : public CObject
{
private:
	void AlarmFields(CString &);

	int iDebugLevel, iTransType;

	enum dbTables
	{
		alarm_tran,
		entrance_tran,
		parker_tran,
		status_tran,
		shift_tran,
		cardSale_tran
	} table;


public:
	CString GetCreditCardFields();

	CTables					( int );
	void	CommonFields	( CString & );
	void	CreditCard		( CString & );
	void	Alarm			( CString & );
	void	Entrance		( CString & );
	void	Parker			( CString & );
	void	Status			( CString & );
	void	Shift			( CString & );
	void	CardSale		( CString & );
	void	SetCredit		( BOOL );
	CString GetTable		( int );
	CString GetFields		( );
	BOOL	bCreditCard;
};
