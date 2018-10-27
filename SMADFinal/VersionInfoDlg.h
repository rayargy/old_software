#if !defined(AFX_VERSIONINFODLG_H__472ADA5D_678E_11D3_80E1_0090271354E9__INCLUDED_)
#define AFX_VERSIONINFODLG_H__472ADA5D_678E_11D3_80E1_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// VersionInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVersionInfoDlg dialog

class CVersionInfoDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CVersionInfoDlg)

// Construction
public:
	void SaveRegistrySettings();
	void GetRegistryInfo();
	CVersionInfoDlg();
	~CVersionInfoDlg();

// Dialog Data
	//{{AFX_DATA(CVersionInfoDlg)
	enum { IDD = IDD_VERSIONDIALOG };
	CListBox	m_VersionList;
	CString	m_sVersionMdbFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CVersionInfoDlg)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CVersionInfoDlg)
	afx_msg void OnDblclkVersionlist();
	afx_msg void OnVslistbutton();
	afx_msg void OnVersionbutton();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VERSIONINFODLG_H__472ADA5D_678E_11D3_80E1_0090271354E9__INCLUDED_)
