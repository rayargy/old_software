// PM820Time.h: interface for the CPM820Time class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PM820TIME_H__EBE17F10_1AE2_11D1_A5D9_00AA00BBBEE3__INCLUDED_)
#define AFX_PM820TIME_H__EBE17F10_1AE2_11D1_A5D9_00AA00BBBEE3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CPM820Time  
{
public:
	CTime ConvertSecs( CString sHexSeconds );
	CPM820Time();
	virtual ~CPM820Time();

private:
	void AdjustForDST(CTime & tTimeValue);
	int GetSeconds( long lSeconds );
	int GetMinutes( long lSeconds );
	int GetHours( long lSeconds );
	long GetDays( long lSeconds );
};

#endif // !defined(AFX_PM820TIME_H__EBE17F10_1AE2_11D1_A5D9_00AA00BBBEE3__INCLUDED_)
