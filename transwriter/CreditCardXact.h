// CreditCardXact.h: interface for the CCreditCardXact class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CREDITCARDXACT_H__38C54EC4_11B3_11D2_9CC3_00104B9A5312__INCLUDED_)
#define AFX_CREDITCARDXACT_H__38C54EC4_11B3_11D2_9CC3_00104B9A5312__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CCreditCardXact : public CObject 
{
public:
	CString GetSaveXactString(CParse *);
	CCreditCardXact();
	virtual ~CCreditCardXact();

private:
	BOOL GetCommonFieldsAndValues(CString &, CString &);
	CString ConvertToMoney(CString);
	CString ConvertDateTime(CString);
	CString Process82();
	CString Process81();
	CParse * m_lpParse;
	int m_intTransType;
};

#endif // !defined(AFX_CREDITCARDXACT_H__38C54EC4_11B3_11D2_9CC3_00104B9A5312__INCLUDED_)
