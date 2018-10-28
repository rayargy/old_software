// PM820Time.cpp: implementation of the CPM820Time class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DebugInfo.h"
#include "ErrorHandler.h"
#include "PARCSMsgs.h"
#include "PM820Time.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPM820Time::CPM820Time()
{

}

CPM820Time::~CPM820Time()
{

}

CTime CPM820Time::ConvertSecs(CString sHexSeconds)
{
	CString sConvString;
	char *szStopString;

	sConvString = sHexSeconds;
	sConvString.TrimLeft();
	sConvString = "0x" + sConvString;
	long lIncomingSecs = strtol(sConvString, &szStopString, 16);
	CTimeSpan TimeSpan(GetDays(lIncomingSecs), GetHours(lIncomingSecs), 
		GetMinutes(lIncomingSecs), GetSeconds(lIncomingSecs));

//	CTime tTimeCalc(1989, 1, 1, 0, 0, 0, 0);
//	Changed DST value from 0 to 1 on 4/17/1998 to fix problem - JLE

	CTime tTimeCalc(1989, 1, 1, 0, 0, 0, -1);

	tTimeCalc += TimeSpan;
//	AdjustForDST ( tTimeCalc );

	return tTimeCalc;
}

long CPM820Time::GetDays(long lSeconds)
{
	return (lSeconds / 86400);
}

int CPM820Time::GetHours(long lSeconds)
{
	return ((lSeconds - (GetDays(lSeconds) * 86400)) / 3600);
}

int CPM820Time::GetMinutes(long lSeconds)
{
	return ((lSeconds - ((GetDays(lSeconds) * 86400) + 
		(GetHours(lSeconds) * 3600))) / 60);
}

int CPM820Time::GetSeconds(long lSeconds)
{
	return ((lSeconds - ((GetDays(lSeconds) * 86400) + 
		(GetHours(lSeconds) * 3600) + (GetMinutes(lSeconds) * 60))));
}

void CPM820Time::AdjustForDST(CTime & tTimeValue)
{
	int iDST;
	TIME_ZONE_INFORMATION tzInfo;
	DWORD dwTimeZoneID = GetTimeZoneInformation( &tzInfo );

	if ( dwTimeZoneID == TIME_ZONE_ID_UNKNOWN )
	{
		iDST = 0;	// No DST to worry about, so set the adjustment to zero..
	}
	else
	{
		if ( tzInfo.StandardDate.wYear == 0 )
		{
			int iDay = 0, 
				iMonth = (int) tzInfo.StandardDate.wMonth,
				iYear = tTimeValue.GetYear(), 
				iHours = tzInfo.StandardDate.wHour,
				iMinutes = tzInfo.StandardDate.wMinute;
			// The year is zero, so the values are 'day-in-month'
			if ( tzInfo.StandardDate.wDay == 5 )
			{
				// Last week of month, so set the date to first of
				// next month...
				iDay = 1;
				iMonth += 1;
			}
			else
			{
				// Set to last of week indicated...
				iDay = (int) tzInfo.StandardDate.wDay * 7;
			}
			// Set up a time object for compare...
			CTime tChngToSTD(iYear, iMonth, iDay, iHours, iMinutes, 0, 0);
			// This time value is in terms of DST, so we need to adjust it to standard
			tChngToSTD -= CTimeSpan( 0, 1, 0, 0 ); // 1 hour exactly

			if ( tzInfo.StandardDate.wDay == 5 )
			{
				// We set up the next month, so now we need to go back to the
				// last day of the month that we need...
				tChngToSTD -= CTimeSpan( 1, 0, 0, 0 ); // 1 day exactly
			}
			while ( tChngToSTD.GetDayOfWeek() != ((int) tzInfo.StandardDate.wDayOfWeek + 1) )
			{
				// Keep subtracting a day until we match the day of week
				tChngToSTD -= CTimeSpan( 1, 0, 0, 0 ); // 1 day exactly
			}

			// Now set the DST change...
			iDay = 0;
			iMonth = (int) tzInfo.DaylightDate.wMonth;
			iYear = tTimeValue.GetYear();
			iHours = tzInfo.DaylightDate.wHour;
			iMinutes = tzInfo.DaylightDate.wMinute;
			if ( tzInfo.DaylightDate.wDay == 5 )
			{
				// Last week of month, so set the date to first of
				// next month...
				iDay = 1;
				iMonth += 1;
			}
			else
			{
				// Set to last of week indicated...
				iDay = (int) tzInfo.DaylightDate.wDay * 7;
			}
			// Set up a time object for compare...
			CTime tChngToDST(iYear, iMonth, iDay, iHours, iMinutes, 0, 0);
			if ( tzInfo.DaylightDate.wDay == 5 )
			{
				// We set up the next month, so now we need to go back to the
				// last day of the month that we need...
				tChngToDST -= CTimeSpan( 1, 0, 0, 0 ); // 1 day exactly
			}
			while ( tChngToDST.GetDayOfWeek() != ((int) tzInfo.DaylightDate.wDayOfWeek + 1) )
			{
				// Keep subtracting a day until we match the day of week
				tChngToDST -= CTimeSpan( 1, 0, 0, 0 ); // 1 day exactly
			}
			if( tTimeValue < tChngToDST )
				iDST = (int) tzInfo.StandardBias;		// We are before DST change...
			else if ( tTimeValue < tChngToSTD )
				iDST = (int) tzInfo.DaylightBias;		// We are before STD change...
			else
				iDST = (int) tzInfo.StandardBias;
		}
		else
		{
			// A date was specified, so we use the set date...
			int iDay = (int) tzInfo.StandardDate.wDay,
				iMonth = (int) tzInfo.StandardDate.wMonth,
				iYear = tTimeValue.GetYear(),
				iHours = tzInfo.StandardDate.wHour,
				iMinutes = tzInfo.StandardDate.wMinute;

			// Set up a time object for compare...
			CTime tChngToSTD(iYear, iMonth, iDay, iHours, iMinutes, 0, 0);
			// This time value is in terms of DST, so we need to adjust it to standard
			tChngToSTD -= CTimeSpan( 0, 1, 0, 0 ); // 1 hour exactly

			// Now set the DST change...
			iDay = (int) tzInfo.DaylightDate.wDay;
			iMonth = (int) tzInfo.DaylightDate.wMonth;
			iYear = tTimeValue.GetYear();
			iHours = tzInfo.DaylightDate.wHour;
			iMinutes = tzInfo.DaylightDate.wMinute;

			// Set up a time object for compare...
			CTime tChngToDST(iYear, iMonth, iDay, iHours, iMinutes, 0, 0);

			if( tTimeValue < tChngToDST )
				iDST = (int) tzInfo.StandardBias;		// We are before DST change...
			else if ( tTimeValue < tChngToSTD )
				iDST = (int) tzInfo.DaylightBias;		// We are before STD change...
			else
				iDST = (int) tzInfo.StandardBias;
		}
	}
	tTimeValue += CTimeSpan( 0, 0, iDST, 0 ); // Apply adjustment
}
