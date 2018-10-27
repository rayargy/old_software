// CParse.h
//////////////////////////////////////////////////////////////////////
// This is a class definition file for the CParse class.
//////////////////////////////////////////////////////////////////////

class CParse : public CObject
{
private:

	CString strOriginal;
	CString strFields[128];

	int iCount;

	BOOL bError;

	void Parse ( CString &, const char, const char );

public:

	CParse				( CString &, const char, const char );
	CString GetField	( int );
	CString GetRecord	( );
	BOOL Error			( );
	int Count			( );
};
