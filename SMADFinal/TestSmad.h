#if !defined(AFX_TESTSMAD_H__DC2B2B84_6B6E_11D3_80E8_0090271354E9__INCLUDED_)
#define AFX_TESTSMAD_H__DC2B2B84_6B6E_11D3_80E8_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TestSmad.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTestSmad dialog

class CTestSmad : public CDialog
{
// Construction
public:
	CTestSmad(CWnd* pParent = NULL);   // standard constructor
	CSMADErrorLog* m_Log;

// Dialog Data
	//{{AFX_DATA(CTestSmad)
	enum { IDD = IDD_TESTSMAD };
	CProgressCtrl	m_SmadProgress;
	CString	m_sProc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestSmad)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTestSmad)
	virtual BOOL OnInitDialog();
	afx_msg void OnTestbutton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTSMAD_H__DC2B2B84_6B6E_11D3_80E8_0090271354E9__INCLUDED_)
