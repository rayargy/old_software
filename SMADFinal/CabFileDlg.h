#if !defined(AFX_CABFILEDLG_H__472ADA59_678E_11D3_80E1_0090271354E9__INCLUDED_)
#define AFX_CABFILEDLG_H__472ADA59_678E_11D3_80E1_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CabFileDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCabFileDlg dialog

class CCabFileDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CCabFileDlg)

// Construction
public:
	void SaveRegistrySettings();
	void GetRegistryInfo();
	CCabFileDlg();
	~CCabFileDlg();

// Dialog Data
	//{{AFX_DATA(CCabFileDlg)
	enum { IDD = IDD_CABDIALOG };
	CString	m_sCabFileName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CCabFileDlg)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCabFileDlg)
	afx_msg void OnCabbutton();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CABFILEDLG_H__472ADA59_678E_11D3_80E1_0090271354E9__INCLUDED_)
