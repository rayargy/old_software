// EventInsertSet.h : interface of the CEventInsertSet class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTINSERTSET_H__3947ED42_555F_11D3_80C9_0090271354E9__INCLUDED_)
#define AFX_EVENTINSERTSET_H__3947ED42_555F_11D3_80C9_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CEventInsertSet : public CDaoRecordset
{
public:
	CString m_sDefaultSQL;
	CEventInsertSet(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CEventInsertSet)

// Field/Param Data
	//{{AFX_FIELD(CEventInsertSet, CDaoRecordset)
	CString	m_Category;
	CString	m_Computer;
	CString	m_Description;
//	CString	m_Data;
	CString	m_HexData;
	CString	m_ASCIIData;
	COleDateTime	m_EventDate;
	long	m_EventID;
	COleDateTime	m_EventTime;
	CString	m_Source;
	CString	m_Type;
	CString	m_User;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventInsertSet)
	public:
	virtual CString GetDefaultSQL(); 	// default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);	// RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EVENTINSERTSET_H__3947ED42_555F_11D3_80C9_0090271354E9__INCLUDED_)
