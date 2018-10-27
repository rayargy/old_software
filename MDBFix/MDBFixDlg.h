// MDBFixDlg.h : header file
//

#if !defined(AFX_MDBFIXDLG_H__66F29137_2A0F_11D3_804B_0090271354E9__INCLUDED_)
#define AFX_MDBFIXDLG_H__66F29137_2A0F_11D3_804B_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CMDBFixDlg dialog

class CMDBFixDlg : public CDialog
{
// Construction
public:
	CMDBFixDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMDBFixDlg)
	enum { IDD = IDD_MDBFIX_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMDBFixDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMDBFixDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MDBFIXDLG_H__66F29137_2A0F_11D3_804B_0090271354E9__INCLUDED_)
