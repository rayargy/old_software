#if !defined(AFX_BACKUPDLG_H__472ADA58_678E_11D3_80E1_0090271354E9__INCLUDED_)
#define AFX_BACKUPDLG_H__472ADA58_678E_11D3_80E1_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BackupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBackupDlg dialog

class CBackupDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CBackupDlg)

// Construction
public:
	void GetRegistrySettings();
	void UpdateRegistrySettings();
	CBackupDlg();
	~CBackupDlg();

// Dialog Data
	//{{AFX_DATA(CBackupDlg)
	enum { IDD = IDD_BACKUPDIALOG };
	CListBox	m_MoveList;
	CListBox	m_CopyList;
	CString	m_sBackupPath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBackupDlg)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBackupDlg)
	afx_msg void OnAddmovebutton();
	afx_msg void OnAddcopybutton();
	afx_msg void OnDblclkCopylist();
	afx_msg void OnDblclkMovelist();
	afx_msg void OnChangebutton();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BACKUPDLG_H__472ADA58_678E_11D3_80E1_0090271354E9__INCLUDED_)
