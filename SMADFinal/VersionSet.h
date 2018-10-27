// VersionSet.h : interface of the CVersionSet class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VERSIONSET_H__4F82DDD1_6228_11D3_80D9_0090271354E9__INCLUDED_)
#define AFX_VERSIONSET_H__4F82DDD1_6228_11D3_80D9_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CVersionSet : public CDaoRecordset
{
public:
	CVersionSet(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CVersionSet)

// Field/Param Data
	//{{AFX_FIELD(CVersionSet, CDaoRecordset)
	COleDateTime m_LastModDate;
	CString m_FileSize;
	CString	m_ProductName;
	CString	m_ProductVersion;
	CString	m_OriginalFilename;
	CString	m_FileDescription;
	CString	m_FileVersion;
	CString	m_CompanyName;
	CString	m_LegalCopyright;
	CString	m_LegalTrademarks;
	CString	m_InternalName;
	CString	m_PrivateBuild;
	CString	m_SpecialBuild;
	CString	m_Comments;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVersionSet)
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

#endif // !defined(AFX_VERSIONSET_H__4F82DDD1_6228_11D3_80D9_0090271354E9__INCLUDED_)
