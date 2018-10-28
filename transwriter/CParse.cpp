// CParse.cpp
////////////////////////////////////////////////////////////////
// These class members functions parse a transaction record.
////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CParse.h"

// Constructor.

CParse::CParse (	CString &strC,		// Record to parse.
					const char sep,		// Field separator.
					const char term )	// Record terminator.
{
	CString strRecord	= strC;
	strOriginal			= strRecord;

	for (int i = 0; i < 128; i++)
		strFields[i] = "";

	Parse ( strRecord, sep, term );
}

// GetField class member function returns a field.

CString CParse::GetField ( int index )
{
	return ( strFields[index - 1] );
}

// GetRecord class member function returns the original record.

CString CParse::GetRecord ( )
{
	return ( strOriginal );
}

// Count class member function returns number of fields.

int CParse::Count ( )
{
	return ( iCount );
}

// Error class member function returns true if an error occured.

BOOL CParse::Error ( )
{
	return ( bError );
}

// Private function that parses the record.

void CParse::Parse ( CString &strRecord, const char sep, const char term )
{
	int i = 0, index = 0;
	iCount = -1;
	bError = false;

	while ( true )
	{
		if ( ( index = strRecord.Find ( sep ) ) >= 0 )
		{
			if ( index == 0 )
				strFields[i++] = "";
			else
				strFields[i++] = strRecord.Mid ( 0, index );
			strRecord = strRecord.Mid ( ++index );
		}
		else if ( ( index = strRecord.Find ( term ) ) > 0 )
		{
			if ( term == NULL )
				strFields[i++] = strRecord;
			else
				strFields[i++] = strRecord.Mid ( 0, index );
			break;
		}
		else
		{
			strFields[i++] = strRecord;
			bError = true;
			break;
		}
	}
	iCount = i;
	strRecord.Empty ( );
	strRecord.FreeExtra ( );
}
