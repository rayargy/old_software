#if !defined(AFX_REGISTRYDLG_H__472ADA5C_678E_11D3_80E1_0090271354E9__INCLUDED_)
#define AFX_REGISTRYDLG_H__472ADA5C_678E_11D3_80E1_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RegistryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegistryDlg dialog

class CRegistryDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CRegistryDlg)

// Construction
public:
	void SaveRegistrySettings();
	void GetRegistryInfo();
	CRegistryDlg();
	~CRegistryDlg();

// Dialog Data
	//{{AFX_DATA(CRegistryDlg)
	enum { IDD = IDD_REGISTRYDIALOG };
	CString	m_sRegFileName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CRegistryDlg)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CRegistryDlg)
	afx_msg void OnRegbutton();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REGISTRYDLG_H__472ADA5C_678E_11D3_80E1_0090271354E9__INCLUDED_)
