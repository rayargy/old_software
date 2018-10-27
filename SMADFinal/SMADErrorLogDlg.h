#if !defined(AFX_SMADERRORLOGDLG_H__472ADA5A_678E_11D3_80E1_0090271354E9__INCLUDED_)
#define AFX_SMADERRORLOGDLG_H__472ADA5A_678E_11D3_80E1_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SMADErrorLogDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSMADErrorLogDlg dialog

class CSMADErrorLogDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CSMADErrorLogDlg)

// Construction
public:
	void SaveRegistrySettings();
	void GetRegistryInfo();
	CSMADErrorLogDlg();
	~CSMADErrorLogDlg();

// Dialog Data
	//{{AFX_DATA(CSMADErrorLogDlg)
	enum { IDD = IDD_ERRORDIALOG };
	CSpinButtonCtrl	m_DebugS;
	CString	m_sSMADLogName;
	DWORD	m_dwDebug;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSMADErrorLogDlg)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSMADErrorLogDlg)
	afx_msg void OnSmadlogbutton();
	virtual BOOL OnInitDialog();
	afx_msg void OnDeltaposDebugspin(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SMADERRORLOGDLG_H__472ADA5A_678E_11D3_80E1_0090271354E9__INCLUDED_)
